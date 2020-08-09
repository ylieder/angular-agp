//
// Defines an angle class. Exact angle comparisons are possible, if kernel with sqrt is used and online cosines are
// compared.
//
// Created by Yannic Lieder on 02.08.20.
//

#ifndef ANGULAR_ART_GALLERY_PROBLEM_ANGLE_H
#define ANGULAR_ART_GALLERY_PROBLEM_ANGLE_H

#include <CGAL/Kernel/global_functions_2.h>
#include <CGAL/Point_2.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Vector_2.h>

#include "interval_arithmetic.h"


template <class Kernel>
class Angle {
public:
    Angle(CGAL::Point_2<Kernel> const & p1, CGAL::Point_2<Kernel> const & p2, CGAL::Point_2<Kernel> const & p3)
        : p1(p1), p2(p2), p3(p3) {
        init();
    }

    Angle(typename CGAL::Polygon_2<Kernel>::Vertex_const_circulator const & v)
        : p1(*(v - 1)), p2(*v), p3(*(v + 1)) {
        init();
    }

    void init() {
        assert(p1 != p2 && p1 != p3 && p2 != p3);
        convex = CGAL::left_turn(p1, p2, p3);
    }

    bool is_convex() const {
        return convex;
    }

    typename Kernel::FT cosine() const {
        CGAL::Vector_2<Kernel> v1 = p3 - p2;
        CGAL::Vector_2<Kernel> v2 = p1 - p2;
        return (v1 * v2) / (CGAL::sqrt(v1.squared_length()) * CGAL::sqrt(v2.squared_length()));
    }

    ia::DoubleInterval interval() const {
        return ia::acos(ia::get<Kernel>(cosine()));
    }

private:
    CGAL::Point_2<Kernel> p1, p2, p3;
    bool convex;
};

/**
 * Returns the cosine of the smalles inner angle of a given polygon.
 *
 * \pre Polygon is counterclockwise oriented.
 */
template <class Kernel>
typename Kernel::FT smallest_inner_angle_cos(CGAL::Polygon_2<Kernel> const & polygon) {
    auto start = polygon.vertices_circulator();
    auto current = start;

    typename Kernel::FT largest_cos(0);

    do {
        auto angle = Angle<Kernel>(current);
        if (!angle.is_convex()) {
            continue;
        }
        typename Kernel::FT cosine = angle.cosine();
        if (cosine > largest_cos) {
            largest_cos = cosine;
        }
    } while (++current != start);

    return largest_cos;
}

#endif //ANGULAR_ART_GALLERY_PROBLEM_ANGLE_H
