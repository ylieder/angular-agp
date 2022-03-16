//
// Created by Yannic Lieder on 03.10.19.
//

#ifndef ANGULARARTGALLERYPROBLEM_SERIALIZATION_H
#define ANGULARARTGALLERYPROBLEM_SERIALIZATION_H

#include <CGAL/Cartesian_converter.h>
#include <CGAL/Quotient.h>
#include <CGAL/MP_Float.h>
#include <CGAL/Simple_cartesian.h>

#include <boost/filesystem.hpp>

#include <limits>
#include <regex>

#include "floodlight/floodlight.h"

namespace fs = boost::filesystem;
using QKernel = CGAL::Simple_cartesian<CGAL::Quotient<CGAL::MP_Float>>;

namespace AAGP
{
    namespace serialization {
        template<class Kernel>
        CGAL::Polygon_2<Kernel> read_agplib_file(fs::path file)
        {
            if (!fs::is_regular_file(file))
            {
                throw std::invalid_argument("Not a file: " + file.string());
            }


            fs::ifstream stream{file};
            CGAL::Polygon_2<Kernel> polygon;

            std::string line;
            std::string first, second;

            // skip first line
            stream >> first;
            int size = std::stoi(first);

            const std::regex r("([0-9]*)/([0-9]*)");

            while (stream >> first >> second)
            {
                std::smatch sm;
                regex_search(first, sm, r);

                typename Kernel::FT x_num = std::stoi(sm[1]);
                typename Kernel::FT x_den = std::stoi(sm[2]);

                regex_search(second, sm, r);
                typename Kernel::FT y_num = std::stoi(sm[1]);
                typename Kernel::FT y_den = std::stoi(sm[2]);

                typename Kernel::FT x = x_num / x_den;
                typename Kernel::FT y = y_num / y_den;

                polygon.push_back(CGAL::Point_2<Kernel>(x, y));
            }

            assert(polygon.size() == size);
            return polygon;
        }

        template<class Kernel>
        CGAL::Polygon_2<Kernel> read_file(fs::path file)
        {
            if (!fs::is_regular_file(file))
            {
                throw std::invalid_argument("Not a file: " + file.string());
            }

            fs::ifstream stream{file};
            CGAL::Polygon_2<Kernel> polygon;

            std::string s;
            std::getline(stream, s);
            int line_number = 1;

            while (std::getline(stream, s))
            {
                try
                {
                    std::istringstream iss(s);
                    typename Kernel::FT x, y;
                    iss >> x >> y;
                    polygon.push_back(CGAL::Point_2<Kernel>(x, y));
                }
                catch (std::exception &ex)
                {
                    throw std::runtime_error("Cannot read file, error in line " + std::to_string(line_number));
                }
            }

            /*
            double x, y;
            while (stream >> x >> y)
            {
                polygon.push_back(CGAL::Point_2<Kernel>(x, y));
            }
            */

            return polygon;
        }

        template <typename Kernel>
        bool write_file(CGAL::Polygon_2<Kernel> &polygon, fs::path file)
        {
            /*
            if (fs::exists(file))
            {
                throw std::invalid_argument("File exists: " + file.string());
            }
            */

            if (!file.parent_path().empty() && !fs::exists(file.parent_path()))
            {
                fs::create_directories(file.parent_path());
            }

            fs::ofstream stream{file};


            for (auto vit = polygon.vertices_begin(); vit != polygon.vertices_end(); ++vit)
            {
                stream << CGAL::exact(vit->x()) << " " << CGAL::exact(vit->y()) << std::endl;
                //stream << std::setprecision(15) << CGAL::to_double(vit->x()) << " " << CGAL::to_double(vit->y()) << std::endl;
            }

            return true;
        }

