#ifndef SOF_EVAL_TYPES_INCLUDED
#define SOF_EVAL_TYPES_INCLUDED

namespace SoFEval {

// Base struct to define helper types and methods for your score type (e. g. define a type to
// represent score pair). You must specialize this struct when you implement a score type.
template <typename T>
struct ScoreTraits {};

}  // namespace SoFEval

#endif  // SOF_EVAL_TYPES_INCLUDED
