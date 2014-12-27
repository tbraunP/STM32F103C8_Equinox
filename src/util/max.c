#include "util/max.h"

/**
 * @brief max
 * If criterion is fullfilled return max(first, second) else first
 * @param criterion - criterion
 * @param first - first element (standard if criterion not fullfilled)
 * @param second - second element
 * @return If criterion is fullfilled return max(first, second) else first
 */
float max(bool criterion, float first, float second){
    if(criterion){
        return ((first > second) ? first : second);
    }
    return first;
}
