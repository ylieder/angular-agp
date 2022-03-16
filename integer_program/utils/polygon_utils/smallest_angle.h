//
// Created by Yannic Lieder on 04.01.20.
//

#ifndef ANGULARARTGALLERYPROBLEM_SMALLEST_ANGLE_H
#define ANGULARARTGALLERYPROBLEM_SMALLEST_ANGLE_H

#include "left_turn.h"

namespace utils {
    namespace polygon {
        template <typename Kernel>
        static void smallestAngleVertex(CGAL::Polygon_2<Kernel> &polygon) {
            if (polygon.size() < 3) {
                assert(false);
            }

            if (polygon.is_clockwise_oriented()) {
                assert(false);
            }

            auto ci = polygon.vertices_circulator();
            auto end = ci;

            auto currentSmallest = ci;
            auto currentCosine = cosineAngle(ci);

            do {
                if (leftTurn(ci)) {
                    if ()
                }
            } while (++ci != end);
        }
    }
}

#endif //ANGULARARTGALLERYPROBLEM_SMALLEST_ANGLE_H
