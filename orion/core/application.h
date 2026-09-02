#ifndef ORION_CORE_APPLICATION_H_
#define ORION_CORE_APPLICATION_H

namespace orion {

// Interface that the game must implement. Not implemented anywhere inside orion
class IGame {
 public:
  virtual ~IGame() = default;
  virtual void Setup() = 0;
  virtual void Update(float delta_time) = 0;
  virtual void Shutdown() = 0;
};
}  // namespace orion
#endif  // ORION_CORE_APPLICATION_H_
