/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COS_MACROS_H
#define LIBCOS_COS_MACROS_H

#define COS_PASTE(a, b) COS_PASTE_IMPL_(a, b)

#define COS_STRINGIFY(x) COS_STRINGIFY_IMPL_(x)

/**
 * @def COS_ARRAY_SIZE(array)
 *
 * @brief Returns the number of elements in the array.
 */
#define COS_ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

// MARK: - Macro implementations

#define COS_PASTE_IMPL_(a, b) a##b

#define COS_STRINGIFY_IMPL_(x) #x

#define COS_MIN(a, b) ((a) < (b) ? (a) : (b))

#endif /* LIBCOS_COS_MACROS_H */
