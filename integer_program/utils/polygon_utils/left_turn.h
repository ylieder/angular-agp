//
// Created by Yannic Lieder on 04.01.20.
//

#ifndef ANGULARARTGALLERYPROBLEM_LEFT_TURN_H
#define ANGULARARTGALLERYPROBLEM_LEFT_TURN_H

namespace utils {
    namespace polygon {
        static bool leftTurn(CGAL::Polygon_2::Vertex_const_circulator &ci) {
            return CGAL::left_turn(*(ci - 1), *ci, *(ci + 1));
        }
    }
}

#endif //ANGULARARTGALLERYPROBLEM_LEFT_TURN_H
