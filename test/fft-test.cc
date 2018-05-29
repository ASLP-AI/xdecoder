// Copyright (c) 2018 Personal (Binbin Zhang)
// Created on 2016-04-09
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

#include "fft.h"

int main() {
  int n = 8;
  float x[8] = {1, 2, 3, 4, 5, 6, 0, 0};
  float y[8] = {0};

  xdecoder::fft(x, y, n);
  for (int i = 0; i < n; i++) {
    printf("%f  %f\n", x[i], y[i]);
  }
  printf("\n\n");
  xdecoder::fft(x, y, -n);
  for (int i = 0; i < n; i++) {
    printf("%f  %f\n", x[i], y[i]);
  }

  return 0;
}
