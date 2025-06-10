# Style Guide Summary

Some standard is better than no standard.
The Orion engine will follow the [Google C++ style guide](https://google.github.io/styleguide/cppguide.html)

## Naming
- **Classes/Structs**: `UpperCamelCase` (e.g., `GameEngine`, `PhysicsSystem`)
- **Functions/Methods**: `camelCase` (e.g., `updatePhysics`, `renderFrame`)
- **Variables**: `snake_case` (e.g., `player_position`, `frame_rate`)
- **Constants**: `kConstantName` (e.g., `kMaxFrameRate`)
- **Namespaces**: `snake_case` (e.g., `game_engine::rendering`)

## Formatting
- **Indentation**: 2 spaces, no tabs.
- **Line Length**: Max 80 characters.
- **Braces**: Use Egyptian-style braces (opening brace on same line):
  ```cpp
  class ActorPlayer {
    void onGameStart() {
      // Code here
    }
  };

## Filetypes
- **Source**: .cc file types
- **Header**: .h file types


## Others
- Google self-owns their 'Style' guide is a lacking title:
> Style, also known as readability, is what we call the conventions that govern our C++ code. The term Style is a bit of a misnomer, since these conventions cover far more than just source file formatting.
