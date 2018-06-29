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
#include "resource-manager.h"

namespace xdecoder {

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
                                    thread_pool_(NULL) {}

void ResourceManager::set_beam(float beam) {
  beam_ = beam;
}

ResourceManager::~ResourceManager() {
  if (faster_decoder_options_ != NULL)
    delete reinterpret_cast<FasterDecoderOptions*>(faster_decoder_options_);
  if (decodable_options_ != NULL)
    delete reinterpret_cast<DecodableOptions*>(decodable_options_);
  if (feature_options_ != NULL)
    delete reinterpret_cast<FeaturePipelineConfig*>(feature_options_);
  if (thread_pool_ != NULL)
    delete reinterpret_cast<ThreadPool*>(thread_pool_);
  for (size_t i = 0; i < resource_pool_.size(); i++) {
    if (resource_pool_[i] != NULL)
      delete reinterpret_cast<Net*>(resource_pool_[i]);
  }
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

void ResourceManager::set_cmvn(std::string cmvn) {
  cmvn_ = cmvn;
}

void ResourceManager::set_hclg(std::string hclg) {
  hclg_ = hclg;
}

void ResourceManager::set_tree(std::string tree) {
  tree_ = tree;
}

void ResourceManager::set_net(std::string net) {
  net_ = net;
}

void ResourceManager::set_pdf_prior(std::string pdf_prior) {
  pdf_prior_ = pdf_prior;
}

void ResourceManager::set_word(std::string word) {
  word_ = word;
}

void ResourceManager::init() {
  CHECK(thread_pool_size_ > 0);
  resource_pool_.resize(thread_pool_size_, NULL);
  for (int i = 0; i < thread_pool_size_; i++) {
    Net *am_net = new Net(net_);
    resource_pool_[i] = reinterpret_cast<void *>(am_net);
  }
  thread_pool_ = reinterpret_cast<void *>(
                     new ThreadPool(thread_pool_size_, &resource_pool_));
}


}  // namespace xdecoder
