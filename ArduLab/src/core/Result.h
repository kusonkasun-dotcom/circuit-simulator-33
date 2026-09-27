#pragma once
#include <QString>
#include <variant>
#include <utility>

namespace ardulab {

// A domain error with a stable machine-readable code and a human message.
struct Error {
    QString code;
    QString message;
    Error() = default;
    Error(QString c, QString m) : code(std::move(c)), message(std::move(m)) {}
};

// Result<T> : holds either a value of type T or an Error. Used across the
// domain layer so failures are explicit and never thrown across module
// boundaries.
template <typename T>
class Result {
public:
    Result(T value) : data_(std::move(value)) {}
    Result(Error error) : data_(std::move(error)) {}

    static Result<T> ok(T value) { return Result<T>(std::move(value)); }
    static Result<T> fail(QString code, QString message) {
        return Result<T>(Error(std::move(code), std::move(message)));
    }

    bool isOk() const { return std::holds_alternative<T>(data_); }
    bool isError() const { return std::holds_alternative<Error>(data_); }
    explicit operator bool() const { return isOk(); }

    const T& value() const { return std::get<T>(data_); }
    T& value() { return std::get<T>(data_); }
    const Error& error() const { return std::get<Error>(data_); }

private:
    std::variant<T, Error> data_;
};

// Status : a Result with no payload, for operations that only succeed or fail.
class Status {
public:
    Status() : ok_(true) {}
    Status(Error e) : ok_(false), error_(std::move(e)) {}

    static Status ok() { return Status(); }
    static Status fail(QString code, QString message) {
        return Status(Error(std::move(code), std::move(message)));
    }

    bool isOk() const { return ok_; }
    bool isError() const { return !ok_; }
    explicit operator bool() const { return ok_; }
    const Error& error() const { return error_; }

private:
    bool ok_;
    Error error_;
};

} // namespace ardulab
