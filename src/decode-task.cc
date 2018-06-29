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

#include <sstream>
#include <string>
#include <vector>

#include "decode-task.h"

namespace xdecoder {

void DecodeTask::operator() (void *resource) {
  Net* net = reinterpret_cast<Net*>(resource);
  FeaturePipeline feature_pipeline(feature_options_);
  OnlineDecodable decodable(tree_, pdf_prior_, decodable_options_,
                            net, &feature_pipeline);
  FasterDecoder decoder(hclg_, decoder_options_);
  decoder.InitDecoding();
  bool done = false;
  while (!done) {
    std::vector<float> wav_data = audio_queue_.Get();
    LOG("WAV LEN %d", static_cast<int>(wav_data.size()));
    if (wav_data.size() == 0) {
      // end of stream
      decodable.SetDone();
      done = true;
    }

    if (wav_data.size() > 0)
      decodable.AcceptRawWav(wav_data);
    decoder.AdvanceDecoding(&decodable);
    std::vector<int32_t> result;
    decoder.GetBestPath(&result);
    std::ostringstream ss;
    for (size_t i = 0; i < result.size(); i++) {
      ss << " " << words_table_.GetSymbol(result[i]);
    }
    result_queue_.Put(ss.str());
    LOG("RESULT: %s", ss.str().c_str());
  }
}

}  // namespace xdecoder
