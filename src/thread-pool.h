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


#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <vector>
#include <queue>
#include <map>

#include "utils.h"

namespace xdecoder {

// Thread interface for thread class
class Threadable {
 public:
  // @resource: thread resource allocated by the thread pool
  virtual void operator() (void *resource) = 0;
  virtual ~Threadable() {}
};

// ThreadPool
class ThreadPool {
 public:
  // resource_pool: optional resource pool for every thread
  // Here we just use the void * for polymorphism, for it is simple and stupid
  // We can also use the template programming, like template <typename C>
  // class ThreadPool, but it is more complicated and requires specific init
  explicit ThreadPool(size_t num_thread = 5,
                      std::vector<void *> *resource_pool = NULL):
      num_thread_(num_thread), stop_(false) {
    if (resource_pool != NULL && resource_pool->size() != num_thread_) {
      ERROR("resource and num thread must equal");
    }
    if (pthread_mutex_init(&mutex_, NULL) != 0) {
      ERROR("mutex init error");
    }
    if (pthread_cond_init(&cond_, NULL) != 0) {
      ERROR("cond init error");
    }

    // Create num_thread thread at once
    threads_.resize(num_thread_);
    for (size_t i = 0; i < threads_.size(); i++) {
      if (pthread_create(&threads_[i], NULL,
            ThreadPool::WorkerThread, reinterpret_cast<void *>(this)) != 0) {
        ERROR("pthread %d create error", static_cast<int32_t>(i));
      }
      if (resource_pool != NULL) {
        resource_table_[threads_[i]] = (*resource_pool)[i];
      } else {
        resource_table_[threads_[i]] = NULL;
      }
    }
  }

  ~ThreadPool() {
    pthread_mutex_lock(&mutex_);
    stop_ = true;
    pthread_mutex_unlock(&mutex_);
    // Notify all thread to stop
    pthread_cond_broadcast(&cond_);

    for (size_t i = 0; i < threads_.size(); i++) {
      pthread_join(threads_[i], NULL);
    }

    pthread_mutex_destroy(&mutex_);
    pthread_cond_destroy(&cond_);
  }

  void *GetResource(pthread_t tid) {
    CHECK(resource_table_.find(tid) != resource_table_.end());
    return resource_table_[tid];
  }

  void AddTask(Threadable *task) {
    pthread_mutex_lock(&mutex_);
    task_queue_.push(task);
    pthread_mutex_unlock(&mutex_);
    pthread_cond_signal(&cond_);
  }

  // Wait a task to execute
  Threadable *WaitTask() {
    Threadable *task = NULL;
    pthread_mutex_lock(&mutex_);
    while (!stop_ && task_queue_.empty()) {
      pthread_cond_wait(&cond_, &mutex_);
    }
    if (task_queue_.size() > 0) {
      task = task_queue_.front();
      task_queue_.pop();
    }
    // else stop_ = true return NULL
    pthread_mutex_unlock(&mutex_);
    return task;
  }

  // PoolWorker thread
  static void *WorkerThread(void *arg) {
    // sleep(3);  // wait main thread to construct the resource_table_
    ThreadPool *pool = static_cast<ThreadPool *>(arg);
    for (;;) {
      Threadable *task = pool->WaitTask();
      // Stop
      if (task == NULL) {
        break;
      } else {
        void *resource = pool->GetResource(pthread_self());
        (*task)(resource);  // Run the task
      }
    }
    return NULL;
  }

 private:
  size_t num_thread_;
  bool stop_;
  std::vector<pthread_t> threads_;
  std::vector<void *> *resource_pool_;
  std::queue<Threadable *> task_queue_;  // TaskQueue
  std::map<pthread_t, void *> resource_table_;
  pthread_cond_t cond_;
  pthread_mutex_t mutex_;
};

}  // namespace xdecoder

#endif  // THREAD_POOL_H_
