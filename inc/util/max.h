#ifndef MAX_H
#define MAX_H

#include <stdbool.h>

#ifdef __cplusplus
 extern "C" {
#endif

/**
 * @brief max
 * If criterion is fullfilled return max(first, second) else first
 * @param criterion - criterion
 * @param first - first element (standard if criterion not fullfilled)
 * @param second - second element
 * @return If criterion is fullfilled return max(first, second) else first
 */
float max(bool criterion, float first, float second);


#ifdef __cplusplus
 }
#endif

#endif // MAX_H

