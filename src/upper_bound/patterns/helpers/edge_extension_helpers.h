//
// Helper functions forpolygon edge extensions.
//
// Created by Yannic Lieder on 06.08.20.
//

#ifndef ANGULAR_ART_GALLERY_PROBLEM_EDGE_EXTENSION_HELPERS_H
#define ANGULAR_ART_GALLERY_PROBLEM_EDGE_EXTENSION_HELPERS_H

#include "kernel_definitions.h"


namespace helpers {
    static std::tuple<bool, Polygon::Edge_const_circulator, Point>
    ray_polygon_intersection(Polygon const &polygon, Ray const &ray) {
        auto start = polygon.edges_circulator();
        auto current = start;

        Polygon::Edge_const_circulator nearest_edge;
        Kernel::FT smallest_squared_distance(0);
        Point intersection_point;
        bool found_intersection = false;

        do {
            auto result = CGAL::intersection(*current, ray);

            if (!result) {
                // no intersection with current edge
                continue;
            }
            if (const CGAL::Point_2<Kernel> *p = boost::get<CGAL::Point_2<Kernel>>(&*result)) {
                // Intersect in one point
                if (*p == current->source() || *p == current->target()) {
                    // Intersection is an endpoint of the segment
                    // TODO
                    continue;
                }

                auto squared_distance = (ray.source() - *p).squared_length();

                if (squared_distance > 0 && (!found_intersection || smallest_squared_distance > squared_distance)) {
                    nearest_edge = current;
                    smallest_squared_distance = squared_distance;
                    intersection_point = *p;
                    found_intersection = true;
                }
            } else {
                // TODO
                continue;
            }
        } while (++current != start);
        return std::make_tuple(found_intersection, nearest_edge, intersection_point);
    }

    static Polygon::Vertex_circulator find_vertex_circulator(Polygon const & polygon, Point const & v) {
        auto start = polygon.vertices_circulator();
        auto current = start;

        do {
            if (*current == v) {
                return current;
            }
        } while (++current != start);
        assert(false);
        return Polygon::Vertex_circulator();
    }
}

#endif //ANGULAR_ART_GALLERY_PROBLEM_EDGE_EXTENSION_HELPERS_H
