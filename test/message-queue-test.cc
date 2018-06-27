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


#include "message-queue.h"

void *Func(void* args) {
  using xdecoder::MessageQueue;
  MessageQueue<int32_t>* message_queue =
                         reinterpret_cast<MessageQueue<int32_t>*>(args);
  while (true) {
    int32_t msg = message_queue->Get();
    printf("thread %d num %d\n", static_cast<int>(pthread_self()), msg);
  }

  return NULL;
}

int main() {
  using xdecoder::MessageQueue;

  MessageQueue<int32_t> message_queue;

  std::vector<pthread_t> tid(5, 0);

  for (size_t i = 0; i < tid.size(); i++) {
    pthread_create(&tid[i], NULL, Func,
                   reinterpret_cast<void*>(&message_queue));
  }

  for (size_t i = 0; i < 10; i++) {
    message_queue.Put(i);
    sleep(1);
  }

  // for (size_t i = 0; i < tid.size(); i++) {
  //   pthread_join(tid[i], NULL);
  // }

  return 0;
}

