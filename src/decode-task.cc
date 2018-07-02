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
  Vad vad(vad_options_);
  FasterDecoder decoder(hclg_, decoder_options_);
  decoder.InitDecoding();
  bool done = false;
  while (!done) {
    std::vector<float> wav_data = audio_queue_.Get();
    if (wav_data.size() == 0) {
      // end of stream
      done = true;
    }
    std::vector<float> speech_wav_data;
    bool is_endpoint = vad.DoVad(wav_data, done, &speech_wav_data);
    LOG("wav data %d speech data %d", static_cast<int>(wav_data.size()),
                                      static_cast<int>(speech_wav_data.size()));
    if (speech_wav_data.size() > 0) decodable.AcceptRawWav(speech_wav_data);
    bool reset = false;
    if (done || is_endpoint) {
      decodable.SetDone();
      reset = true;
    }
    decoder.AdvanceDecoding(&decodable);
    std::vector<int32_t> result;
    decoder.GetBestPath(&result);
    std::ostringstream ss;
    if (reset) {
      ss << "final:";
      decodable.Reset();
      decoder.InitDecoding();
    } else {
      ss << "partial:";
    }
    for (size_t i = 0; i < result.size(); i++) {
      ss << " " << words_table_.GetSymbol(result[i]);
    }
    result_queue_.Put(ss.str());
    LOG("%s", ss.str().c_str());
  }
  LOG("Finish decoding");
}

}  // namespace xdecoder
