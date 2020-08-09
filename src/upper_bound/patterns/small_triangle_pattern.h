//
// Created by Yannic Lieder on 02.08.20.
//

#ifndef ANGULAR_ART_GALLERY_PROBLEM_SMALL_TRIANGLE_PATTERN_H
#define ANGULAR_ART_GALLERY_PROBLEM_SMALL_TRIANGLE_PATTERN_H

#include "base_pattern.h"
#include "cgal_helpers/angle.h"
#include "cgal_helpers/polygon_normalization.h"
#include "upper_bound/patterns/helpers/cosine_30.h"
#include "upper_bound/patterns/helpers/split_polygon.h"
#include "upper_bound/visualizer.h"


class SmallTrianglePattern: public BasePattern {
public:
    SmallTrianglePattern(int value) : BasePattern(value) {}

    bool split(Polygon const & polygon, std::stack<Polygon const> & remaining_polygons,
            Visualizer & visualizer) override {
        auto start = polygon.vertices_circulator();
        auto current = start;

        do {
#ifdef DEBUG_LOG
            std::cout << "Current vertex: " << *current << std::endl;
#endif

            if (!Angle<Kernel>(current).is_convex()) {
                continue;
            }

            auto prev = current - 1;
            auto next = current + 1;


            auto triangle = Triangle(*prev, *current, *next);

            bool triangle_empty = true;
            for (auto v = next + 1; v != prev; ++v) {
                if (triangle.has_on_bounded_side(*v)) {
                    triangle_empty = false;
                    break;
                }
            }

            if (!triangle_empty) {
                continue;
            }

            if (
                    Angle(*prev, *current, *next).cosine() >= cosine_30(1) ||
                    Angle(*current, *next, *prev).cosine() >= cosine_30(1) ||
                    Angle(*next, *prev, *current).cosine() >= cosine_30(1)
            ) {
                remaining_polygons.pop();
                auto result = split_polygon(polygon, prev, next);
                assert(result.right.size() == 1 && result.right[0].size() == 3);

                for (Polygon & subpolygon : result.left) {
                    subpolygon = normalize_polygon(subpolygon);
                    remaining_polygons.push(subpolygon);
                }

                Segment split_seg(*prev, *next);
                visualizer.split_step(
                        polygon,
                        &(result.right[0]),
                        &split_seg,
                        nullptr,
                        this
                );

                return true;
            }
        } while (++current != start);

        return false;
    }

    bool combine_visualizations() const override {
        return true;
    }

    std::string description() const override  {
        return "Small triangle pattern";
    }

private:
    static FT smallest_triangle_angle_cos(Triangle const & triangle) {
        return FT(0);
    }
};

#endif //ANGULAR_ART_GALLERY_PROBLEM_SMALL_TRIANGLE_PATTERN_H
