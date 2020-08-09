//
// Splits a polygon, given a segment through to polygon vertices.
//
// Created by Yannic Lieder on 02.08.20.
//

#ifndef ANGULAR_ART_GALLERY_PROBLEM_SPLIT_POLYGON_H
#define ANGULAR_ART_GALLERY_PROBLEM_SPLIT_POLYGON_H

#include <CGAL/Polygon_2.h>
#include <CGAL/Segment_2.h>


template <class Kernel>
using SplitContainer = std::vector<CGAL::Polygon_2<Kernel>>;

template <class Kernel>
struct SplitResult {
    SplitContainer<Kernel> left;
    SplitContainer<Kernel> right;
};

/**
 * Given a polygon and two vertex cirulators v1 and v2, the subpolygons on the right side of the segment v1-v2 are
 * returned (respectively on the left side, if the polygon is clockwise-oriented).
 *
 * \pre v1 != v2 && v1 + 1 != v2 && v1 - 1 != v2
 */
template <class Kernel>
SplitContainer<Kernel> split_right_side(
        CGAL::Polygon_2<Kernel> const & polygon,
        typename CGAL::Polygon_2<Kernel>::Vertex_const_circulator v1,
        typename CGAL::Polygon_2<Kernel>::Vertex_const_circulator v2) {

    CGAL::Segment_2<Kernel> split_segment = CGAL::Segment_2<Kernel>(*v1, *v2);
    SplitContainer<Kernel> subpolygons;

    auto current = v1;
    auto subpolygon_begin = v1;
    bool on_split_segment = true;

    do {
        if (split_segment.has_on(*current)) {
            if (on_split_segment) {
                subpolygon_begin = current;
            } else {
                subpolygons.emplace_back(subpolygon_begin, current + 1);
                subpolygon_begin = current;
            }
            on_split_segment = true;
        } else {
            on_split_segment = false;
        }
    } while (++current != v2 + 1);

    return subpolygons;
}

/**
 * Splits the polygon in subpolygons by the segment v1-v2 between polygon vertices v1 and v2 (defined by circulators).
 *
 * \pre Segment lies completely inside the polygon.
 */
template <class Kernel>
SplitResult<Kernel> split_polygon(
        CGAL::Polygon_2<Kernel> const & polygon,
        typename CGAL::Polygon_2<Kernel>::Vertex_const_circulator v1,
        typename CGAL::Polygon_2<Kernel>::Vertex_const_circulator v2) {

    assert (v1 != v2);

    if (v2 == (v1 + 1)) {
        SplitContainer<Kernel> left_side, right_side;
        left_side.push_back(polygon);
        return SplitResult<Kernel>{left_side, right_side};
    } else if (v1 == (v2 + 1)){
        SplitContainer<Kernel> left_side, right_side;
        right_side.push_back(polygon);
        return SplitResult<Kernel>{left_side, right_side};
    }

    auto right_side = split_right_side(polygon, v1, v2);
    auto left_side = split_right_side(polygon, v2, v1);
    return SplitResult<Kernel>{left_side, right_side};
}

/**
 * \pre v1 and v2 are vertices of the polygon.
 * \pre Segment lies completely inside the polygon.

 */
template <class Kernel>
SplitResult<Kernel> split_polygon(
        CGAL::Polygon_2<Kernel> const & polygon,
        CGAL::Point_2<Kernel> const & v1,
        CGAL::Point_2<Kernel> const & v2) {

    auto start = polygon.vertices_circulator();
    auto current = start;
    typename CGAL::Polygon_2<Kernel>::Vertex_const_circulator v1_circulator, v2_circulator;

    do {
        if (*current == v1) {
            v1_circulator = current;
        }
        if (*current == v2) {
            v2_circulator = current;
        }
    } while (++current != start);

    return split_polygon(polygon, v1_circulator, v2_circulator);
}



#endif //ANGULAR_ART_GALLERY_PROBLEM_SPLIT_POLYGON_H
