//
// Created by Yannic Lieder on 03.10.19.
//

#ifndef ANGULARARTGALLERYPROBLEM_CONVERSION_UTILS_H
#define ANGULARARTGALLERYPROBLEM_CONVERSION_UTILS_H

#include "common_utils.h"

namespace utils
{
    namespace conversion
    {
        static double to_degree(double radians)
        {
            return radians * 180 / M_PI;
        }

        static double to_radians(double degree)
        {
            return degree * M_PI / 180;
        }

        static std::pair<double, double> polar_to_cartesian(double angle, double r = 1)
        {
            double x = r * cos(angle);
            double y = r * sin(angle);

            return std::make_pair(x, y);
        }

        static utils::common::Triple<int, int, int> hsv_to_rgb(double h, double s, double v)
        {
            int H = (int) (h * 6);
            double f = h * 6 - H;

            int p = (int)round(v * (1 - s) * 256);
            int q = (int)round(v * (1 - f * s) * 256);
            int t = (int)round(v * (1 - (1 - f) * s) * 256);
            int v_int = (int)round(v * 256);

            switch (H)
            {
                case 0:
                    return utils::common::make_triple<int, int, int>(v_int, t, p);
                case 1:
                    return utils::common::make_triple<int, int, int>(q, v_int, p);
                case 2:
                    return utils::common::make_triple<int, int, int>(p, v_int, t);
                case 3:
                    return utils::common::make_triple<int, int, int>(p, q, v_int);
                case 4:
                    return utils::common::make_triple<int, int, int>(t, p, v_int);
                case 5:
                    return utils::common::make_triple<int, int, int>(v_int, p, q);
                default:
                    return utils::common::make_triple<int, int, int>(0, 0, 0);
            }
        }
    }
}

#endif //ANGULARARTGALLERYPROBLEM_CONVERSION_UTILS_H
