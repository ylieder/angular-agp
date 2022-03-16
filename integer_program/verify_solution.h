//
// Created by Yannic Lieder on 03.10.19.
//

#ifndef ANGULARARTGALLERYPROBLEM_VERIFY_SOLUTION_H
#define ANGULARARTGALLERYPROBLEM_VERIFY_SOLUTION_H

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Polygon_set_2.h>

#include "simple_svg/simple_svg_cgal_extension.h"

#include "utils/cgal_utils.h"

using Epeck          = CGAL::Exact_predicates_exact_constructions_kernel;

namespace AAGP
{
    static std::pair<bool, double> verify_solution(const CGAL::Polygon_2<Epeck> &polygon, const std::vector<Floodlight<Epeck>> &floodlights)
    {
        std::vector<CGAL::Polygon_2<Epeck>> visibility_polygons;

        for (auto &f : floodlights)
        {
            visibility_polygons.push_back(f.visibility_polygon(polygon));
        }

        CGAL::Polygon_set_2<Epeck> union_polygon;
        for (auto &p : visibility_polygons)
        {
            union_polygon.join(p);
        }

        CGAL::Polygon_set_2<Epeck> uncovered_areas(polygon);
        uncovered_areas.difference(union_polygon);

        if (uncovered_areas.is_empty())
        {
            return std::make_pair(true, 1);
        } else {
            Epeck::FT a(0);

            std::vector<CGAL::Polygon_with_holes_2<Epeck>> pwh;
            uncovered_areas.polygons_with_holes(std::back_inserter(pwh));

            for (auto &p : pwh)
            {
                a += utils::cgal::pwh_area(p);
            }

            double b = CGAL::to_double(polygon.area());

            return std::make_pair(false, 1 - CGAL::to_double(a) / b);
        }
    }
}

#endif //ANGULARARTGALLERYPROBLEM_VERIFY_SOLUTION_H
