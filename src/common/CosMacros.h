//
// Created by david on 16/12/23.
//

#ifndef LIBCOS_COS_MACROS_H
#define LIBCOS_COS_MACROS_H

/**
 * @def COS_ARRAY_SIZE(array)
 *
 * @brief Returns the number of elements in the array.
 */
#define COS_ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

#endif /* LIBCOS_COS_MACROS_H */
