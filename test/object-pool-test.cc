// Copyright (c) 2016 Personal (Binbin Zhang)
// Created on 2018-07-11
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

#include <iostream>

#include "object-pool.h"
#include "timer.h"

struct Point {
  int x, y, z;
};

int main(int argc, char* argv[]) {
  using xdecoder::NaiveObjectPool;
  using xdecoder::CacheObjectPool;
  using xdecoder::Timer;

  const int count = 1e5;

  NaiveObjectPool<Point> naive_pool;
  Timer t1;
  for (int i = 0; i < count; i++) {
    Point *point = naive_pool.New();
    point->x = 1;
    point->y = 2;
    point->z = 3;
    naive_pool.Delete(point);
  }
  std::cout << "timer 1 " << t1.Elapsed() << std::endl;

  CacheObjectPool<Point> cache_pool;
  Timer t2;
  for (int i = 0; i < count; i++) {
    Point *point = cache_pool.New();
    point->x = 1;
    point->y = 2;
    point->z = 3;
    cache_pool.Delete(point);
  }
  std::cout << "timer 2 " << t2.Elapsed() << std::endl;
  return 0;
}

