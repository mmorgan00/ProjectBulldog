#include "scripting.h"

#include "orion/entity/components/transform.h"

DECLARE_LOG_CATEGORY(SCRIPTING);
namespace scripting {
void Runtime::initialize(entt::registry* registry) {
  // Transformations
  this->registerNative(
      "set-rotation!",
      Callable{
          [registry](const Args& args) -> Value {
            requireArgs(args, 4, "set-rotation!");
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
  this->registerNative(
      "set-position!",
      Callable{
          [registry](const Args& args) -> Value {
            requireArgs(args, 4, "set-position!");
            auto& trans_cmp =
                registry->get<TransformComponent>(args[0].as_entity());
            trans_cmp.position = {
                static_cast<float>(args[1].as_number()),
                static_cast<float>(args[2].as_number()),
                static_cast<float>(args[3].as_number()),
            };

            return Value::null();
          },
      });
  this->registerNative(
      "get-position-x!",
      Callable{
          [registry](const Args& args) -> Value {
            requireArgs(args, 1, "get-position-x!");
            const TransformComponent& trans_cmp =
                registry->get<TransformComponent>(args[0].as_entity());
            return Value::num(trans_cmp.position.x);
          },
      });
  this->registerNative(
      "get-position-y!",
      Callable{
          [registry](const Args& args) -> Value {
            requireArgs(args, 1, "get-position-x!");
            const TransformComponent& trans_cmp =
                registry->get<TransformComponent>(args[0].as_entity());
            return Value::num(trans_cmp.position.y);
          },
      });

  this->registerNative(
      "get-position-z!",
      Callable{
          [registry](const Args& args) -> Value {
            requireArgs(args, 1, "get-position-x!");
            const TransformComponent& trans_cmp =
                registry->get<TransformComponent>(args[0].as_entity());
            return Value::num(trans_cmp.position.z);
          },
      });
  this->registerNative(
      "set-scale!", Callable{
                        [registry](const Args& args) -> Value {
                          requireArgs(args, 4, "set-scale!");
                          auto& trans_cmp = registry->get<TransformComponent>(
                              args[0].as_entity());
                          trans_cmp.scale = {
                              static_cast<float>(args[1].as_number()),
                              static_cast<float>(args[2].as_number()),
                              static_cast<float>(args[3].as_number()),
                          };

                          return Value::null();
                        },
                    });

  // Update
  // TODO: This needs to be revisted so that we only call an update function per
  // loop, and can support other things like a onSpawn!/onDestroy! type of fxns
  // this->registerNative("update!",
  //                      Callable{
  //                          [registry](const Args& args) -> Value {
  //                            requireArgs(args, 1, "update!");
  //                            return Value::null();
  //                          },
  //                      }

  // );
  // Arithmetic
  this->registerNative("mult!", Callable{
                                    [registry](const Args& args) -> Value {
                                      requireArgs(args, 2, "mult!");
                                      return Value::num(args[0].as_number() *
                                                        args[1].as_number());
                                    },
                                });
  this->registerNative("add!", Callable{
                                   [registry](const Args& args) -> Value {
                                     requireArgs(args, 2, "add!");
                                     return Value::num(args[0].as_number() +
                                                       args[1].as_number());
                                   },
                               });
  this->registerNative(
      "subtract!",
      Callable{
          [registry](const Args& args) -> Value {
            requireArgs(args, 2, "subtract!");
            return Value::num(args[0].as_number() - args[1].as_number());
          },
      });
};

std::vector<std::string> tokenize(const std::string& src) {
  std::vector<std::string> tokens;
  std::string cur_token;
  auto flush = [&] {
    if (!cur_token.empty()) {
      tokens.push_back(cur_token);
      cur_token.clear();
    }
  };
  for (char cur_char : src) {
    if (cur_char == '(' || cur_char == ')') {
      flush();
      tokens.emplace_back(1, cur_char);
    } else if (std::isspace(static_cast<unsigned char>(cur_char))) {
      flush();
    } else {
      cur_token += cur_char;
    }
  }
  flush();
  return tokens;
}
Node parse(const std::vector<std::string>& toks, size_t& pos) {
  if (pos >= toks.size()) throw ScriptError("unexpected end of input");
  const std::string& tok = toks[pos++];

  if (tok == "(") {
    Node list;
    list.kind = Node::Kind::List;
    while (pos < toks.size() && toks[pos] != ")") {
      list.children.push_back(parse(toks, pos));  // recursion handles nesting
    }
    if (pos >= toks.size()) throw ScriptError("missing ')'");
    ++pos;  // consume ')'
    return list;
  }
  if (tok == ")") throw ScriptError("unexpected ')'");

  Node atom;
  char* end = nullptr;
  double num = std::strtod(tok.c_str(), &end);
  if (*end == '\0') {
    atom.kind = Node::Kind::Number;
    atom.number = num;
  } else {
    atom.kind = Node::Kind::Symbol;
    atom.text = tok;
  }
  return atom;
}
Script::Script(File& filesource) {
  std::string src;
  filesource.read_text(src);
  auto toks = tokenize(src);
  size_t pos = 0;
  while (pos < toks.size()) {
    this->nodes.push_back(parse(toks, pos));
  }
}

Value lookup(std::string text, EvalContext ctx) {
  // it's a ~~boy!~~ function!
  if (text.ends_with('!')) {
    return Value::string(&text);
  }
  if (text == "self") {
    return Value::ent(ctx.self);
  }
  if (text == "delta_time") {
    return Value::num(ctx.delta_time);
  }
  if (text == "elapsed_time") {
    return Value::num(ctx.elapsed_time);
  }
  return Value::null();
}

Value Runtime::eval(const Node& node, EvalContext ctx) {
  switch (node.kind) {
    case Node::Kind::Number:
      OE_LOG(SCRIPTING, DEBUG, "NUMBER {}", node.number);
      return Value::num(node.number);
      // TODO: This needs the full wiring
    case Node::Kind::Symbol:
      OE_LOG(SCRIPTING, DEBUG, "SYMBOL {}", node.text);
      return lookup(node.text, ctx);  //`self`, variables, etc.
    case Node::Kind::List: {
      if (node.children.empty()) {
        return Value::null();
      }
      Args args;
      OE_LOG(SCRIPTING, DEBUG, "LIST");
      for (size_t i = 1; i < node.children.size(); ++i) {
        args.push_back(
            eval(node.children[i], ctx));  // nested calls resolve here
      }
      OE_LOG(SCRIPTING, DEBUG, "CALLING {}", node.children[0].text);
      return call(node.children[0].text, args);
    }
  }
  return Value::null();
}
}  // namespace scripting
