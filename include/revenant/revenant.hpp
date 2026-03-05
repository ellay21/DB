#pragma once

#include <revenant/config.hpp>
#include <revenant/errors.hpp>
#include <revenant/version.hpp>

namespace revenant {

/// Returns the library version string, e.g. "0.1.0".
const char* version_string() noexcept;

} // namespace revenant
