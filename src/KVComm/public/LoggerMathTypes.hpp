#pragma once

#if 0

#include <given/logger/public/LoggerTypes.hpp>

#include <math/Quaternion.hpp>  // Quaternion, EulerAngles
#include <math/Vector.hpp>      // Vec2f, Vec3f

/// @addtogroup logger
/// @{

/**
 * @file  
 * @brief   Logger type definitions for the math types (Vec2f, Vec3f, 
 *          Quaternion, EulerAngles) to make them loggable.
 */

/// Specialization to make Quaternion loggable.
template <>
struct LoggableType<Quaternion> {
    inline static uint8_t getTypeID() { return 100; }
    inline static size_t getLength() { return 4 * sizeof(float); }
    inline static void writeToBuffer(const Quaternion &q, uint8_t *buffer) {
        memcpy(buffer + 0 * sizeof(float), &q.w, sizeof(float));
        memcpy(buffer + 1 * sizeof(float), &q.x, sizeof(float));
        memcpy(buffer + 2 * sizeof(float), &q.y, sizeof(float));
        memcpy(buffer + 3 * sizeof(float), &q.z, sizeof(float));
    }
    inline static Quaternion readFromBuffer(const uint8_t *buffer) {
        Quaternion q;
        memcpy(&q.w, buffer + 0 * sizeof(float), sizeof(float));
        memcpy(&q.x, buffer + 1 * sizeof(float), sizeof(float));
        memcpy(&q.y, buffer + 2 * sizeof(float), sizeof(float));
        memcpy(&q.z, buffer + 3 * sizeof(float), sizeof(float));
        return q;
    }
};

/// Specialization to make EulerAngles loggable.
template <>
struct LoggableType<EulerAngles> {
    inline static uint8_t getTypeID() { return 101; }
    inline static size_t getLength() { return 3 * sizeof(float); }
    inline static void writeToBuffer(const EulerAngles &e, uint8_t *buffer) {
        memcpy(buffer + 0 * sizeof(float), &e.yaw, sizeof(float));
        memcpy(buffer + 1 * sizeof(float), &e.pitch, sizeof(float));
        memcpy(buffer + 2 * sizeof(float), &e.roll, sizeof(float));
    }
    inline static EulerAngles readFromBuffer(const uint8_t *buffer) {
        EulerAngles e;
        memcpy(&e.yaw, buffer + 0 * sizeof(float), sizeof(float));
        memcpy(&e.pitch, buffer + 1 * sizeof(float), sizeof(float));
        memcpy(&e.roll, buffer + 2 * sizeof(float), sizeof(float));
        return e;
    }
};

/// Specialization to make Vec2f loggable.
template <>
struct LoggableType<Vec2f> {
    inline static uint8_t getTypeID() { return 102; }
    inline static size_t getLength() { return 2 * sizeof(float); }
    inline static void writeToBuffer(const Vec2f &v, uint8_t *buffer) {
        memcpy(buffer + 0 * sizeof(float), &v.x, sizeof(float));
        memcpy(buffer + 1 * sizeof(float), &v.y, sizeof(float));
    }
    inline static Vec2f readFromBuffer(const uint8_t *buffer) {
        Vec2f v;
        memcpy(&v.x, buffer + 0 * sizeof(float), sizeof(float));
        memcpy(&v.y, buffer + 1 * sizeof(float), sizeof(float));
        return v;
    }
};

/// Specialization to make Vec3f loggable.
template <>
struct LoggableType<Vec3f> {
    inline static uint8_t getTypeID() { return 103; }
    inline static size_t getLength() { return 3 * sizeof(float); }
    inline static void writeToBuffer(const Vec3f &v, uint8_t *buffer) {
        memcpy(buffer + 0 * sizeof(float), &v.x, sizeof(float));
        memcpy(buffer + 1 * sizeof(float), &v.y, sizeof(float));
        memcpy(buffer + 2 * sizeof(float), &v.z, sizeof(float));
    }
    inline static Vec3f readFromBuffer(const uint8_t *buffer) {
        Vec3f v;
        memcpy(&v.x, buffer + 0 * sizeof(float), sizeof(float));
        memcpy(&v.y, buffer + 1 * sizeof(float), sizeof(float));
        memcpy(&v.z, buffer + 2 * sizeof(float), sizeof(float));
        return v;
    }
};

/// @}

#endif