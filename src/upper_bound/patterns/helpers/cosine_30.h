//
// Returns the cosine of a multiple of pi/6.
//
// Created by Yannic Lieder on 02.08.20.
//

#ifndef ANGULAR_ART_GALLERY_PROBLEM_COSINE_30_H
#define ANGULAR_ART_GALLERY_PROBLEM_COSINE_30_H

#include "kernel_definitions.h"


FT cosine_30(int k) {
    int sign = k % 12 < 4 || k % 12 > 8 ? 1 : -1;
    switch (k % 6) {
        case 0: // 0°, 180°
            return FT(1) * sign;
        case 1: // 30°
        case 5: // 150°
            return CGAL::sqrt(FT(3)) / 2 * sign;
        case 2: // 60°
        case 4: // 120°
            return FT(1) / 2 * sign;
        case 3: // 90°
        default:
            return FT(0);
    }
}

#endif //ANGULAR_ART_GALLERY_PROBLEM_COSINE_30_H
