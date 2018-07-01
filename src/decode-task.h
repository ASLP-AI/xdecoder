// Copyright (c) 2018 Personal (Binbin Zhang)
// Created on 2018-06-28
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

#ifndef DECODE_TASK_H_
#define DECODE_TASK_H_

#include <vector>
#include <string>

#include "decodable.h"
#include "faster-decoder.h"
#include "feature-pipeline.h"
#include "net.h"
#include "thread-pool.h"
#include "message-queue.h"
#include "vad.h"

namespace xdecoder {

class DecodeTask : public Threadable {
 public:
  DecodeTask(const FasterDecoderOptions& decoder_options,
             const DecodableOptions& decodable_options,
             const FeaturePipelineConfig& feature_options,
             const VadConfig& vad_options,
             const Fst& hclg,
             const Tree& tree,
             const Vector<float>& pdf_prior,
             const SymbolTable& words_table):
      decoder_options_(decoder_options),
      decodable_options_(decodable_options),
      feature_options_(feature_options),
      vad_options_(vad_options),
      hclg_(hclg),
      tree_(tree),
      pdf_prior_(pdf_prior),
      words_table_(words_table) {}
  // Here resource is a pointer to a Nnet ojbect
  virtual void operator() (void* resource);
  void AddWavData(const std::vector<float>& data) {
    audio_queue_.Put(data);
  }
  std::string GetResult() {
    return result_queue_.Get();
  }

 private:
  const FasterDecoderOptions& decoder_options_;
  const DecodableOptions& decodable_options_;
  const FeaturePipelineConfig& feature_options_;
  const VadConfig& vad_options_;
  const Fst& hclg_;
  const Tree& tree_;
  const Vector<float>& pdf_prior_;
  const SymbolTable& words_table_;

  MessageQueue<std::vector<float> > audio_queue_;
  MessageQueue<std::string> result_queue_;
};

}  // namespace xdecoder

#endif  // DECODE_TASK_H_


