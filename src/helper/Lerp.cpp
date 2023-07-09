#include "Lerp.h"

int lerp(int current, int target, unsigned long unitsPerSecond, unsigned long timePassedInMillis) {
    if (current == target) {
        return target;
    }
    int change = round(unitsPerSecond * (timePassedInMillis / 1000.f));

    if (current < target) {
        return min(current + change, target);
    } else {
        return max(current - change, target);
    }

}