//
// Created by Yannic Lieder on 24.09.19.
//
#include <CGAL/random_polygon_2.h>
#include <CGAL/point_generators_2.h>

#include <boost/program_options.hpp>

#include <random>

#include "utils/conversion_utils.h"
#include "utils/timeout.h"

#include "include/simple_svg/simple_svg_1.0.0.hpp"
#include "integer_program/aagp_approximation.h"
#include "verify_solution.h"
#include "serialization.h"


// TODO:
// * HSV colors for floodlights -- check
// * export approximated floodlights -- check
// * verify solution -- check
// * svg verbose
// * filepath output
// * svg scale
// * replace CGALSVG completely
// * simple_svg margin
// * non-regular visibility output (triangulation algorithm)

using Epeck          = CGAL::Exact_predicates_exact_constructions_kernel;
using PointGenerator = CGAL::Random_points_in_square_2<CGAL::Point_2<Epeck>>;

namespace po = boost::program_options;

int main(int argc, const char* argv[])
{
    int n = 15;
    int max_angle = 10;
    uint seed = std::random_device{}();
    bool use_threading = true;
    bool exact_kernel = false;
    bool minimize_angle = true;
    bool silent = false;
    bool svg_verbose = false;
    bool varify = true;
    bool agplib = false;

    int timeout = 0;

    fs::path instance_path;
    fs::path solution_path;
    fs::path outpath;

    po::options_description option_description("Allowed options");
    option_description.add_options()
            ("help,h", "Produce help message")

            ("agplib", po::value<bool>(&agplib)->implicit_value(true), "Read AGPLIB instance")
            ("exact,e", po::value<bool>(&exact_kernel)->implicit_value(true),  "Use exact kernel for all computations (very slow)")
            ("file,f", po::value<fs::path>(&instance_path), "Read polygon from file")
            ("furthersvg", po::value<bool>(&svg_verbose)->implicit_value(true),  "Save several additional figures (arrangement, the guard candidates)")
            ("maxangle,a", po::value<int>(&max_angle), "Maximum floodlight angle")
            ("minimizeangle", po::value<bool>(&minimize_angle)->implicit_value(true),  "Minimize the total angle [default]")
            ("minimizenum", po::value<bool>(&minimize_angle)->implicit_value(false),  "Minimize the total number of floodlights")
            ("nothreading", po::value<bool>(&use_threading)->implicit_value(false),  "Disable multithreading")
            ("novarification", po::value<bool>(&varify)->implicit_value(false), "No varification of the solution")
            ("outpath,o", po::value<fs::path>(&outpath), "Output path")
            ("seed", po::value<uint>(&seed), "Seed for random")
            ("silent", po::value<bool>(&silent)->implicit_value(true),  "Suppress console progress output")
            ("size,n", po::value<int>(&n),  "Creates random polygon of size n")
            ("solution,s", po::value<fs::path>(&solution_path), "Read solution from file")
            ("timeout,t", po::value<int>(&timeout), "Set timeout in ms")
            ;

    po::variables_map options;
    try {
        po::store(po::command_line_parser(argc, argv).
                          options(option_description).
                          run(),
                  options);

        po::notify(options);
    }
    catch(const po::error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    CGAL::Polygon_2<Epeck> polygon;
    std::vector<AAGP::Floodlight<Epeck>> solution;

    if (!instance_path.empty())
    {
        if (agplib)
        {
            polygon = AAGP::serialization::read_agplib_file<Epeck>(instance_path);
        } else {
            polygon = AAGP::serialization::read_file<Epeck>(instance_path);
        };
    } else {
        CGAL::Random rand(seed);
        CGAL::random_polygon_2(n, std::back_inserter(polygon), PointGenerator(50, rand));

        if (!fs::exists("resources/random_polygons"))
            fs::create_directories("resources/random_polygons");

        instance_path = "resources/random_polygons/random_" + std::to_string(n) + "_" + std::to_string(seed) + ".instance";
        AAGP::serialization::write_file(polygon, instance_path);

        std::cout << "instance path: " << instance_path << std::endl;
        std::cout << "seed: " << seed << std::endl;
    }

    std::cout << "maximum floodlight angle: " << max_angle << "°" << std::endl;
    std::cout << "polygon size: " << polygon.size() << std::endl << std::endl;

    if (use_threading)
        std::cout << "use up to " << std::thread::hardware_concurrency() << " threads" << std::endl;

    if (minimize_angle)
    {
        std::cout << "minimize total angle (AAGP)" << std::endl;
    } else {
        std::cout << "minimize total number of floodlights (AGP)" << std::endl;
    }

    if (!polygon.is_simple())
    {
        std::cerr << "Polygon is not simple" << std::endl;
        return -1;
    }

    if (!polygon.is_counterclockwise_oriented())
    {
        if (!solution_path.empty())
        {
            std::cerr << "Polygon is not counterclockwise oriented, cannot verify solution" << std::endl;
            return -1;
        }
        polygon.reverse_orientation();
    }

    svg::Document doc_polygon(instance_path.stem().string() + ".svg");
    doc_polygon << svg::Polygon_(polygon);
    doc_polygon.save();

    if (!solution_path.empty())
    {
        solution = AAGP::serialization::read_solution<Epeck>(solution_path, polygon);
    } else {
        AAGP::IPApproximation<Epeck> approx_solver(polygon, utils::conversion::to_radians(max_angle), true);

        if (!minimize_angle) approx_solver.minimize_floodlight_num();
        approx_solver.solve_exact(exact_kernel);
        approx_solver.set_silent(silent);
        approx_solver.set_svg_verbose(svg_verbose);
        approx_solver.use_threading(use_threading);

        if (timeout > 0)
        {
            utils::timeout(timeout, [&]{
                approx_solver.compute();
            });
        } else {
            approx_solver.compute();
        }

        //try {
        //    approx_solver.compute();
        //} catch (const std::exception &ex)
        //{
        //    std::cerr << "Error occurred while solving " << instance_path << ": " << ex.what() << std::endl;
        //    return -1;
        //}

        solution = approx_solver.solution();

        std::string solution_filename;
        if (!outpath.empty())
        {
            solution_filename = outpath.string() + "/" + instance_path.stem().string() + "_" + std::to_string(max_angle);
        } else {
            solution_filename = instance_path.parent_path().string() + "/" + instance_path.stem().string() + "_" + std::to_string(max_angle);
        }

        if (!minimize_angle)
            solution_filename += "_minimizenum";

        AAGP::serialization::write_solution(approx_solver.solution(), solution_filename + ".sol");
        //AAGP::svg_floodlight_placement(solution_filename + "_solution.svg", solution, polygon);

        std::cout << std::endl << approx_solver.statistics() << std::endl;
    }

    if (varify)
    {
        std::pair<bool, double> verify = AAGP::verify_solution(polygon, solution);
        if(verify.first)
        {
            std::cout << "Solution is valid :)" << std::endl;
        }
        else {
            std::cout << "Solution covers only " << (100 * verify.second) << "% of the polygon :(" << std::endl;
        }
    }

    return 0;
}