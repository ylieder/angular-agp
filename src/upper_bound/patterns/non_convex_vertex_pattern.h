//
// Created by Yannic Lieder on 05.08.20.
//

#ifndef ANGULAR_ART_GALLERY_PROBLEM_NON_CONVEX_VERTEX_PATTERN_H
#define ANGULAR_ART_GALLERY_PROBLEM_NON_CONVEX_VERTEX_PATTERN_H

#include "base_pattern.h"
#include "cgal_helpers/angle.h"
#include "cgal_helpers/polygon_normalization.h"
#include "helpers/convex_subpolygon_helpers.h"
#include "upper_bound/patterns/helpers/cosine_30.h"
#include "upper_bound/patterns/helpers/segment_inside_polygon.h"
#include "upper_bound/patterns/helpers/split_polygon.h"
#include "upper_bound/visualizer.h"

class NonConvexVertexPattern : public BasePattern {
public:
    NonConvexVertexPattern(int value) : BasePattern(value) { }

    bool split(Polygon const &polygon, std::stack<Polygon const> &remaining_polygons,
               Visualizer &visualizer) override {
        auto start = polygon.vertices_circulator();
        auto current = start;

        do {
#ifdef DEBUG_LOG
            std::cout << "Current vertex: " << *current << std::endl;
#endif

            auto prev = current - 1;
            auto next = current + 1;

            if (!CGAL::right_turn(*prev, *current, *next)) {
                continue;
            }

            auto nearest_non_convex_prev = helpers::find_nearest_non_convex_vertices(current, true);
            auto nearest_non_convex_next = helpers::find_nearest_non_convex_vertices(current, false);

            bool only_one_non_convex = false;
            if (nearest_non_convex_prev == prev && nearest_non_convex_next == next) {
                continue;
            } else if (nearest_non_convex_prev == current) {
                assert(nearest_non_convex_next == current);
                nearest_non_convex_prev = current + 2;
                only_one_non_convex = true;
            }

            auto current_prev = nearest_non_convex_prev;
            do {
                Polygon::Vertex_const_circulator current_next;
                if (only_one_non_convex){
                    current_next = current_prev - 1;
                } else {
                    current_next = nearest_non_convex_next;
                }

                do {
                    if (current_prev == current_next || (current_prev == prev && current_next == next)) {
                        continue;
                    }

                    if (
                            CGAL::left_turn(*current_next, *current_prev, *(current_prev + 1)) &&
                            CGAL::left_turn(*(current_next - 1), *current_next, *current_prev) &&
                            segment_inside_polygon(polygon, Segment(*current_prev, *current_next))
                    ) {

                        Polygon split_candidate;
                        if (current_prev == current_next + 1) {
                            split_candidate = polygon;
                        } else {
                            split_candidate = Polygon(current_prev, current_next + 1);
                        }

                        if (helpers::one_non_convex_vertex_subpolygon_coverable(split_candidate)) {
                            remaining_polygons.pop();

                            auto result = split_polygon(polygon, current_prev, current_next);
                            assert(result.right.size() == 1);

                            for (Polygon & subpolygon : result.left) {
                                subpolygon = normalize_polygon(subpolygon);
                                remaining_polygons.push(subpolygon);
                            }

                            Segment split_seg(*current_prev, *current_next);
                            visualizer.split_step(
                                    polygon,
                                    &(result.right[0]),
                                    &split_seg,
                                    nullptr,
                                    this
                            );

                            return true;
                        }

                    }
                } while (--current_next != current_prev && current_next != current);
            } while (++current_prev != current);
        } while (++current != start);

        return false;
    }

    bool combine_visualizations() const override {
        return true;
    }

    std::string description() const override {
        return "One non-convex vertex subpolygon pattern";
    }
private:
};

#endif //ANGULAR_ART_GALLERY_PROBLEM_NON_CONVEX_VERTEX_PATTERN_H