        template <typename Kernel>
        static std::vector<Floodlight<Kernel>> read_solution(const fs::path &file, const CGAL::Polygon_2<Kernel> &polygon)
        {
            if (!fs::is_regular_file(file))
            {
                throw std::invalid_argument("Not a file: " + file.string());
            }

            fs::ifstream stream{file};
            std::vector<Floodlight<Kernel>> floodlights;

            int line = 1;
            size_t vertex_index;
            double v1_x, v1_y, v2_x, v2_y;
            while (stream >> vertex_index >> v1_x >> v1_y >> v2_x >> v2_y)
            {
                if (vertex_index >= polygon.size())
                {
                    throw std::runtime_error("invalid vertex index in line " + std::to_string(line) + ": "
                                             + std::to_string(vertex_index) + " >= " + std::to_string(polygon.size()));
                }
                floodlights.emplace_back(polygon.vertex(vertex_index),
                                         CGAL::Vector_2<Kernel>(v1_x, v1_y),
                                         CGAL::Vector_2<Kernel>(v2_x, v2_y), vertex_index);

                ++line;
            }

            return floodlights;
        }

        template <typename Kernel>
        static double write_solution(const std::vector<Floodlight<Kernel>> &floodlights, const fs::path &file, bool rounding_up = true)
        {
            const std::size_t max_precision = std::numeric_limits<double>::digits;

            /*
            if (fs::exists(file))
            {
                throw std::invalid_argument("File exists: " + file.string());
            }
            */

            if (!file.parent_path().empty() && !fs::exists(file.parent_path()))
            {
                fs::create_directories(file.parent_path());
            }

            fs::ofstream stream{file};

            for (auto &f: floodlights) {
                stream << f.vertex_index << " "
                << CGAL::exact(f.v1.x()) << " " << CGAL::exact(f.v1.y()) << " "
                << CGAL::exact(f.v2.x()) << " " << CGAL::exact(f.v2.y()) << std::endl;
            }

            return 0;

            /*
            double rounding_angle = 0;

            fs::ofstream stream{file};

            for (auto &f: floodlights)
            {
                double v1_x, v1_y, v2_x, v2_y;
                if (rounding_up)
                {
                    switch(utils::cgal::quadrant(f.v1))
                    {
                        case 0:
                            v1_x = CGAL::to_interval(f.v1.x()).second;
                            v1_y = CGAL::to_interval(f.v1.y()).first;
                            break;
                        case 1:
                            v1_x = CGAL::to_interval(f.v1.x()).second;
                            v1_y = CGAL::to_interval(f.v1.y()).second;
                            break;
                        case 2:
                            v1_x = CGAL::to_interval(f.v1.x()).first;
                            v1_y = CGAL::to_interval(f.v1.y()).second;
                            break;
                        case 3:
                            v1_x = CGAL::to_interval(f.v1.x()).first;
                            v1_y = CGAL::to_interval(f.v1.y()).first;
                            break;
                        default:
                            assert(false);
                    }

                    switch(utils::cgal::quadrant(f.v2))
                    {
                        case 0:
                            v2_x = CGAL::to_interval(f.v2.x()).first;
                            v2_y = CGAL::to_interval(f.v2.y()).second;
                            break;
                        case 1:
                            v2_x = CGAL::to_interval(f.v2.x()).first;
                            v2_y = CGAL::to_interval(f.v2.y()).first;
                            break;
                        case 2:
                            v2_x = CGAL::to_interval(f.v2.x()).second;
                            v2_y = CGAL::to_interval(f.v2.y()).first;
                            break;
                        case 3:
                            v2_x = CGAL::to_interval(f.v2.x()).second;
                            v2_y = CGAL::to_interval(f.v2.y()).second;
                            break;
                        default:
                            assert(false);
                    }
                } else {
                    v1_x = CGAL::to_double(f.v1.x());
                    v1_y = CGAL::to_double(f.v1.y());
                    v2_x = CGAL::to_double(f.v2.x());
                    v2_y = CGAL::to_double(f.v2.y());
                }

                rounding_angle += utils::cgal::angle(CGAL::Vector_2<Kernel>(v1_x, v1_y), f.v1)
                                  + utils::cgal::angle(f.v2, CGAL::Vector_2<Kernel>(v2_x, v2_y));

                stream << std::setprecision(max_precision) << CGAL::to_double(f.vertex_index) << " "
                       << v1_x << " " << v1_y << " "
                       << v2_x << " " << v2_y << std::endl;
            }

            return rounding_angle;
            */
        }
    }
}

#endif //ANGULARARTGALLERYPROBLEM_SERIALIZATION_H
