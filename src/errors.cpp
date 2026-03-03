#include <revenant/errors.hpp>
#include <revenant/revenant.hpp>
#include <revenant/version.hpp>

namespace revenant {

const char* version_string() noexcept {
    return REVENANT_VERSION_STRING;
}

const char* status_to_string(StatusCode code) noexcept {
    switch (code) {
    case StatusCode::Ok:               return "Ok";
    case StatusCode::InvalidArgument:  return "InvalidArgument";
    case StatusCode::NotFound:         return "NotFound";
    case StatusCode::AlreadyExists:    return "AlreadyExists";
    case StatusCode::PermissionDenied: return "PermissionDenied";
    case StatusCode::ResourceExhausted:return "ResourceExhausted";
    case StatusCode::WouldBlock:       return "WouldBlock";
    case StatusCode::Backpressure:     return "Backpressure";
    case StatusCode::Overrun:          return "Overrun";
    case StatusCode::Discontinued:     return "Discontinued";
    case StatusCode::Corrupt:          return "Corrupt";
    case StatusCode::VersionMismatch:  return "VersionMismatch";
    case StatusCode::LayoutMismatch:   return "LayoutMismatch";
    case StatusCode::RebootDetected:   return "RebootDetected";
    case StatusCode::Internal:         return "Internal";
    }
    return "Unknown";
}

} // namespace revenant
