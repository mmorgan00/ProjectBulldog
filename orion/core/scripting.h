#ifndef ORION_CORE_SCRIPTING_H_
#define ORION_CORE_SCRIPTING_H_

#include <functional>
#include <vector>
struct Value {
  enum class Type : uint8_t { Null, Number, String, Function, Boolean };
  Type type;
  union {
    double number;
    std::string* str;
    void* func_ptr;  // opaque pointer to actual callable
    bool boolean;
  };
};

using Args = std::vector<Value>;

struct Callable {
  std::function<Value(const Args&)> invoke;

  Value call(const Args& args) { return invoke(args); }
};

#endif  // ORION_CORE_SCRIPTING_H_
