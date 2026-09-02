#include "orion/entry.h"

#include "orion/util/logger.h"

DECLARE_LOG_CATEGORY(SANDBOX);
void OE_init() {
  Logger::Get().SetMinVerbosity(LOG_LEVEL::TRACE);
  OE_LOG(SANDBOX, TRACE, "Sandbox initializing");
}

void OE_update(float delta_time) {
  // OE_LOG(SANDBOX, INFO, "Sandbox ticking with {} delta time", delta_time);
}

void OE_shutdown() { OE_LOG(SANDBOX, INFO, "Shutting down sandbox..."); }
