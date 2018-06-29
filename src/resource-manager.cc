// Copyright (c) 2018 Personal (Binbin Zhang)
// Created on 2018-06-27
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "decodable.h"
#include "faster-decoder.h"
#include "feature-pipeline.h"
#include "thread-pool.h"
#include "net.h"
#include "decode-task.h"
#include "resource-manager.h"

namespace xdecoder {

Recognizer::Recognizer(): decode_task_(NULL) {}

Recognizer::~Recognizer() {
  if (decode_task_ != NULL)
    delete reinterpret_cast<DecodeTask*>(decode_task_);
}

void Recognizer::add_wav(const std::vector<float>& wav_data) {
  CHECK(decode_task_ != NULL);
  reinterpret_cast<DecodeTask*>(decode_task_)->AddWavData(wav_data);
}

std::string Recognizer::get_result() {
  CHECK(decode_task_ != NULL);
  return reinterpret_cast<DecodeTask*>(decode_task_)->GetResult();
}

void Recognizer::set_done() {
  CHECK(decode_task_ != NULL);
  reinterpret_cast<DecodeTask*>(decode_task_)->AddWavData(std::vector<float>());
}

void Recognizer::set_decode_task(void *decode_task) {
  CHECK(decode_task_ == NULL);
  decode_task_ = decode_task;
}

ResourceManager::ResourceManager(): beam_(13.0),
                                    max_active_(7000),
                                    acoustic_scale_(0.1f),
                                    skip_(0),
                                    max_batch_size_(16),
                                    num_bins_(40),
                                    left_context_(5),
                                    right_context_(5),
                                    thread_pool_size_(8),
                                    faster_decoder_options_(NULL),
                                    decodable_options_(NULL),
                                    feature_options_(NULL),
                                    thread_pool_(NULL),
                                    hclg_(NULL),
                                    tree_(NULL),
                                    pdf_prior_(NULL),
                                    words_table_(NULL) {}

ResourceManager::~ResourceManager() {
  if (thread_pool_ != NULL)
    delete reinterpret_cast<ThreadPool*>(thread_pool_);

  if (faster_decoder_options_ != NULL)
    delete reinterpret_cast<FasterDecoderOptions*>(faster_decoder_options_);
  if (decodable_options_ != NULL)
    delete reinterpret_cast<DecodableOptions*>(decodable_options_);
  if (feature_options_ != NULL)
    delete reinterpret_cast<FeaturePipelineConfig*>(feature_options_);
  if (hclg_ != NULL)
    delete reinterpret_cast<Fst*>(hclg_);
  if (tree_ != NULL)
    delete reinterpret_cast<Tree*>(tree_);
  if (pdf_prior_ != NULL)
    delete reinterpret_cast<Vector<float>*>(pdf_prior_);
  if (words_table_ != NULL)
    delete reinterpret_cast<SymbolTable*>(words_table_);

  for (size_t i = 0; i < resource_pool_.size(); i++) {
    if (resource_pool_[i] != NULL)
      delete reinterpret_cast<Net*>(resource_pool_[i]);
  }
}

void ResourceManager::set_beam(float beam) {
  beam_ = beam;
}

void ResourceManager::set_max_active(int max_active) {
  max_active_ = max_active;
}

void ResourceManager::set_acoustic_scale(float acoustic_scale) {
  acoustic_scale_ = acoustic_scale;
}

void ResourceManager::set_skip(int skip) {
  skip_ = skip;
}

void ResourceManager::set_max_batch_size(int max_batch_size) {
  max_batch_size_ = max_batch_size;
}

void ResourceManager::set_num_bins(int num_bins) {
  num_bins_ = num_bins;
}

void ResourceManager::set_left_context(int left_context) {
  left_context_ = left_context;
}

void ResourceManager::set_right_context(int right_context) {
  right_context_ = right_context;
}

void ResourceManager::set_thread_pool_size(int size) {
  thread_pool_size_ = size;
}

void ResourceManager::set_cmvn(const std::string& cmvn) {
  cmvn_file_ = cmvn;
}

void ResourceManager::set_hclg(const std::string& hclg) {
  hclg_file_ = hclg;
}

void ResourceManager::set_tree(const std::string& tree) {
  tree_file_ = tree;
}

void ResourceManager::set_net(const std::string& net) {
  net_file_ = net;
}

void ResourceManager::set_pdf_prior(const std::string& pdf_prior) {
  pdf_prior_file_ = pdf_prior;
}

void ResourceManager::set_lexicon(const std::string& lexicon) {
  words_table_file_ = lexicon;
}

void ResourceManager::init() {
  FasterDecoderOptions* faster_decoder_options = new FasterDecoderOptions();
  faster_decoder_options->beam = beam_;
  faster_decoder_options->max_active = max_active_;
  faster_decoder_options_ = reinterpret_cast<void*>(faster_decoder_options);

  DecodableOptions* decodable_options = new DecodableOptions();
  decodable_options->acoustic_scale = acoustic_scale_;
  decodable_options->skip = skip_;
  decodable_options->max_batch_size = max_batch_size_;
  decodable_options_ = reinterpret_cast<void*>(decodable_options);

  FeaturePipelineConfig* feature_options = new FeaturePipelineConfig();
  feature_options->num_bins = num_bins_;
  feature_options->left_context = left_context_;
  feature_options->right_context = right_context_;
  CHECK(cmvn_file_ != "");
  feature_options->cmvn_file = cmvn_file_;
  feature_options_ = reinterpret_cast<void*>(feature_options);

  CHECK(hclg_file_ != "");
  hclg_ = reinterpret_cast<void*>(new Fst(hclg_file_));

  CHECK(tree_file_ != "");
  tree_ = reinterpret_cast<void*>(new Tree(tree_file_));

  CHECK(pdf_prior_file_ != "");
  Vector<float> *pdf_prior = new Vector<float>();
  pdf_prior->Read(pdf_prior_file_);
  pdf_prior_ = reinterpret_cast<void*>(pdf_prior);

  CHECK(words_table_file_ != "");
  words_table_ = reinterpret_cast<void*>(new SymbolTable(words_table_file_));

  CHECK(thread_pool_size_ > 0);
  resource_pool_.resize(thread_pool_size_, NULL);
  for (int i = 0; i < thread_pool_size_; i++) {
    Net *am_net = new Net(net_file_);
    resource_pool_[i] = reinterpret_cast<void *>(am_net);
  }
  thread_pool_ = reinterpret_cast<void *>(
                     new ThreadPool(thread_pool_size_, &resource_pool_));
  LOG("Resource manager init okay");
}

void ResourceManager::add_recognizer(Recognizer *recognizer) {
  CHECK(recognizer != NULL);
  DecodeTask *task = new DecodeTask(
      *(reinterpret_cast<FasterDecoderOptions*>(faster_decoder_options_)),
      *(reinterpret_cast<DecodableOptions*>(decodable_options_)),
      *(reinterpret_cast<FeaturePipelineConfig*>(feature_options_)),
      *(reinterpret_cast<Fst*>(hclg_)),
      *(reinterpret_cast<Tree*>(tree_)),
      *(reinterpret_cast<Vector<float>*>(pdf_prior_)),
      *(reinterpret_cast<SymbolTable*>(words_table_)));
  recognizer->set_decode_task(task);
  reinterpret_cast<ThreadPool*>(thread_pool_)->AddTask(task);
}


}  // namespace xdecoder
