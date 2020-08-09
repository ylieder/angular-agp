//
// Created by Yannic Lieder on 04.08.20.
//

#ifndef ANGULAR_ART_GALLERY_PROBLEM_RADIUS_PATTERN_H
#define ANGULAR_ART_GALLERY_PROBLEM_RADIUS_PATTERN_H

#include "base_pattern.h"
#include "cgal_helpers/polygon_normalization.h"
#include "upper_bound/patterns/helpers/cosine_30.h"
#include "upper_bound/patterns/helpers/segment_inside_polygon.h"
#include "upper_bound/visualizer.h"


class RadiusPattern: public BasePattern {
public:
    RadiusPattern(int value) : BasePattern(value) {}

    bool split(Polygon const &polygon, std::stack<Polygon const> &remaining_polygons,
               Visualizer &visualizer) override {
        if (polygon.size() < 6) {
            return false;
        }

        auto start = polygon.vertices_circulator();
        auto current = start;

        do {
#ifdef DEBUG_LOG
            std::cout << "Current vertex: " << *current << std::endl;
#endif
            auto prev = current - 1;
            auto next = current + 1;

            auto floodlight_candidate = next + 2;
            do {
#ifdef DEBUG_LOG
                std::cout << "Current floodlight candidate: " << *floodlight_candidate << std::endl;
#endif
                if (
                    Angle(*prev, *floodlight_candidate, *next).cosine() >= cosine_30(1) &&
                    segment_inside_polygon(polygon, Segment(*floodlight_candidate, *prev)) &&
                    segment_inside_polygon(polygon, Segment(*floodlight_candidate, *next)) &&
                    segment_inside_polygon(polygon, Segment(*floodlight_candidate, *current))
                ) {
                    remaining_polygons.pop();
                    auto result = split_polygon(polygon, floodlight_candidate, next);

                    for (Polygon & subpolygon : result.left) {
                        subpolygon = normalize_polygon(subpolygon);
                        remaining_polygons.push(subpolygon);
                    }

                    assert(result.right.size() == 1);
                    auto result_2 = split_polygon(result.right[0], *floodlight_candidate, *prev);
                    assert(result_2.left.size() == 1 && (result_2.left[0].size() == 4 || result_2.left[0].size() == 3));
                    for (Polygon & subpolygon : result_2.right) {
                        subpolygon = normalize_polygon(subpolygon);
                        remaining_polygons.push(subpolygon);
                    }

                    Segment split_seg_1(*floodlight_candidate, *prev);
                    Segment split_seg_2(*floodlight_candidate, *next);
                    visualizer.split_step(
                            polygon,
                            &(result_2.left[0]),
                            &split_seg_1,
                            &split_seg_2,
                            this
                    );

                    return true;
                    //visualizer.add_circle(CGAL::Circle(*))
                }
            } while (++floodlight_candidate != prev - 1);
        } while (++current != start);
        return false;
    }

    std::string description() const override {
        return "Radius pattern";
    }
};

#endif //ANGULAR_ART_GALLERY_PROBLEM_RADIUS_PATTERN_H
