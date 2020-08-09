//
// Class to create random polygons.
//
// Created by Yannic Lieder on 04.08.20.
//

#ifndef ANGULAR_ART_GALLERY_PROBLEM_RANDOM_POLYGON_GENERATOR_H
#define ANGULAR_ART_GALLERY_PROBLEM_RANDOM_POLYGON_GENERATOR_H

#include <random>

#include <CGAL/Kernel/global_functions_2.h>
#include <CGAL/point_generators_2.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/random_polygon_2.h>


template <typename Kernel>
class RandomPolygonGenerator {
private:
    static constexpr double DEFAULT_MIN_VALUE = 0.0;
    static constexpr double DEFAULT_MAX_VALUE = 1000.0;
public:
    explicit RandomPolygonGenerator() : gen(std::random_device{}()), dis(DEFAULT_MIN_VALUE, DEFAULT_MAX_VALUE) { }
    explicit RandomPolygonGenerator(int seed) : gen(seed), dis(DEFAULT_MIN_VALUE, DEFAULT_MAX_VALUE) {}

    CGAL::Polygon_2<Kernel> generate(size_t size) {
        while (true) { // In rare cases, the algorithm produces a non-simple polygon, if three consecutive points are almost
                       // collinear. In this case, repeat process until the resulting polyogn is simple.
            std::vector<CGAL::Point_2<Kernel>> points;

            // Ensure that no three points are collinear according to a bug in CGALs polygon generation algorithm.
            // See https://github.com/CGAL/cgal/issues/1445 for further information
            while (points.size() != size) {
                CGAL::Point_2<Kernel> new_point(dis(gen), dis(gen));
                bool good_point = true;
                for (int i = 0; i < points.size() && good_point; ++i) {
                    if (points[i] == new_point) {
                        good_point = false;
                    }

                    for (int j = i + 1; j < points.size() && good_point; ++j) {
                        if (CGAL::collinear(points[i], points[j], new_point)) {
                            good_point = false;
                        }
                    }
                }

                if (good_point) {
                    points.emplace_back(new_point);
                }
            }

            CGAL::Polygon_2<Kernel> polygon;
            CGAL::random_polygon_2(size, std::back_inserter(polygon), points.begin());

            if (polygon.is_simple()) {
                if (polygon.is_clockwise_oriented()) {
                    polygon.reverse_orientation();
                }
                return polygon;
            }
        }
    }

    unsigned int getSeed() {
        return rand.get_seed();
    }

private:
    CGAL::Random rand;
    std::mt19937 gen; //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<double> dis;
};

#endif //ANGULAR_ART_GALLERY_PROBLEM_RANDOM_POLYGON_GENERATOR_H
