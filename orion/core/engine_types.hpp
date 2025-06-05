#pragma once

#include "simdjson.h"
#include <string>

using namespace simdjson;
typedef struct app_state {
  std::string appName;
  std::string graphicsAPI;

  void build(ondemand::document &data) {
    auto appNameValue = data["appName"].get_string();
    if (!appNameValue.error()) {
      this->appName = std::string(appNameValue.value());
    } else {
      throw std::runtime_error("Failed to parse appName or it is not a string");
    }

    auto graphicsAPIValue = data["graphicsAPI"].get_string();
    if (!graphicsAPIValue.error()) {
      this->graphicsAPI = std::string(graphicsAPIValue.value());
    } else {
      throw std::runtime_error(
          "Failed to parse graphicsAPI or it is not a string");
    }
  }
} app_state;
