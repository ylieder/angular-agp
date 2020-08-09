//
// Command line tool to solve an AAGP instance.
//
// Created by Yannic Lieder on 06.08.20.
//

#include <filesystem>
#include <iostream>

#include <boost/program_options.hpp>
#include <get_time_str.h>

#include "cgal_helpers/random_polygon_generator.h"
#include "serialization.h"
#include "upper_bound/upper_bound_solver.h"


namespace po = boost::program_options;
namespace fs = std::filesystem;

bool parse_args(int argc, char* argv[], std::string & input_file, std::string & output_dir, int & random) {
    po::options_description desc;
    desc.add_options()
            ("output,o", po::value<std::string>(&output_dir), "Specify output directory")
            ("file,f", po::value<std::string>(&input_file), "Path to the instance file")
            ("random,r", po::value<int>(&random), "Create random polygon of specified size")
    ;

    po::positional_options_description pdesc;
    pdesc.add("file", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
            options(desc).positional(pdesc).run(), vm);
    po::notify(vm);

    if (input_file.empty() and random <= 0) {
        std::cerr << "Specify either an instance file (-f <path>) or create a random polygon (-r <size>)" << std::endl;
        return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    std::string input_file;
    std::string output_dir = "out/aagp_" + get_time_str();
    int random_size = 0;

    if (!parse_args(argc, argv, input_file, output_dir, random_size)) {
        return 1;
    }

    Polygon polygon;
    std::string polygon_name;
    if (!input_file.empty()) {
        std::cout << "Read polygon " << input_file << std::endl;
        fs::path path(input_file);

        if (!fs::exists(path) || !fs::is_regular_file(path)) {
            std::cerr << "Not a file: " << path << std::endl;
        }

        polygon = serialization::read_file<Kernel>(input_file);
        polygon_name = path.stem();
    } else {
        RandomPolygonGenerator<Kernel> generator;
        auto seed = generator.getSeed();

        std::cout << "Create random polygon of size " << random_size << " (Seed: " << seed << ")" << std::endl;

        polygon = generator.generate(random_size);
        polygon_name = "random_" + std::to_string(random_size);
    }
    std::cout << "Output directory: " << (output_dir + "/" + polygon_name) << std::endl;
    std::cout << "Polygon: " << polygon << std::endl;

    std::vector<Pattern> patterns = {
            Pattern::SMALL_TRIANGLE,
            Pattern::RADIUS,
            Pattern::DUCT,
            Pattern::HISTOGRAM,
            Pattern::NON_CONVEX_VERTEX,
            Pattern::CONVEX_SUBPOLYGON,
            Pattern::EDGE_EXTENSION
    };

    UpperBoundSolver solver = UpperBoundSolver(polygon, patterns);
    solver.set_output(output_dir, polygon_name);
    solver.set_visualize(true);

    std::pair<bool, Polygon> result =  solver.solve();
    if (std::get<0>(result)) {
        std::cout << "Solved! " << std::endl;
    } else {
        std::cout << "Unsolved! " << std::endl;
    }

    return 0;
}
