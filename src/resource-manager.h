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

class Recognizer {
 public:
  Recognizer();
  ~Recognizer();

  void add_wav(const std::vector<float>& wav_data);
  std::string get_result();
  void set_done();

 public:
  // Don't call this function from python code
  void set_decode_task(void *decode_task);

 private:
  void *decode_task_;
};

class ResourceManager {
 public:
  ResourceManager();
  ~ResourceManager();

  void set_beam(float beam);
  void set_max_active(int max_active);
  void set_acoustic_scale(float acoustic_scale);
  void set_skip(int skip);
  void set_max_batch_size(int max_batch_size);
  void set_am_num_bins(int num_bins);
  void set_am_left_context(int left_context);
  void set_am_right_context(int right_context);

  void set_vad_num_bins(int num_bins);
  void set_vad_left_context(int left_context);
  void set_vad_right_context(int right_context);
  void set_silence_thresh(float thresh);
  void set_silence_to_speech_thresh(int thresh);
  void set_speech_to_sil_thresh(int thresh);
  void set_endpoint_trigger_thresh(int thresh);

  void set_am_cmvn(const std::string& cmvn);
  void set_vad_cmvn(const std::string& cmvn);
  void set_hclg(const std::string& hclg);
  void set_tree(const std::string& tree);
  void set_am_net(const std::string& net);
  void set_vad_net(const std::string& net);
  void set_pdf_prior(const std::string& pdf_prior);
  void set_lexicon(const std::string& lexicon);

  void set_thread_pool_size(int size);

  void init();
  void add_recognizer(Recognizer *recognizer);

 private:
  // FasterDecoderOptions
  float beam_;
  int max_active_;

  // DecodableOptions
  float acoustic_scale_;
  int skip_;
  int max_batch_size_;

  // FeaturePipelineConfig
  int am_num_bins_;
  int am_left_context_;
  int am_right_context_;

  // ThreadPool number
  int thread_pool_size_;

  // Vad FeaturePipelineConfig
  int vad_num_bins_;
  int vad_left_context_;
  int vad_right_context_;
  float silence_thresh_;
  int silence_to_speech_thresh_;
  int speech_to_sil_thresh_;
  int endpoint_trigger_thresh_;

  // Config files, all of them are paths
  std::string am_cmvn_file_;
  std::string vad_cmvn_file_;
  std::string hclg_file_;
  std::string tree_file_;
  std::string am_net_file_;
  std::string vad_net_file_;
  std::string pdf_prior_file_;
  std::string words_table_file_;  // word list by int, refer to kaldi words.txt

  void* faster_decoder_options_;
  void* decodable_options_;
  void* feature_options_;
  void* vad_options_;
  void* thread_pool_;
  void* hclg_;
  void* tree_;
  void* pdf_prior_;
  void* words_table_;
  std::vector<void *> resource_pool_;
};

}  // namespace xdecoder

#endif  // RESOURCE_MANAGER_H_
