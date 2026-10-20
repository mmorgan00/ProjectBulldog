#ifndef ORION_CORE_SCRIPTING_H_
#define ORION_CORE_SCRIPTING_H_

#include <entt/entt.hpp>
#include <functional>
#include <stack>
#include <vector>

#include "orion/util/filesystem.h"

struct ScriptError : std::runtime_error {
  using std::runtime_error::runtime_error;
};
struct Value {
  enum class Type : uint8_t { Null, Number, String, Function, Boolean, Entity };
  Type type = Type::Null;
  union {
    double number;
    std::string* str;
    void* func_ptr;  // opaque pointer to actual callable
    bool boolean;
    uint32_t entity;  // underlying pointer to an ECS entity
  };

  Value() : number(0.0) {}
  static Value null() {
    Value val;
    val.type = Type::Null;
    return val;
  };
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

class Runtime {
 public:
  void initialize(entt::registry* registry);
  void register_native(std::string name, Callable fxn) {
    natives_[std::move(name)] = std::move(fxn);
  };
  Value call(const std::string& name, const Args& args) {
    return natives_.at(name).invoke(args);
  }

 private:
  std::unordered_map<std::string, Callable> natives_;
};

// TODO: Scriptable component, has a 'call stack' stack (literally) that gets
// built at load time and then evaluated for each 'update' invocation

// TODO: You should write some documentation
class OCScriptable {
  std::stack<Callable> call_stack;
  std::unordered_map<std::string, Value> variables;
  File source;

 public:
  static void load(File& filesource, entt::entity ent);
  void reload();
  void run();
};

#endif  // ORION_CORE_SCRIPTING_H_
