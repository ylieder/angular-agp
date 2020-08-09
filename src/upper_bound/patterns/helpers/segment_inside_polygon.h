//
// Segment inside polygon test.
//
// Created by Yannic Lieder on 20.07.20.
//

#ifndef ANGULAR_ART_GALLERY_PROBLEM_SEGMENT_INSIDE_POLYGON_H
#define ANGULAR_ART_GALLERY_PROBLEM_SEGMENT_INSIDE_POLYGON_H

#include <CGAL/intersection_2.h>
#include <CGAL/Kernel/global_functions_2.h>
#include <CGAL/Point_2.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_2_algorithms.h>


/**
 * Returns the direction of the Segment defined by the points source and target.
 */
template <class Kernel>
CGAL::Direction_2<Kernel> direction(CGAL::Point_2<Kernel> const & source, CGAL::Point_2<Kernel> const & target) {
    return CGAL::Direction_2<Kernel>(CGAL::Segment_2<Kernel>(source, target));
}

/**
 * Returns true, iff direction d is counterclockwise in between d1 and d2 or d equlas either d1 or d2.
 */
template <class Kernel>
bool counterclockwise_in_between_or_equal(
        CGAL::Direction_2<Kernel> const &d,
        CGAL::Direction_2<Kernel> const &d1,
        CGAL::Direction_2<Kernel> const &d2
) {
    return d == d1 || d == d2 || d.counterclockwise_in_between(d1, d2);
}

/**
 * Returns true, iff the segment is inside the polygon (or on its boundary).
 */
template<class Kernel>
bool segment_inside_polygon(CGAL::Polygon_2<Kernel> const &polygon, CGAL::Segment_2<Kernel> const &segment) {
    auto start = polygon.edges_circulator();
    auto current_edge = start;
    do {
        auto result = CGAL::intersection(*current_edge, segment);

        if (!result) {
            // no intersection with current edge
            continue;
        }
        if (const CGAL::Point_2<Kernel> *p = boost::get<CGAL::Point_2<Kernel>>(&*result)) {
            // Intersect in one point
            if (*p == segment.source() || *p == segment.target()) {
                // Intersection is an endpoint of the segment
                continue;
            }

            CGAL::Direction_2<Kernel> d1, d2;
            if (*p == current_edge->target()) {
                auto next_edge = current_edge + 1;
                d1 = direction(next_edge->source(), next_edge->target());
                d2 = direction(current_edge->target(), current_edge->source());
            } else if (*p == current_edge->source()) {
                auto prev_edge = current_edge - 1;
                d1 = direction(current_edge->source(), current_edge->target());
                d2 = direction(prev_edge->target(), prev_edge->source());
            } else {
                // Intersection point is neither endpoint of the current edge nor the segment ('real' intersection)
                return false;
            }

            // Intersects current edge at an endpoint. Check if segment just touches the vertex or intersects it
            if (
                    counterclockwise_in_between_or_equal(direction(*p, segment.source()), d1, d2) &&
                    counterclockwise_in_between_or_equal(direction(*p, segment.target()), d1, d2)
                    ) {
                // Segment only touches the boundary
                continue;
            }
            // Segment intersects the boundary
            return false;
        } else {
            // Segment is part of current edge
            if (current_edge->has_on(segment.source()) and current_edge->has_on(segment.target())) {
                // Segment lies completely on current edge
                return true;
            }

            CGAL::Point_2<Kernel> const *s_source;
            CGAL::Point_2<Kernel> const *s_target;
            if (current_edge->direction() == segment.direction()) {
                s_source = &segment.source();
                s_target = &segment.target();
            } else {
                s_source = &segment.target();
                s_target = &segment.source();
            }

            if (
                    !current_edge->has_on(*s_source) and
                    !CGAL::right_turn((current_edge - 1)->source(), current_edge->source(), current_edge->target())
                    ) {
                return false;
            }

            if (
                    !current_edge->has_on(*s_target) and
                    !CGAL::right_turn(current_edge->source(), current_edge->target(), (current_edge + 1)->target())
                    ) {
                return false;
            }
        }
    } while (++current_edge != start);

    CGAL::Point_2<Kernel> s_center = segment.source() + CGAL::Vector_2<Kernel>(segment) * 0.5;
    CGAL::Bounded_side bounded_side = CGAL::bounded_side_2(
            polygon.vertices_begin(), polygon.vertices_end(), s_center, Kernel()
    );

    return bounded_side == CGAL::ON_BOUNDED_SIDE || bounded_side == CGAL::ON_BOUNDARY;
}

#endif //ANGULAR_ART_GALLERY_PROBLEM_SEGMENT_INSIDE_POLYGON_H
