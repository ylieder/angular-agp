//
// Command line tool to generate sets of random polygon of specific sizes.
//
// Created by Yannic Lieder on 06.08.20.
//

#include <string>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>

#include "cgal_helpers/random_polygon_generator.h"
#include "get_time_str.h"
#include "kernel_definitions.h"
#include "serialization.h"

namespace po = boost::program_options;

void parse_args(int argc, char* argv[],
        std::vector<std::pair<int,int>> & creation_parameters, std::string & output_dir) {
    std::vector<std::string> str_parameter_sequences;

    po::options_description desc;
    desc.add_options()
            ("instances,r", po::value<std::vector<std::string>>(&str_parameter_sequences)->required(),
                    "Specify an instance set as a squence of integers. First one specifies number of instances per "
                    "size and all following specify the sizes")
            ("output_dir,o", po::value<std::string>(&output_dir),
                    "Output directory")
    ;

    po::positional_options_description pdesc;
    pdesc.add("instance_set", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
            options(desc).positional(pdesc).run(), vm);
    po::notify(vm);

    typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
    for (std::string &sequence: str_parameter_sequences) {
        boost::char_separator<char> sep(":,");
        tokenizer tokens(sequence, sep);
        boost::tokenizer tok(sequence);
        auto token = tok.begin();
        int size = stoi(*token);
        while (++token != tok.end()) {
            creation_parameters.emplace_back(size, stoi(*token));
        }
    }
}

void progress(int step, int max) {
    int diff = (int)(50 * (double)step/max) - (int)(50 * (double)(step - 1)/max);
    if (step > 0 && diff > 0) {
        std::cout << std::string(diff, '.') << std::flush;
    }
}

int main(int argc, char* argv[]) {
    std::string output_dir = "resources/instances/random_" + get_time_str(); // default output directory
    std::vector<std::pair<int,int>> creation_parameters;
    parse_args(argc, argv, creation_parameters, output_dir);

    RandomPolygonGenerator<Kernel> generator;
    std::cout << "Output directory: " << output_dir << std::endl;
    std::cout << "Seed: " << generator.getSeed() << std::endl;

    for (auto & parameter: creation_parameters) {
        int n = std::get<0>(parameter);
        int size = std::get<1>(parameter);

        std::cout << "Create " << n << " polygons of size " << size << std::endl;

        for (int i = 0; i < n; ++i) {
            Polygon polygon = generator.generate(size);
            std::stringstream ss;
            ss << "random_" << size << "_" << i;
            serialization::write_file(polygon, output_dir + "/" + std::to_string(size), ss.str());
            progress(i + 1, n);
        }
        std::cout << std::endl;
    }
}