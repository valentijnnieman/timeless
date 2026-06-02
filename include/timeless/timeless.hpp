#pragma once

// ===========================================================================
// timeless/timeless.hpp — backward-compatible umbrella.
//
// Historically this header pulled in the entire engine, so it now simply
// forwards to <timeless/all.hpp> to keep existing #include "timeless/timeless.hpp"
// call sites working unchanged.
//
// For new code prefer:
//   #include "timeless/core.hpp"   // the TE:: facade + its direct deps
//   #include "timeless/components/..."  // only the modules you use
//   #include "timeless/systems/..."
// ...or <timeless/all.hpp> if you really want everything.
// ===========================================================================

#include "timeless/all.hpp"
