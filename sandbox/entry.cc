#include <util/logger.h>

DECLARE_LOG_CATEGORY(SANDBOX);
void init() {
  Logger::Get().SetMinVerbosity(LOG_LEVEL::TRACE);
  OE_LOG(SANDBOX, TRACE, "Sandbox demo initializing");
}

void run() {
  OE_LOG(SANDBOX, INFO, "Sandbox demo starting");
}
