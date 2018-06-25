// Copyright (c) 2016 Personal (Binbin Zhang)
// Created on 2016-05-24
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
#include <stdlib.h>

#include "thread-pool.h"

namespace xdecoder {

class MyTask : public Threadable {
 public:
  explicit MyTask(int id): id_(id) {}
  ~MyTask() {
      // printf("thread %d exit\n", id_);
  }
  virtual void operator() (void *resource) {
    int num = rand() % 100000;
    printf("thread %d num %d\n", id_, num);
    for (int i = 0; i < num; i++) {}
  }

 private:
  int id_;
};

void TestThreadPool() {
  ThreadPool thread_pool(5);
  for (int i = 0; i < 10; i++) {
    Threadable *task = new MyTask(i);
    thread_pool.AddTask(task);
  }
}

}  // namespace xdecoder


int main() {
  xdecoder::TestThreadPool();
  return 0;
}



