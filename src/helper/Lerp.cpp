#include "Lerp.h"

int lerp(int current, int target, unsigned long unitsPerSecond, unsigned long timePassedInMillis) {
    timePassedInMillis = max(timePassedInMillis, 1);
    if (current == target) {
        return target;
    }
    int change = max(round(unitsPerSecond * (timePassedInMillis / 1000.f)), 1);

    if (current < target) {
        return min(current + change, target);
    } else {
        return max(current - change, target);
    }

}