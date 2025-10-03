// Copyright 2025 Max Morgan

#ifndef ORION_CORE_ENGINE_TYPES_H_
#define ORION_CORE_ENGINE_TYPES_H_
#include <simdjson.h>

#include <string>

constexpr int MAX_CONCURRENT_FRAMES = 2;

typedef struct app_state {
  std::string appName;
  std::string graphicsAPI;

  void build(simdjson::ondemand::document &data) {
    auto appNameValue = data["appName"].get_string();
    if (!appNameValue.error()) {
      this->appName = appNameValue.value();
    } else {
      throw std::runtime_error("Failed to parse appName or it is not a string");
    }

    auto graphicsAPIValue = data["graphicsAPI"].get_string();
    if (!graphicsAPIValue.error()) {
      this->graphicsAPI = graphicsAPIValue.value();
    } else {
      throw std::runtime_error(
          "Failed to parse graphicsAPI or it is not a string");
    }
  }
} app_state;

#endif  // ORION_CORE_ENGINE_TYPES_H_
