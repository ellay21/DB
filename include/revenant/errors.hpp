#pragma once

#include <cstdint>
#include <optional>
#include <string_view>
#include <system_error>

namespace revenant {

enum class StatusCode : std::uint32_t {
    Ok = 0,
    InvalidArgument,
    NotFound,
    AlreadyExists,
    PermissionDenied,
    ResourceExhausted,
    WouldBlock,
    Backpressure,
    Overrun,
    Discontinued,
    Corrupt,
    VersionMismatch,
    LayoutMismatch,
    RebootDetected,
    Internal,
};

const char* status_to_string(StatusCode code) noexcept;

class Status {
  public:
    Status() noexcept = default;
    explicit Status(StatusCode code, std::string_view msg = {}) noexcept : code_(code), msg_(msg) {}

    [[nodiscard]] bool ok() const noexcept { return code_ == StatusCode::Ok; }
    [[nodiscard]] StatusCode code() const noexcept { return code_; }
    [[nodiscard]] std::string_view message() const noexcept { return msg_; }

    static Status success() noexcept { return {}; }

    explicit operator bool() const noexcept { return ok(); }

  private:
    StatusCode code_ = StatusCode::Ok;
    std::string_view msg_{};
};

template <typename T> class Result {
  public:
    Result(T value) noexcept(noexcept(T(std::move(value)))) : value_(std::move(value)), status_() {}

    Result(Status status) noexcept : status_(status) {}

    [[nodiscard]] bool ok() const noexcept { return status_.ok(); }
    [[nodiscard]] const Status& status() const noexcept { return status_; }

    [[nodiscard]] const T& value() const& noexcept { return *value_; }
    [[nodiscard]] T& value() & noexcept { return *value_; }
    [[nodiscard]] T&& value() && noexcept { return std::move(*value_); }

    explicit operator bool() const noexcept { return ok(); }

  private:
    std::optional<T> value_;
    Status status_;
};

} // namespace revenant
