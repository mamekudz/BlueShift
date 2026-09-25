#pragma once

// Evidence / implementation confidence labels (Milestone 2).
// Never mark VERIFIED without physical T-Lion confirmation.

namespace blueshift {

enum class Evidence : unsigned char {
    Verified = 0,
    Documented = 1,
    ImplementedUnverified = 2,
    Assumed = 3,
    Unknown = 4
};

} // namespace blueshift
