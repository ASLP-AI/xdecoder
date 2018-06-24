// Copyright (c) 2018 Personal (Binbin Zhang)
// Created on 2018-06-18

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

#include <algorithm>

#include "decodable.h"

namespace xdecoder {

float OnlineDecodable::LogLikelihood(int32_t frame, int32_t index) {
  ComputeForFrame(frame);
  int32_t pdf_id = tree_.TransitionIdToPdf(index);
  CHECK(frame >= begin_frame_ &&
  frame < begin_frame_ + scaled_loglikes_.NumRows());
  return scaled_loglikes_(frame - begin_frame_, pdf_id);
}

void OnlineDecodable::ComputeForFrame(int32_t frame) {
  CHECK(frame >= 0);
  int32_t features_ready = NumFramesReady();
  CHECK(frame < features_ready);

  if (frame >= begin_frame_ &&
      frame < begin_frame_ + scaled_loglikes_.NumRows())
    return;

  int32_t input_frame_begin = frame;
  int32_t max_possible_input_frame_end = features_ready;
  int32_t shift = options_.skip + 1, batch_size = options_.max_batch_size;
  int32_t input_frame_end = std::min<int32_t>(max_possible_input_frame_end,
                          input_frame_begin + batch_size * shift);

  CHECK(input_frame_end > input_frame_begin);
  int32_t feat_dim = feature_pipeline_->FeatureDim();
  int32_t num_frames_out = (input_frame_end - input_frame_begin);
  int32_t num_frames_forward = (num_frames_out - 1) / shift + 1;

  CHECK(feat_dim == net_->InDim());
  Matrix<float> in(num_frames_forward, feat_dim),
                out(num_frames_forward, net_->OutDim());
  for (int i = 0; i < num_frames_forward; i++) {
    feature_pipeline_->ReadOneFrame(input_frame_begin + i * shift,
                                    in.Row(i).Data());
  }
  net_->Forward(in, &out);
  scaled_loglikes_.Resize(num_frames_out, net_->OutDim());
  for (int i = 0; i < num_frames_forward; i++) {
    for (int j = 0; j < shift; j++) {
      int32_t row = i * shift + j;
      if (row < scaled_loglikes_.NumRows())
        scaled_loglikes_.Row(row).CopyFrom(out.Row(i));
    }
  }
  // Here we suppose softmax is remove in the AM
  // Directly substract log prior
  scaled_loglikes_.AddVec(pdf_prior_, -1.0f);
  scaled_loglikes_.Scale(options_.acoustic_scale);
  begin_frame_ = frame;
}

}  // namespace xdecoder
