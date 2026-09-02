#ifndef ORION_UTIL_SCENES_H_
#define ORION_UTIL_SCENES_H_

#include <filesystem>

#include "orion/util/logger.h"
#include "simdjson.h"
struct SceneNode {};

DECLARE_LOG_CATEGORY(SCENE_LOADER);
class SceneLoader {
 public:
  static bool validate(const std::string_view& content) {
    simdjson::padded_string padded(content);
    simdjson::ondemand::parser parser;

    simdjson::ondemand::document scene;
    auto error = parser.iterate(padded).get(scene);

    if (error) {
      OE_LOG(SCENE_LOADER, ERROR, "JSON parse failed: {}",
             simdjson::error_message(error));
      return false;
    }

    // Check version field
    if (auto ver = scene["version"].get_string(); ver.error()) {
      OE_LOG(SCENE_LOADER, WARN, "Missing 'version' field");
    }

    // Check entities field exists and is an array
    auto entities_result = scene["entities"].get_array();
    if (entities_result.error()) {
      OE_LOG(SCENE_LOADER, ERROR, "Missing 'entities' or wrong type: {}",
             simdjson::error_message(entities_result.error()));
      return false;  // Return false, don't throw!
    }

    return true;
  }
  static bool validateFromFilePath(const std::filesystem::path& path) {
    return true;
  };
  std::vector<SceneNode> parse(std::string_view data) {
    // TODO: Load from file to content
    std::string_view demo_scene = R"(
    {
        "version": "1.0",
        "sceneName": "DemoLevel",
        "entities": [
            {
                "name": "TestCube",
                "type": "primitive",
                "primitive": "cube"
            }
        ]})";
    // TODO: Remove this
    data = demo_scene;

    simdjson::padded_string padded(data);

    if (!validate(data)) {
      OE_LOG(SCENE_LOADER, WARN, "Scene load found no valid nodes!");
      return std::vector<SceneNode>();
    }
    simdjson::ondemand::parser parser;
    simdjson::ondemand::document scene;
    auto error = parser.iterate(padded).get(scene);

    if (error) {
      OE_LOG(SCENE_LOADER, ERROR, "JSON parse failed: {}",
             simdjson::error_message(error));
      return std::vector<SceneNode>();
    }

    auto entities_result = scene["entities"].get_array();
    for (auto entity : entities_result) {
      auto obj = entity.get_object();
      if (obj.error()) {
        OE_LOG(SCENE_LOADER, WARN, "Entity is not an object");
        continue;
      }

      auto name_result = obj.value()["name"].get_string();
      if (name_result.error()) {
        OE_LOG(SCENE_LOADER, DEBUG, "Entity missing 'name' field");
        continue;
      }

      std::string name = std::string(name_result.value());

      auto type_result = obj.value()["type"].get_string();
      if (!type_result.error()) {
        std::string type = std::string(type_result.value());
        OE_LOG(SCENE_LOADER, DEBUG, "Found entity: {} (type: {})", name, type);
      } else {
        OE_LOG(SCENE_LOADER, DEBUG, "Found entity: {}", name);
      }
    }
    return std::vector<SceneNode>();
  };
};

#endif  // ORION_UTIL_SCENESE_H_
