// Copyright (c) 2017 Personal (Binbin Zhang)
// Created on 2017-08-17
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

#ifndef VAD_H_
#define VAD_H_

#include <string>
#include <vector>

#include "feature-pipeline.h"
#include "net.h"

namespace xdecoder {

const int kMaxAudioBuffer = 10 * 60 * 16000;  // 10min buffer for VAD

struct VadConfig {
  FeaturePipelineConfig feature_config;
  float silence_thresh;  // (0, 1)
  std::string net_file;
  int silence_to_speech_thresh;
  int speech_to_sil_thresh;
  int endpoint_trigger_thresh;  // 1.0s, 100 frames
  int num_frames_lookback;
  VadConfig(): silence_thresh(0.5),
               silence_to_speech_thresh(3),
               speech_to_sil_thresh(15),
               endpoint_trigger_thresh(100),
               num_frames_lookback(0) {}
};

typedef enum {
  kSpeech,
  kSilence
} VadState;

class Vad {
 public:
  explicit Vad(const VadConfig& config);
  // return true is contains endpoint
  bool DoVad(const std::vector<float>& wave, bool end_of_stream,
             std::vector<float>* speech = NULL);
  // internal state machine smooth
  bool Smooth(bool is_voice);
  void Reset();
  bool EndpointDetected() const { return endpoint_detected_; }
  const std::vector<bool>& Results() const {
    return results_;
  }
  void SetDone() {
    feature_pipeline_.SetDone();
  }
  void Lookback();

 private:
  const VadConfig& config_;
  FeaturePipeline feature_pipeline_;
  int silence_frame_count_, speech_frame_count_, frame_count_;
  VadState state_;
  Net net_;
  bool endpoint_detected_;
  std::vector<bool> results_;
  std::vector<float> audio_buffer_;
  int t_;
};

}  // namespace xdecoder

#endif  // VAD_H_
