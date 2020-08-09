//
// Created by Yannic Lieder on 05.08.20.
//

#ifndef ANGULAR_ART_GALLERY_PROBLEM_EDGE_EXTENSION_PATTERN_H
#define ANGULAR_ART_GALLERY_PROBLEM_EDGE_EXTENSION_PATTERN_H

#include "base_pattern.h"
#include "cgal_helpers/angle.h"
#include "helpers/edge_extension_helpers.h"
#include "upper_bound/patterns/helpers/cosine_30.h"
#include "upper_bound/visualizer.h"


class EdgeExtensionPattern : public BasePattern {
public:
    EdgeExtensionPattern(int value) : BasePattern(value) { }

    bool split(Polygon const &polygon, std::stack<Polygon const> &remaining_polygons,
               Visualizer &visualizer) override {
        if (check_forward_direction(polygon, remaining_polygons, visualizer)) {
            return true;
        }

        return check_backward_direction(polygon, remaining_polygons, visualizer);
    }

    std::string description() const override {
        return "Edge extension pattern";
    }
private:
    bool check_forward_direction(Polygon const &polygon, std::stack<Polygon const> &remaining_polygons,
            Visualizer &visualizer) {
        return check_one_direction(polygon, remaining_polygons, visualizer, false);
    }

    bool check_backward_direction(Polygon const &polygon, std::stack<Polygon const> &remaining_polygons,
            Visualizer &visualizer) {
        return check_one_direction(polygon, remaining_polygons, visualizer, true);
    }

    bool check_one_direction(Polygon const &polygon, std::stack<Polygon const> &remaining_polygons,
            Visualizer &visualizer, bool reverse = false) {
        auto start = polygon.vertices_circulator();
        auto current = start;

        int direction = reverse ? -1 : 1;

        do {
#ifdef DEBUG_LOG
            std::cout << "Current vertex: " << *current << std::endl;
#endif

            if (!CGAL::right_turn(*(current - 1), *current, *(current + 1))) {
                continue;
            }

            auto result = helpers::ray_polygon_intersection(polygon, Ray(*current, *current - *(current - direction)));

            if (!std::get<0>(result)) {
                continue;
            }

            Polygon::Vertex_const_circulator subpolygon_end;
            Polygon subpolygon;
            if (reverse) {
                subpolygon_end = helpers::find_vertex_circulator(polygon, std::get<1>(result)->target());
                subpolygon = Polygon(subpolygon_end, current + 1);
            } else {
                subpolygon_end = helpers::find_vertex_circulator(polygon, std::get<1>(result)->source());
                subpolygon = Polygon(current, subpolygon_end + 1);
            }
            subpolygon.push_back(std::get<2>(result));

            if (
                    subpolygon.is_convex() &&
                    smallest_inner_angle_cos(subpolygon) >= cosine_30(std::min((int)subpolygon.size() - 2, 6))
                    ) {
                remaining_polygons.pop();

                Polygon remaining_polygon;
                if (reverse) {
                    remaining_polygon = Polygon(current + 1, subpolygon_end);
                } else {
                    remaining_polygon = Polygon(subpolygon_end + 1, current);
                }
                remaining_polygon.push_back(std::get<2>(result));
                remaining_polygon = normalize_polygon(remaining_polygon);
                remaining_polygons.push(remaining_polygon);

                remaining_polygon.push_back(std::get<2>(result));

                Segment split_seg(*current, std::get<2>(result));
                visualizer.split_step(
                        polygon,
                        &subpolygon,
                        &split_seg,
                        nullptr,
                        this
                );

                return true;
            }
        } while (++current != start);
        return false;
    }
};

#endif //ANGULAR_ART_GALLERY_PROBLEM_EDGE_EXTENSION_PATTERN_H
