//
// Created by Yannic Lieder on 04.08.20.
//

#ifndef ANGULAR_ART_GALLERY_PROBLEM_DUCT_PATTERN_H
#define ANGULAR_ART_GALLERY_PROBLEM_DUCT_PATTERN_H

#include "base_pattern.h"
#include "cgal_helpers/angle.h"
#include "cgal_helpers/polygon_normalization.h"
#include "upper_bound/patterns/helpers/cosine_30.h"
#include "upper_bound/patterns/helpers/segment_inside_polygon.h"
#include "upper_bound/patterns/helpers/split_polygon.h"
#include "upper_bound/visualizer.h"


class DuctPattern: public BasePattern {
public:
    DuctPattern(int value) : BasePattern(value) { }

    /**
     * \pre polygon.size() >= 6
     */
    bool split(Polygon const & polygon, std::stack<Polygon const> & remaining_polygons,
               Visualizer & visualizer) override {
        auto start = polygon.edges_circulator();
        auto e1 = start;
        do {
#ifdef DEBUG_LOG
            std::cout << "Current edge e1: " << *e1 << std::endl;
#endif
            auto e2 = e1 + 3;
            do {
#ifdef DEBUG_LOG
                std::cout << "Current edge e2: " << *e2 << std::endl;
#endif
                if (
                        CGAL::collinear(e1->source(), e1->target(), e2->source()) ||
                        CGAL::collinear(e1->source(), e1->target(), e2->target())) {
                    continue;
                }

                auto split_seg_1 = Segment(e1->target(), e2->source());
                auto split_seg_2 = Segment(e2->target(), e1->source());

                if (
                        segment_inside_polygon(polygon, split_seg_1) &&
                        segment_inside_polygon(polygon, split_seg_2)
                        ) {
                    Polygon quadrilateral;
                    quadrilateral.push_back(e1->source());
                    quadrilateral.push_back(e1->target());
                    quadrilateral.push_back(e2->source());
                    quadrilateral.push_back(e2->target());
                    if (quadrilateral.is_convex() && quadrilateral_30_deg_coverable(quadrilateral)) {
                          remaining_polygons.pop();

                        auto result = split_polygon(polygon, e1->target(), e2->source());

                        for (Polygon &subpolygon : result.right) {
                            subpolygon = normalize_polygon(subpolygon);
                            remaining_polygons.push(subpolygon);
                        }

                        assert(result.left.size() == 1);
                        auto result_2 = split_polygon(result.left[0], e2->target(), e1->source());
                        assert(result_2.left.size() == 1 && result_2.left[0].size() == 4);
                        for (Polygon &subpolygon : result_2.right) {
                            subpolygon = normalize_polygon(subpolygon);
                            remaining_polygons.push(subpolygon);
                        }

                        visualizer.split_step(
                                polygon,
                                &(result_2.left[0]),
                                &split_seg_1,
                                &split_seg_2,
                                this
                        );
                        return true;
                    }
                }
            } while (++e2 != e1 - 2);
        } while (++e1 != start);

        return false;
    }

    std::string description() const override {
        return "Duct pattern";
    }

private:
    static bool quadrilateral_30_deg_coverable(Polygon const & quadrilateral) {
        auto smallest_angle_cos = smallest_inner_angle_cos(quadrilateral);
        if (smallest_angle_cos >= cosine_30(1)) {
            return true;
        }

        auto *p1 = &quadrilateral[0];
        auto *p2 = &quadrilateral[1];
        auto *p3 = &quadrilateral[2];
        auto *p4 = &quadrilateral[3];

        auto diagonal_angle_1 = Angle(*p1, *p2, *p4);
        auto diagonal_angle_2 = Angle(*p2, *p3, *p1);
        auto diagonal_angle_3 = Angle(*p3, *p4, *p2);
        auto diagonal_angle_4 = Angle(*p4, *p1, *p3);
        auto diagonal_angle_5 = Angle(*p4, *p2, *p3);
        auto diagonal_angle_6 = Angle(*p1, *p3, *p4);
        auto diagonal_angle_7 = Angle(*p2, *p4, *p1);
        auto diagonal_angle_8 = Angle(*p3, *p1, *p2);

        auto check_angles = [](auto const & a, auto const & b)
        {
            return (a.interval() + b.interval()).upper() < (ia::pi() / ia::DoubleInterval(6)).lower();
        };

        return
            check_angles(diagonal_angle_1, diagonal_angle_3) ||
            check_angles(diagonal_angle_2, diagonal_angle_4) ||
            check_angles(diagonal_angle_5, diagonal_angle_7) ||
            check_angles(diagonal_angle_6, diagonal_angle_8);
    }
};

#endif //ANGULAR_ART_GALLERY_PROBLEM_DUCT_PATTERN_H
