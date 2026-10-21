#ifndef ORION_CORE_SCRIPTING_H_
#define ORION_CORE_SCRIPTING_H_

#include <entt/entt.hpp>
#include <forward_list>
#include <functional>
#include <stack>
#include <vector>

#include "orion/util/filesystem.h"

namespace scripting {
struct ScriptError : std::runtime_error {
  using std::runtime_error::runtime_error;
};
struct Value {
  enum class Type : uint8_t { Null, Number, String, Function, Boolean, Entity };
  Type type = Type::Null;
  union {
    double number;
    std::string* str;
    bool boolean;
    uint32_t entity;  // underlying pointer to an ECS entity
  };

  Value() : number(0.0) {}
  static Value null() {
    Value val;
    val.type = Type::Null;
    return val;
  };
  static Value string(std::string* rawStr) {
    Value val;
    val.type = Type::String;
    val.str = rawStr;
    return val;
  }
  static Value num(double dbl) {
    Value val;
    val.type = Type::Number;
    val.number = dbl;
    return val;
  }
  static Value ent(entt::entity entity) {
    Value val;
    val.type = Type::Entity;
    val.entity = static_cast<uint32_t>(entity);
    return val;
  }
  double as_number() const {
    if (type != Type::Number) {
      throw ScriptError("expected number");
    }
    return number;
  }
  entt::entity as_entity() const {
    if (type != Type::Entity) {
      throw ScriptError("expected entity");
    }
    return static_cast<entt::entity>(entity);
  }
};
struct CallEntry {
  std::string fnName;
  std::forward_list<Value> params;
};

using Args = std::vector<Value>;

inline void require_args(const Args& args, size_t n, const char* fxn) {
  if (args.size() != n) {
    throw ScriptError(std::string(fxn) + ": expected " + std::to_string(n) +
                      " args, got " + std::to_string(args.size()));
  }
}

struct Callable {
  std::function<Value(const Args&)> invoke;
};
struct Node {
  enum class Kind : uint8_t { Number, Symbol, List } kind = Kind::Symbol;
  double number = 0.0;
  std::string text;            // symbol name
  std::vector<Node> children;  // only for List
};
class Runtime {
 public:
  void initialize(entt::registry* registry);
  void register_native(std::string name, Callable fxn) {
    natives_[std::move(name)] = std::move(fxn);
  };
  Value eval(const Node& node, entt::entity self);
  Value call(const std::string& name, const Args& args) {
    return natives_.at(name).invoke(args);
  }

 private:
  std::unordered_map<std::string, Callable> natives_;
};

// A parsed script
struct Script {
  std::vector<Node> nodes;
  explicit Script(File& filesource);
  Script() = default;
};
class OCScriptable {
  Script script;

 public:
  void reload();
  void run();
};
}  // namespace scripting
#endif  // ORION_CORE_SCRIPTING_H_
