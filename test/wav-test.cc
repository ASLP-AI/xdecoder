// Copyright (c) 2016 Personal (Binbin Zhang)
// Created on 2016-08-15
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


#include "wav.h"


int main(int argc, char *argv[]) {
  using xdecoder::WavReader;
  using xdecoder::WavWriter;
  // const char *usage = "Test wav reader and writer\n"
  //                     "Usage: wav-test wav_in_file wav_output_file\n";
  // if (argc != 3) {
  //   printf(usage);
  //   exit(-1);
  // }

  WavReader reader("res/test.wav");

  WavWriter writer(reader.Data(), reader.NumSample(), reader.NumChannel(),
                   reader.SampleRate(), reader.BitsPerSample());
  writer.Write("res/test.mirror.wav");
  return 0;
}
