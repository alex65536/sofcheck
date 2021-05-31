#ifndef SOF_UTIL_NO_COPY_MOVE_INCLUDED
#define SOF_UTIL_NO_COPY_MOVE_INCLUDED

namespace SoFUtil {

// Inherit from this class if you want to forbid copying the object
struct NoCopy {
  NoCopy() = default;
  NoCopy(const NoCopy &) = delete;
  NoCopy(NoCopy &&) noexcept = default;
  NoCopy &operator=(const NoCopy &) = delete;
  NoCopy &operator=(NoCopy &&) noexcept = default;
};

// Inherit from this class if you want to forbid copying and moving the object
struct NoCopyMove {
  NoCopyMove() = default;
  NoCopyMove(const NoCopyMove &) = delete;
  NoCopyMove(NoCopyMove &&) = delete;
  NoCopyMove &operator=(const NoCopyMove &) = delete;
  NoCopyMove &operator=(NoCopyMove &&) = delete;
};

}  // namespace SoFUtil

#endif  // SOF_UTIL_NO_COPY_MOVE_INCLUDED
