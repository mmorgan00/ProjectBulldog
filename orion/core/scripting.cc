#include "scripting.h"

#include "orion/entity/components/transform.h"

DECLARE_LOG_CATEGORY(SCRIPTING);
namespace scripting {
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

std::vector<std::string> tokenize(const std::string& src) {
  std::vector<std::string> tokens;
  std::string cur;
  auto flush = [&] {
    if (!cur.empty()) {
      tokens.push_back(cur);
      cur.clear();
    }
  };
  for (char c : src) {
    if (c == '(' || c == ')') {
      flush();
      tokens.emplace_back(1, c);
    } else if (std::isspace(static_cast<unsigned char>(c))) {
      flush();
    } else {
      cur += c;
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

// void OCScriptable::load(File& filesource, entt::entity ent) {
//   std::string src;
//   filesource.read_text(src);
//   // Iterate over the script, parsing '(',')', and ops
//   std::string curr_symbol;
//   for (size_t i = 0; i < src.length(); i++) {
//     // Whitespace. end of a symbol
//     if (std::isspace(static_cast<unsigned char>(src[i]))) {
//       if (!curr_symbol.empty()) {
//         //
//       }
//       OE_LOG(SCRIPTING, DEBUG, "Parser found whitespace");
//       continue;
//     }
//     // `(` - start of an expression
//     if (src[i] == '(') {
//       OE_LOG(SCRIPTING, DEBUG, "Parser found start of expression");
//       continue;
//     }
//     if (src[i] == ')') {
//       OE_LOG(SCRIPTING, DEBUG, "Parser found end of expression");
//       continue;
//     }
//     // Otherwise, Value. `!` will indicate expression, so have to read full
//     word OE_LOG(SCRIPTING, DEBUG, "Parser found Value {}", src[i]);
//   }
// };

Value lookup(std::string text, entt::entity ent) {
  // it's a ~~boy!~~ function!
  if (text.ends_with('!')) {
    return Value::string(&text);
  }
  if (text == "self") {
    return Value::ent(ent);
  }
  return Value::null();
}

Value Runtime::eval(const Node& node, entt::entity self) {
  switch (node.kind) {
    case Node::Kind::Number:
      OE_LOG(SCRIPTING, DEBUG, "NUMBER {}", node.number);
      return Value::num(node.number);
      // TODO: This needs the full wiring
    case Node::Kind::Symbol:
      OE_LOG(SCRIPTING, DEBUG, "SYMBOL {}", node.text);
      return lookup(node.text, self);  // `self`, variables, etc.
    case Node::Kind::List: {
      if (node.children.empty()) {
        return Value::null();
      }
      Args args;
      OE_LOG(SCRIPTING, DEBUG, "LIST");
      for (size_t i = 1; i < node.children.size(); ++i) {
        args.push_back(eval(node.children[i], self));  // nested calls resolve here
      }
      OE_LOG(SCRIPTING, DEBUG, "CALLING {}", node.children[0].text);
      return call(node.children[0].text, args);
    }
  }
  return Value::null();
}
}  // namespace scripting
