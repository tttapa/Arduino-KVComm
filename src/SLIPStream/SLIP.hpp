#pragma once

#include <stdint.h> // uint8_t

/// SLIP special character codes.
namespace SLIP_Constants {
const static uint8_t END     = 0300; ///< indicates end of packet
const static uint8_t ESC     = 0333; ///< indicates byte stuffing
const static uint8_t ESC_END = 0334; ///< ESC ESC_END means END data byte
const static uint8_t ESC_ESC = 0335; ///< ESC ESC_ESC means ESC data byte
} // namespace SLIP_Constants
