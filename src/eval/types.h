#ifndef SOF_EVAL_TYPES_INCLUDED
#define SOF_EVAL_TYPES_INCLUDED

namespace SoFEval {

// Base struct to define helper types and methods for your score type (e. g. define a type to
// represent score pair). To do this, you must specialize this struct.
template <typename T>
struct ScoreTraits {};

}  // namespace SoFEval

#endif  // SOF_EVAL_TYPES_INCLUDED
