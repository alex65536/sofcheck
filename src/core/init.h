#ifndef SOF_CORE_INIT_INCLUDED
#define SOF_CORE_INIT_INCLUDED

namespace SoFCore {

// Initialization routine, must be called once before other methods in `SoFCore` are used.
//
// It is disregarded to use this function during static initialization.
void init();

}  // namespace SoFCore

#endif  // SOF_CORE_INIT_INCLUDED
