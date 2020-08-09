//
// Function for polygon normalization (remove collinearities and force counterclockwise orientation).
//
// Created by Yannic Lieder on 03.08.20.
//

#ifndef ANGULAR_ART_GALLERY_PROBLEM_POLYGON_NORMALIZATION_H
#define ANGULAR_ART_GALLERY_PROBLEM_POLYGON_NORMALIZATION_H

#include <CGAL/Kernel/global_functions_2.h>
#include <CGAL/Polygon_2.h>


/**
 * \pre Polygon is simple and has at least three vertices.
 */
template <class Kernel>
bool is_normalized(CGAL::Polygon_2<Kernel> const & polygon) {
    if (polygon.is_clockwise_oriented()) {
        return false;
    }

    auto start = polygon.vertices_circulator();
    auto current = start;
    do {
        if (CGAL::collinear(*(current - 1), *current, *(current + 1))) {
            return false;
        }
    } while (++current != start);

    return true;
}

/**
 * \pre Polygon is simple and has at least three vertices.
 */
template <class Kernel>
CGAL::Polygon_2<Kernel> normalize_polygon(CGAL::Polygon_2<Kernel> const & polygon) {
    CGAL::Polygon_2<Kernel> normalized;

    auto start = polygon.vertices_circulator();
    auto current = start;
    do {
        if (!CGAL::collinear(*(current - 1), *current, *(current + 1))) {
            normalized.push_back(*current);
        }
    } while (++current != start);

    if (normalized.is_clockwise_oriented()) {
        normalized.reverse_orientation();
    }

    return normalized;
}

#endif //ANGULAR_ART_GALLERY_PROBLEM_POLYGON_NORMALIZATION_H
