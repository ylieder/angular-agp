//
// Created by Yannic Lieder on 09.10.19.
//
#include <random>

#include <CGAL/random_polygon_2.h>
#include <CGAL/point_generators_2.h>

#include <boost/filesystem.hpp>

#include "serialization.h"

using Epeck          = CGAL::Exact_predicates_exact_constructions_kernel;
using PointGenerator = CGAL::Random_points_in_square_2<CGAL::Point_2<Epeck>>;

namespace fs = boost::filesystem;

int main(int argc, const char* argv[])
{
    fs::path instance_path = "resources/instances";
    fs::path instance_directory = "CGALRAND";

    int seed = std::random_device{}();
    int instances_per_size = 50;

    CGAL::Random rand(seed);

    int sizes[] = {5,6,7,8,9,10,20,50,100};

    for (int size : sizes)
    {
        for (int i = 0; i < instances_per_size; ++i)
        {
            CGAL::Polygon_2<Epeck> polygon;
            CGAL::random_polygon_2(size, std::back_inserter(polygon), PointGenerator(50, rand));

            fs::path file = instance_path / instance_directory / ("rand-" + std::to_string(size) + "-" + std::to_string(i) + ".pol");
            AAGP::serialization::write_file(polygon, file);
        }
    }

    return 0;
}