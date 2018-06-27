// Copyright (c) 2018 Personal (Binbin Zhang)
// Created on 2018-06-27
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


#ifndef MESSAGE_QUEUE_H_
#define MESSAGE_QUEUE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <vector>
#include <queue>

#include "utils.h"

namespace xdecoder {

template <class Type>
class MessageQueue {
 public:
  MessageQueue() {
    if (pthread_mutex_init(&mutex_, NULL) != 0) {
      ERROR("mutex init error");
    }
    if (pthread_cond_init(&cond_, NULL) != 0) {
      ERROR("cond init error");
    }
  }

  ~MessageQueue() {
    pthread_mutex_destroy(&mutex_);
    pthread_cond_destroy(&cond_);
  }

  void Put(const Type& msg) {
    pthread_mutex_lock(&mutex_);
    queue_.push(msg);
    pthread_mutex_unlock(&mutex_);
    pthread_cond_signal(&cond_);
  }

  Type Get() {
    Type msg;
    pthread_mutex_lock(&mutex_);
    while (queue_.empty()) {
      pthread_cond_wait(&cond_, &mutex_);
    }
    if (queue_.size() > 0) {
      msg = queue_.front();
      queue_.pop();
    }
    pthread_mutex_unlock(&mutex_);
    return msg;
  }

 private:
  std::queue<Type> queue_;
  pthread_cond_t cond_;
  pthread_mutex_t mutex_;
};

}  // namespace xdecoder

#endif  // MESSAGE_QUEUE_H_
