//
// Created by Yannic Lieder on 04.01.20.
//

#ifndef ANGULARARTGALLERYPROBLEM_ANGLE_H
#define ANGULARARTGALLERYPROBLEM_ANGLE_H

#include "left_turn.h"

namespace utils {
    namespace polygon {
        static Kernel::FT cosineAngle(CGAL::Polygon_2<Kernel>::Vertex_const_circulator &ci) {
            if (leftTurn(ci)) {
                return
            }
        }
    }
}

#endif //ANGULARARTGALLERYPROBLEM_ANGLE_H
