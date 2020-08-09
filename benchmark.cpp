//
// Solves a set of instances and print statistics.
//
// Created by Yannic Lieder on 06.08.20.
//

#include <filesystem>
#include <iostream>
#include <vector>

#include <boost/program_options.hpp>

#include "get_time_str.h"
#include "upper_bound/upper_bound_solver.h"
#include "serialization.h"


namespace fs = std::filesystem;
namespace po = boost::program_options;

std::string hline() {
    return std::string(80, '*');
}

void run_benchmark(std::string const & input_dir, std::string const & instance_set, std::string const & output_dir,
        bool visualize, int max_size) {
    std::string directory = input_dir + "/" + instance_set;

    std::cout << hline() << std::endl;
    std::cout << "BENCHMARK " << instance_set << std::endl;
    std::cout << "Input directory: " << input_dir << std::endl;
    std::cout << "Output directory: " << output_dir << std::endl;
    std::cout << "Start time: " << get_time_str("%Y-%m-%d %H:%M:%S") << std::endl;
    std::cout << hline() << std::endl;

    std::vector<Pattern> patterns = {
            Pattern::SMALL_TRIANGLE,
            Pattern::RADIUS,
            Pattern::DUCT,
            Pattern::HISTOGRAM,
            Pattern::NON_CONVEX_VERTEX,
            Pattern::CONVEX_SUBPOLYGON,
            Pattern::EDGE_EXTENSION,
    };

    int n_solved = 0;
    int n_unsolved = 0;
    std::vector<std::string> unsolved;

    if (!fs::is_directory(directory)) {
        std::cerr << "Not a directory: " << directory << std::endl;
        return;
    }

    int i = 0;
    using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
    for (const auto & file : recursive_directory_iterator(directory)) {
        if (!fs::is_regular_file(file) || file.path().extension() != ".pol") {
            // Skip directories
            continue;
        }

        auto polygon = serialization::read_file<Kernel>(file);

        if (max_size > 0 && polygon.size() > max_size) {
            continue;
        }

        fs::path rel_path = fs::relative(file.path(), fs::path(input_dir));
        std::string rel_dir = fs::path(rel_path).remove_filename();
        std::string filename = fs::path(rel_path).stem();

        std::cout << i++ << ". " << rel_path << std::flush;

        UpperBoundSolver solver = UpperBoundSolver(polygon, patterns);
        solver.set_visualize(visualize);
        solver.set_output(output_dir, filename, rel_dir);

        std::pair<bool, Polygon> result =  solver.solve();

        if (std::get<0>(result)) {
            ++n_solved;
            std::cout << " -> solved" << std::endl;
        } else {
            ++n_unsolved;
            std::cout << " -> unsolved" << std::endl;
            unsolved.push_back(rel_path);
        }
    }

    std::cout << hline() << std::endl;
    std::cout << "FINISH BENCHMARK " << instance_set << std::endl;
    std::cout << "End time: " << get_time_str("%Y-%m-%d %H:%M:%S") << std::endl;
    std::cout << "Solved: " << n_solved << std::endl;
    std::cout << "Unsolved: " << n_unsolved << std::endl;
    std::cout << "Total: " << (n_solved + n_unsolved) << std::endl;
    if (n_unsolved > 0) {
        std::cout << "Unsolved instances:" << std::endl;
        for (auto const &instance: unsolved) {
            std::cout << "\t" << instance << std::endl;
        }
    }
    std::cout << hline() << std::endl;
}

struct Options {
    bool visualize = false;
    std::string output_dir = "out";
    std::string base_dir;
    std::vector<std::string> instance_sets;
    int max_size = 0;
};

void parse_args(int argc, char* argv[], Options &ops) {
    po::options_description desc;
    desc.add_options()
            ("visualize,v", po::bool_switch(&ops.visualize), "Save svg image for every partitioning step")
            ("output,o", po::value<std::string>(&ops.output_dir), "Specify output directory")
            ("base_dir,b", po::value<std::string>(&ops.base_dir), "Instance base directory")
            ("instance_set,i", po::value<std::vector<std::string>>(&ops.instance_sets)->required(),
                    "The instance directory, relative to the base directory")
            ("max_size,m", po::value<int>(&ops.max_size),
                    "Consider only instances with a size less than or equal to max size")
            ;

    po::positional_options_description pdesc;
    pdesc.add("instance_set", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
            options(desc).positional(pdesc).run(), vm);
    po::notify(vm);
}

int main(int argc, char* argv[]) {
    Options options;
    parse_args(argc, argv, options);

    options.output_dir += "/benchmark_" + get_time_str();
    for (auto const & set: options.instance_sets) {
        run_benchmark(options.base_dir, set, options.output_dir, options.visualize, options.max_size);
    }
}
