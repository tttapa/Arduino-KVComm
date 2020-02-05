#pragma once

#if !defined(ARDUINO) || defined(DOXYGEN)

#include <sstream>
#include <stdexcept>
#include <string>

/// @addtogroup KVComm
/// @{

/// Custom exception for KVComm. Not used on Arduino.
class KV_Exception : public std::exception {
  public:
    KV_Exception(const std::string message, int errorCode)
        : message(std::move(message)), errorCode(errorCode) {}
    const char *what() const throw() override { return message.c_str(); }
    int getErrorCode() const { return errorCode; }

  private:
    const std::string message;
    const int errorCode;
};

/// Throw an error.
#define KV_ERROR(msg, errc)                                                    \
    do {                                                                       \
        std::ostringstream s;                                                  \
        s << __PRETTY_FUNCTION__ << ":" << __LINE__ << msg;                    \
        throw KV_Exception(s.str(), errc);                                     \
    } while (0)

/// @}

#ifdef ARDUINO_TEST
#include <AH/PrintStream/PrintStream.hpp>
#else
#define F(x) (x)
#endif

#else

#include <AH/Error/Error.hpp>

#define KV_ERROR(msg, errc) ERROR(msg, errc)

#endif