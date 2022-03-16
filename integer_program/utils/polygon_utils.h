#ifndef ANGULARARTGALLERYPROBLEM_POLYGON_UTILS_H
#define ANGULARARTGALLERYPROBLEM_POLYGON_UTILS_H

#include <CGAL/Polygon_2.h>

#include "utils/conversion_utils.h"

namespace utils
{
    namespace polygon
    {
        template<typename Kernel>
        CGAL::Polygon_2<Kernel> create_regular(int size, double radius = 1)
        {
            CGAL::Polygon_2<Kernel> polygon;
            double angle = 2 * M_PI / size;

            for (int i = 0; i < size; ++i)
            {
                std::pair<double, double> coords = utils::conversion::polar_to_cartesian((double)i * angle, radius);
                polygon.push_back(CGAL::Point_2<Kernel>(coords.first, coords.second));
            }

            return polygon;
        }
    }
}

#endif //ANGULARARTGALLERYPROBLEM_POLYGON_UTILS_H
