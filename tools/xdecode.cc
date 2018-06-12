// Copyright (c) 2016 Personal (Binbin Zhang)
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

#include <stdio.h>

#include <limits>

#include "fst.h"
#include "faster-decoder.h"
#include "parse-option.h"

int main(int argc, char* argv[]) {
  using xdecoder::ParseOptions;
  const char *usage = "\n";
  ParseOptions option(usage);
  float beam = 16.0;
  int32_t max_active = std::numeric_limits<int32_t>::max();
  int32_t min_active = 20;
  float beam_delta = 0.5;
  float hash_ratio = 2.0;
  bool full = false;


  option.Register("beam", &beam,
                  "Decoding beam.  Larger->slower, more accurate.");
  option.Register("max-active", &max_active,
                 "Decoder max active states.  Larger->slower; more accurate");
  option.Register("min-active", &min_active,
                 "Decoder min active states "
                 "(don't prune if #active less than this).");
  if (full) {
    option.Register("beam-delta", &beam_delta,
                   "Increment used in decoder [obscure setting]");
    option.Register("hash-ratio", &hash_ratio,
                   "Setting used in decoder to control hash behavior");
  }

  option.Read(argc, argv);

  if (option.NumArgs() != 2) {
    option.PrintUsage();
    exit(1);
  }

  return 0;
}

