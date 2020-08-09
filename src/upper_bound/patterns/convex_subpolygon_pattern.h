//
// Created by Yannic Lieder on 05.08.20.
//

#ifndef ANGULAR_ART_GALLERY_PROBLEM_CONVEX_SUBPOLYGON_PATTERN_H
#define ANGULAR_ART_GALLERY_PROBLEM_CONVEX_SUBPOLYGON_PATTERN_H

#include "base_pattern.h"
#include "cgal_helpers/angle.h"
#include "helpers/convex_subpolygon_helpers.h"
#include "helpers/split_polygon.h"
#include "non_convex_vertex_pattern.h"
#include "upper_bound/pattern_manager.h"
#include "upper_bound/patterns/helpers/cosine_30.h"
#include "upper_bound/patterns/helpers/segment_inside_polygon.h"
#include "upper_bound/visualizer.h"


class ConvexSubpolygonPattern: public BasePattern {
public:
    ConvexSubpolygonPattern(int value) : BasePattern(value) { }

    /**
     * \pre polygon.size() > 4
     * \pre polygon is not convex
     */
    bool split(Polygon const &polygon, std::stack<Polygon const> &remaining_polygons,
               Visualizer &visualizer) override {
        auto start = polygon.vertices_circulator();
        auto current = start;

        do {
#ifdef DEBUG_LOG
            std::cout << "Current vertex: " << *current << std::endl;
#endif
            if (
                    !CGAL::left_turn(*current, *(current + 1), *(current + 2)) ||
                    !CGAL::left_turn(*(current + 1), *(current + 2), *(current + 3))
            ) {
                continue;
            }

            auto subpolygon_end = current + 3;
            do {
                bool success = false;
                BasePattern* pattern_ptr = this;
                Segment split_segment = Segment(*subpolygon_end, *current);
                if (
                        !CGAL::right_turn(*subpolygon_end, *current, *(current + 1)) &&
                        !CGAL::right_turn(*(subpolygon_end - 1), *subpolygon_end, *current) &&
                        segment_inside_polygon(polygon, split_segment)
                ) {
                    // all vertices are convex
                    Polygon convex_subpolygon(current, subpolygon_end + 1);
                    int size = convex_subpolygon.size();
                    if (smallest_inner_angle_cos(convex_subpolygon) >= cosine_30(std::min(size - 2, 6))) {
                        success = true;
                    }
                } else if (
                        (
                                !CGAL::right_turn(*subpolygon_end, *current, *(current + 1)) ||
                                !CGAL::right_turn(*(subpolygon_end - 1), *subpolygon_end, *current)
                        ) && segment_inside_polygon(polygon, split_segment)
                ) {
                    // exactly one vertex is non-conex. This one is one at the split segment.
                    Polygon non_convex_subpolygon(current, subpolygon_end + 1);
                    success = helpers::one_non_convex_vertex_subpolygon_coverable(non_convex_subpolygon);
                    pattern_ptr = PatternManager::get(Pattern::NON_CONVEX_VERTEX);
                }

                if (success) {
                    remaining_polygons.pop();

                    auto result = split_polygon(polygon, subpolygon_end, current);
                    assert(result.left.size() == 1);

                    for (auto & subpolygon : result.right) {
                        subpolygon = normalize_polygon(subpolygon);
                        remaining_polygons.push(subpolygon);
                    }

                    visualizer.split_step(
                            polygon,
                            &(result.left[0]),
                            &split_segment,
                            nullptr,
                            pattern_ptr
                    );
                    return true;
                }
            } while (
                CGAL::left_turn(*(subpolygon_end - 1), *subpolygon_end, *(subpolygon_end + 1)) &&
                ++subpolygon_end != current - 1 // Condition is unnecessary, if precondition holds
            );
        } while (++current != start);

        return false;
    }

    bool combine_visualizations() const override {
        return true;
    }

    std::string description() const override {
        return "Convex subpolygon pattern";
    }
};

#endif //ANGULAR_ART_GALLERY_PROBLEM_CONVEX_SUBPOLYGON_PATTERN_H
