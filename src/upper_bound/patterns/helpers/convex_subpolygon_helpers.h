//
// Helper functions for convex and non-convex subpolygons.
//
// Created by Yannic Lieder on 06.08.20.
//

#ifndef ANGULAR_ART_GALLERY_PROBLEM_CONVEX_SUBPOLYGON_HELPERS_H
#define ANGULAR_ART_GALLERY_PROBLEM_CONVEX_SUBPOLYGON_HELPERS_H

#include "cosine_30.h"
#include "kernel_definitions.h"
#include "segment_inside_polygon.h"


namespace helpers {
    Polygon::Vertex_const_circulator
    static find_nearest_non_convex_vertices(Polygon::Vertex_const_circulator const &v, bool reverse = false) {
        int direction = reverse ? -1 : 1;
        auto current = v + direction;
        while (CGAL::left_turn(*(current - 1), *current, *(current + 1))) {
            current += direction;
        }
        return current;
    }

    static bool one_non_convex_vertex_subpolygon_coverable(Polygon const & split_candidate) {
        int size = split_candidate.size();
        assert(split_candidate.size() > 3);
        auto floodlight_candidates_start = split_candidate.vertices_circulator();
        auto floodlight_candidate = floodlight_candidates_start;

#ifdef DEBUG_LOG
        std::cout << "Test if subpolygon with one non-convex subpolygon is coverable" << std::endl;
        std::cout << "Subpolygon: " << split_candidate << std::endl;
#endif

        do {
#ifdef DEBUG_LOG
            std::cout << "Floodlight candidate: " << *floodlight_candidate << std::endl;
#endif
            auto angle = Angle<Kernel>(floodlight_candidate);
            if (angle.is_convex() and angle.cosine() >= cosine_30(std::min(size - 2, 6))) {
                bool all_visible = true;
                auto visibiliy_vertex = floodlight_candidate + 2;
                do {
                    if (
                            !segment_inside_polygon(
                                    split_candidate,
                                    Segment(*floodlight_candidate, *visibiliy_vertex)
                            )
                            ) {
                        all_visible = false;
                    }
                } while (all_visible && ++visibiliy_vertex != floodlight_candidate - 1);

                if (all_visible) {
                    return true;
                }
            }
        } while (++floodlight_candidate != floodlight_candidates_start);

        return false;
    }
}

#endif //ANGULAR_ART_GALLERY_PROBLEM_CONVEX_SUBPOLYGON_HELPERS_H
