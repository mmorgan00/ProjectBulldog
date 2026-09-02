// Copyright 2025 Max Morgan
#ifndef ORION_ENTRY_H_
#define ORION_ENTRY_H_
#pragma once

// The stubs a new game must implement
extern "C" {
void OE_init();
void OE_update();
void OE_shutdown();
}

#endif  // ORION_ENTRY_H_
