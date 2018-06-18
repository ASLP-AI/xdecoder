// Copyright (c) 2018 Personal (Binbin Zhang)
// Created on 2018-06-12
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

#include "utils.h"

#ifndef DECODABLE_H_
#define DECODABLE_H_

namespace xdecoder {

class Decodable {
 public:
  /// Returns the log likelihood, which will be negated in the decoder.
  /// The "frame" starts from zero.  You should verify that IsLastFrame(frame-1)
  /// returns false before calling this.
  virtual float LogLikelihood(int32_t frame, int32_t index) = 0;

  /// Returns true if this is the last frame.  Frames are zero-based, so the
  /// first frame is zero.  IsLastFrame(-1) will return false, unless the file
  /// is empty (which is a case that I'm not sure all the code will handle, so
  /// be careful).  Caution: the behavior of this function in an online setting
  /// is being changed somewhat.  In future it may return false in cases where
  /// we haven't yet decided to terminate decoding, but later true if we decide
  /// to terminate decoding.  The plan in future is to rely more on
  /// NumFramesReady(), and in future, IsLastFrame() would always return false
  /// in an online-decoding setting, and would only return true in a
  /// decoding-from-matrix setting where we want to allow the last delta or LDA
  /// features to be flushed out for compatibility with the baseline setup.
  virtual bool IsLastFrame(int32_t frame) const = 0;

  /// The call NumFramesReady() will return the number of frames currently
  /// available for this decodable object.  This is for use in setups where
  /// you don't want the decoder to block while waiting for input.
  /// This is newly added as of Jan 2014, and I hope, going forward,
  /// to rely on this mechanism more than IsLastFrame to
  /// know when to stop decoding.
  virtual int32_t NumFramesReady() const {
      ERROR("NumFramesReady() not implemented for this decodable type.");
      return -1;
  }

  /// Returns the number of states in the acoustic model
  /// (they will be indexed one-based, i.e. from 1 to NumIndices();
  /// this is for compatibility with OpenFst.
  virtual int32_t NumIndices() const = 0;

  virtual ~Decodable() {}
};

}  // namespace xdecoder

#endif  // DECODABLE_H_
