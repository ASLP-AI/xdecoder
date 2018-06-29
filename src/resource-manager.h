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


#ifndef RESOURCE_MANAGER_H_
#define RESOURCE_MANAGER_H_

#include <string>
#include <vector>

namespace xdecoder {

class ResourceManager {
 public:
  ResourceManager();
  ~ResourceManager();

  void set_beam(float beam);
  void set_max_active(int max_active);
  void set_acoustic_scale(float acoustic_scale);
  void set_skip(int skip);
  void set_max_batch_size(int max_batch_size);
  void set_num_bins(int num_bins);
  void set_left_context(int left_context);
  void set_right_context(int right_context);

  void set_cmvn(std::string cmvn);
  void set_hclg(std::string hclg);
  void set_tree(std::string tree);
  void set_net(std::string net);
  void set_pdf_prior(std::string pdf_prior);
  void set_word(std::string word);

  void set_thread_pool_size(int size);

  void init();

 private:
  // FasterDecoderOptions
  float beam_;
  int max_active_;

  // DecodableOptions
  float acoustic_scale_;
  int skip_;
  int max_batch_size_;

  // FeaturePipelineConfig
  int num_bins_;
  int left_context_;
  int right_context_;

  // ThreadPool number
  int thread_pool_size_;

  // Config files, all of them are paths
  std::string cmvn_;
  std::string hclg_;
  std::string tree_;
  std::string net_;
  std::string pdf_prior_;
  std::string word_;  // word list by int, refer to kaldi words.txt

  void* faster_decoder_options_;
  void* decodable_options_;
  void* feature_options_;
  void* thread_pool_;
  std::vector<void *> resource_pool_;
};

}  // namespace xdecoder

#endif  // RESOURCE_MANAGER_H_
