//
// Created by Yannic Lieder on 04.10.19.
//

#include <CGAL/random_polygon_2.h>
#include <CGAL/point_generators_2.h>

#include <random>
#include <integer_program/aagp_approximation.h>

#include "utils/conversion_utils.h"
#include "utils/polygon_utils.h"
#include "utils/progress_bar.h"
#include "utils/timeout.h"

#include "include/simple_svg/simple_svg_1.0.0.hpp"
#include "integer_program/aagp_approximation.h"
#include "verify_solution.h"
#include "serialization.h"

using Epeck          = CGAL::Exact_predicates_exact_constructions_kernel;
using PointGenerator = CGAL::Random_points_in_square_2<CGAL::Point_2<Epeck>>;

int main(int argc, const char* argv[])
{
    int timeout_min = 2 * 60 * 1000;

    fs::path instance_path = "resources/instances";
    fs::path solution_path = "resources/solutions";
    fs::path svg_path = "resources/solutions_svg";

    fs::path instance_directory = "CGALRAND";

    if (!fs::exists(solution_path))
        fs::create_directories(solution_path);

    if (!fs::exists(svg_path))
        fs::create_directories(svg_path);

    if (!fs::exists(solution_path / "statistics.csv"))
    {
        std::ofstream statistics_file;
        statistics_file.open((solution_path / "statistics.csv").string(), std::ios_base::app);
        statistics_file << "instance_name;size;max_floodlight_angle;total_angle;number_of_floodlights;number_of_candidates;number_of_cells;"
                        <<"time_total;time_arrangement_build;time_cell_centroid_computation;time_floodlight_cell_mapping;time_ip" << std::endl;
    }


    int angles[5][2] = {{20, 100}, {10, 100}, {5, 50}, {2, 20}, {1, 10}};
    std::vector<std::string> current_sub_path;

    auto path_difference = [] (const fs::path & basepath, const fs::path & path)
    {
        fs::path diffpath;
        fs::path tmppath = path;
        while(!fs::equivalent(tmppath, basepath)) {
            diffpath = tmppath.stem() / diffpath;
            tmppath = tmppath.parent_path();
        }
        return diffpath;
    };

    for (int &(angle[2]) : angles)
    {
        std::cout << angle[0] << std::endl;
        std::function<void(fs::path)> recursive_dir_search = [&] (const fs::path &path)
        {
            fs::directory_iterator it_end;
            for (fs::directory_iterator it(path); it != it_end; ++it)
            {
                if (fs::is_directory(it->path()))
                {
                    recursive_dir_search(it->path());
                }
                else if (fs::extension(it->path()) == ".pol" ) {
                    std::cout << "\t" << it->path() << std::endl;

                    fs::path relative = path_difference(instance_path, it->path().parent_path());

                    if (!fs::exists(solution_path / relative))
                        fs::create_directories(solution_path / relative);

                    if (!fs::exists(svg_path / relative))
                        fs::create_directories(svg_path / relative);

                    //auto polygon = AAGP::serialization::read_agplib_file<Epeck>(it->path());
                    auto polygon = AAGP::serialization::read_file<Epeck>(it->path());

                    if (polygon.size() > angle[1])
                        continue;

                    AAGP::IPApproximation<Epeck> approximation_solver(polygon, utils::conversion::to_radians(angle[0]),
                                                                      true);

                    approximation_solver.use_threading(true);
                    approximation_solver.set_silent(true);

                    try
                    {
                        utils::timeout(timeout_min, [&] {
                            approximation_solver.compute();
                        });

                        auto stats = approximation_solver.statistics();

                        std::pair<bool, double> verify = AAGP::verify_solution(polygon, approximation_solver.solution());

                        std::ofstream statistics_file;
                        statistics_file.open((solution_path / "statistics.csv").string(), std::ios_base::app);

                        if (verify.first)
                        {
                            statistics_file << it->path() << ";" << polygon.size() << ";" << angle << ";"
                                            << utils::conversion::to_degree(stats.angle) << ";" << stats.num_floodlights << ";"
                                            << stats.num_floodlight_candidates << ";" << stats.num_cells << ";"
                                            << stats.time.total.count() << ";" << stats.time.build_arrangement.count() << ";"
                                            << stats.time.compute_cell_centroids.count() << stats.time.floodlight_cell_mapping.count() << ";"
                                            << stats.time.ip.total.count() << std::endl;
                        } else {
                            statistics_file << it->path() << ";" << polygon.size() << ";" << angle << ";notvalid" << std::endl;
                            std::cerr << it->path() << ";" << polygon.size() << ";" << angle << ";notvalid" << std::endl;
                        }

                        std::string instance_name = (it->path()).filename().stem().string();

                        AAGP::serialization::write_solution(approximation_solver.solution(), solution_path / relative / (instance_name + "_" + std::to_string(angle) + ".solution"));
                        AAGP::svg_floodlight_placement(svg_path / relative / (instance_name + "_" + std::to_string(angle) + ".svg"), approximation_solver.solution(), polygon);
                    }
                    catch (std::runtime_error &ex)
                    {
                        std::ofstream statistics_file;
                        statistics_file.open((solution_path / "statistics.csv").string(), std::ios_base::app);
                        statistics_file << it->path() << ";" << polygon.size() << ";" << angle << ";exception=" << ex.what() << std::endl;
                        std::cerr << it->path() << ";" << polygon.size() << ";" << angle << ";exception=" << ex.what() << std::endl;
                    }
                    catch (std::logic_error &ex)
                    {
                        std::ofstream statistics_file;
                        statistics_file.open((solution_path / "statistics.csv").string(), std::ios_base::app);
                        statistics_file << it->path() << ";" << polygon.size() << ";" << angle << ";exception=" << ex.what() << std::endl;
                        std::cerr << it->path() << ";" << polygon.size() << ";" << angle << ";exception=" << ex.what() << std::endl;
                    }
                    catch (std::exception &ex)
                    {
                        std::ofstream statistics_file;
                        statistics_file.open((solution_path / "statistics.csv").string(), std::ios_base::app);
                        statistics_file << it->path() << ";" << polygon.size() << ";" << angle << ";exception=" << ex.what() << std::endl;
                        std::cerr << it->path() << ";" << polygon.size() << ";" << angle << ";exception=" << ex.what() << std::endl;
                    }
                }
            }
        };

        recursive_dir_search(instance_path / instance_directory);
    }
}