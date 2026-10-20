#include "scripting.h"

#include "orion/entity/components/transform.h"

DECLARE_LOG_CATEGORY(SCRIPTING);
void Runtime::initialize(entt::registry* registry) {
  this->register_native(
      "set-rotation!",
      Callable{
          [registry](const Args& args) -> Value {
            require_args(args, 4, "set-rotation!");
            auto& trans_cmp =
                registry->get<TransformComponent>(args[0].as_entity());
            trans_cmp.rotation = {
                static_cast<float>(args[1].as_number()),
                static_cast<float>(args[2].as_number()),
                static_cast<float>(args[3].as_number()),
            };
            return Value::null();
          },
      });
  this->register_native("update!",
                        Callable{
                            [registry](const Args& args) -> Value {
                              require_args(args, 1, "update!");
                              // OE_LOG(SCRIPTING, DEBUG, "Update called for
                              // {}",
                              //        static_cast<uint32_t>(args[0].as_entity()));
                              return Value::null();
                            },
                        }

  );
};

void OCScriptable::load(File& filesource, entt::entity ent) {
  std::string src;
  filesource.read_text(src);
  // Iterate over the script, parsing '(',')', and ops
  // std::string curr_symbol;
  for (size_t i = 0; i < src.length(); i++) {
    // Whitespace
    if (std::isspace(static_cast<unsigned char>(src[i]))) {
      OE_LOG(SCRIPTING, DEBUG, "Parser found whitespace");
      continue;
    }
    // `(` - start of an expression
    if (src[i] == '(') {
      OE_LOG(SCRIPTING, DEBUG, "Parser found start of expression");
      continue;
    }
    if (src[i] == ')') {
      OE_LOG(SCRIPTING, DEBUG, "Parser found end of expression");
      continue;
    }
    // Otherwise, Value. `!` will indicate expression, so have to read full word
    OE_LOG(SCRIPTING, DEBUG, "Parser found Value {}", src[i]);
  }
};
