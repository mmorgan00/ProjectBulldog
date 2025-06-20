// Copyright 2025 Max Morgan
#ifndef ORION_UTIL_CONTAINERS_H_
#define ORION_UTIL_CONTAINERS_H_

#include <deque>
#include <functional>

struct DeletionQueue {
  std::deque<std::function<void()>> deletors;

  void push_function(std::function<void()>&& function) {
    deletors.push_back(function);
  }

  void flush() {
    // reverse iterate the deletion queue to execute all the functions
    for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
      (*it)();  // call functors
    }

    deletors.clear();
  }
};

#endif  // ORION_UTIL_CONTAINERS_H_
