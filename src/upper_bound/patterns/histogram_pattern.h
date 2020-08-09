//
// Created by Yannic Lieder on 04.08.20.
//

#ifndef ANGULAR_ART_GALLERY_PROBLEM_HISTOGRAM_PATTERN_H
#define ANGULAR_ART_GALLERY_PROBLEM_HISTOGRAM_PATTERN_H

#include "base_pattern.h"
#include "cgal_helpers/angle.h"
#include "helpers/edge_extension_helpers.h"
#include "upper_bound/visualizer.h"


class HistogramPattern : public BasePattern {
public:
    HistogramPattern(int value) : BasePattern(value) { }

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

            auto polygon_intersection_prev = helpers::ray_polygon_intersection(polygon, Ray(*current, *current - *prev));
            auto polygon_intersection_next = helpers::ray_polygon_intersection(polygon, Ray(*current, *current - *next));

            if (!std::get<0>(polygon_intersection_prev) || !std::get<0>(polygon_intersection_next)) {
                // TODO: Remove this branch, if intersection function ist complete
                continue;
            }

            if (std::get<1>(polygon_intersection_prev) == std::get<1>(polygon_intersection_next)) {
                remaining_polygons.pop();

                auto intersecting_edge = std::get<1>(polygon_intersection_prev);
                auto intersecting_edge_target = helpers::find_vertex_circulator(polygon, intersecting_edge->target());

                Polygon subpolygon_1(intersecting_edge_target, current);
                subpolygon_1.push_back(std::get<2>(polygon_intersection_prev));
                subpolygon_1 = normalize_polygon(subpolygon_1);

                Polygon subpolygon_2(next, intersecting_edge_target);
                subpolygon_2.push_back(std::get<2>(polygon_intersection_next));
                subpolygon_2 = normalize_polygon(subpolygon_2);

                remaining_polygons.push(subpolygon_1);
                remaining_polygons.push(subpolygon_2);

                Segment split_seg_1(*current, std::get<2>(polygon_intersection_prev));
                Segment split_seg_2(*current, std::get<2>(polygon_intersection_next));
                visualizer.split_step(
                        polygon,
                        nullptr,
                        &split_seg_1,
                        &split_seg_2,
                        this
                );

                return true;
            }
        } while (++current != start);
        return false;
    }

    std::string description() const override {
        return "Histogram pattern";
    }
};

#endif //ANGULAR_ART_GALLERY_PROBLEM_HISTOGRAM_PATTERN_H
