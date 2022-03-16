//
// Created by Yannic Lieder on 26.09.19.
//

#ifndef ANGULARARTGALLERYPROBLEM_IP_APPROXIMATION_H
#define ANGULARARTGALLERYPROBLEM_IP_APPROXIMATION_H

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

#include <CGAL/Simple_polygon_visibility_2.h>
#include <CGAL/Triangular_expansion_visibility_2.h>

#include <CGAL/Arr_naive_point_location.h>
#include <CGAL/Arr_batched_point_location.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Cartesian_converter.h>

#include <boost/filesystem.hpp>

#include <chrono>

#include "utils/cgal_utils.h"
#include "utils/common_utils.h"
#include "utils/random_utils.hpp"
#include "utils/progress_bar.h"

#include "floodlight/floodlight.h"
#include "floodlight/svg_floodlight.h"

#include "aagp_ip_solver.h"

#include "utils/profiling.h"
#include "utils/progress_bar.h"


namespace fs = boost::filesystem;

using Epeck          = CGAL::Exact_predicates_exact_constructions_kernel;
using Epick          = CGAL::Exact_predicates_inexact_constructions_kernel;

namespace AAGP {

    struct IPStatistics {
        int num_floodlights;
        int num_floodlight_candidates;
        int num_cells;

        double angle;

        struct {
            std::chrono::milliseconds total;
            std::chrono::milliseconds build_arrangement;
            std::chrono::milliseconds compute_cell_centroids;
            std::chrono::milliseconds floodlight_cell_mapping;

            struct {
                std::chrono::milliseconds total;
                std::chrono::milliseconds add_obj_func;
                std::chrono::milliseconds add_cell_constraints;
                std::chrono::milliseconds solve;
                std::chrono::milliseconds write_solution;
            } ip;
        } time;

        friend std::ostream& operator<< (std::ostream& stream, const IPStatistics& stats) {
            stream << "Instance statistics:" << "\n\t"
                   << "Number of floodlight candidates: " << stats.num_floodlight_candidates << "\n\t"
                   << "Number of cells: " << stats.num_cells << "\n\t"
                   << "Number of floodlights: " << stats.num_floodlights << "\n\t"
                   << "Total angle: " << utils::conversion::to_degree(stats.angle) << "°\n\n\t";

            stream << "Time: " << stats.time.total.count() << "ms (total)\n\t\t"
                   << "Build arrangement: " << stats.time.build_arrangement.count() << "ms\n\t\t"
                   << "Compute cell centroids: " << stats.time.compute_cell_centroids.count() << "ms\n\t\t"
                   << "Floodlight cell mapping: " << stats.time.floodlight_cell_mapping.count() << "ms\n\t\t"
                   << "IP: " << stats.time.ip.total.count() << "ms (total)\n\t\t\t"
                   << "Construct objective function: " << stats.time.ip.add_obj_func.count() << "ms\n\t\t\t"
                   << "Add cell constraints: " << stats.time.ip.add_cell_constraints.count() << "ms\n\t\t\t"
                   << "Solve: " << stats.time.ip.solve.count() << "ms" << std::endl;

            return stream;
        }
    };

    template <typename TKernel>
    using FloodlightVertex = std::vector<Floodlight<TKernel>>;

    template <typename Kernel>
    class IPApproximation {
    public:
        IPApproximation(const CGAL::Polygon_2<Kernel> &polygon, double max_guard_angle, bool delayed = false) :
            _polygon(polygon),
            _max_guard_angle(max_guard_angle)
        {
            if (!delayed) compute();
        };

        void compute()
        {
            if (svg_verbose && !fs::exists("verbose"))
            {
                fs::create_directories("verbose/visibility_polygons");
                fs::create_directories("verbose/floodlight_candidates");
            }

            _stats.time.total = measure_time<std::chrono::milliseconds>([&] {
                log("* Partition polygon");
                _stats.time.build_arrangement = measure_time<std::chrono::milliseconds>([&] {
                    partition_polygon();
                });
                log("\t" + std::to_string(total_num_floodlights) + " floodlight candidates");

                log("* Compute cell centroids");
                _stats.time.compute_cell_centroids = measure_time<std::chrono::milliseconds>([&] {
                    compute_cell_centroids();
                });
                _stats.num_cells = cell_centroids.size();
                log("\t" + std::to_string(cell_centroids.size()) + " cells");

                if (svg_verbose)
                {
                    svg::Document doc_arrangement("verbose/arrangement.svg");
                    doc_arrangement << svg::Arrangement_(_arrangement);
                    doc_arrangement.save();
                }

                log("* Compute floodlight cell mapping");
                _stats.time.floodlight_cell_mapping = measure_time<std::chrono::milliseconds>([&] {
                    floodlight_cell_mapping();
                });

                int num_floodlight_candidates = 0;
                for (auto &fv : floodlights)
                {
                    num_floodlight_candidates += fv.size();
                }
                _stats.num_floodlight_candidates = num_floodlight_candidates;

                for (int i = 0; i < cell_centroids.size(); ++i)
                {
                    if (visible_floodlights[i].empty())
                        throw std::logic_error("Error: some cell centroids are not visible by any floodlight");
                }

                log("* Solve IP");
                log();
                typename IPSolver<Kernel>::ResultType result;
                _stats.time.ip.total = measure_time<std::chrono::milliseconds>([&] {
                    IPSolver<Kernel> solver(visible_floodlights, floodlights, num_floodlight_candidates, logging, _minimize_angle);
                    result = solver.solve();
                });

                _stats.time.ip.add_obj_func = result.time.add_obj_func;
                _stats.time.ip.add_cell_constraints = result.time.add_cell_constraints;
                _stats.time.ip.solve = result.time.solve;

                log("* Merge floodlight neighborhood");
                _solution = merge_neighbored_floodlights(result.solution); // TODO: change to result.solution

                _stats.angle = result.value;
                _stats.num_floodlights = _solution.size();
            });

        }

        const IPStatistics & statistics() const
        {
            return _stats;
        }

         const std::vector<Floodlight<Kernel>> solution() const
        {
            return _solution;
        }

        void use_threading(bool flag){
            if (flag)
            {
                _use_threading = true;
                _thread_num = std::thread::hardware_concurrency();
            } else {
                _use_threading = false;
                _thread_num = 1;
            }
        };

        void minimize_floodlight_num() { _minimize_angle = false; }
        void minimize_angle() { _minimize_angle = true; }
        void solve_exact(bool flag = true) { _exact_kernel = flag; }
        void set_silent(bool flag = true) { logging = !flag; }
        void set_svg_verbose(bool flag = true) {svg_verbose = flag; }
    private:
        CGAL::Polygon_2<Kernel> _polygon;
        CGAL::Arrangement_2<CGAL::Arr_segment_traits_2<Epeck>> _arrangement;
        std::vector<Floodlight<Kernel>> _solution;

        std::vector<FloodlightVertex<Kernel>> floodlights;
        double _max_guard_angle;
        int total_num_floodlights = 0;

        std::vector<CGAL::Point_2<Kernel>> cell_centroids;
        std::vector<std::vector<std::pair<int,int>>> visible_floodlights;


        IPStatistics _stats;

        bool _minimize_angle = true;
        bool _exact_kernel = false;
        bool logging = true;
        bool svg_verbose = false;

        bool _use_threading = true;
        int _thread_num = 1;


        void log(const std::string & message, bool new_line = true)
        {
            if (logging)
            {
                std::cout << message;
                if (new_line) std::cout << std::endl;
            }

        }

        void log()
        {
            if (logging) std::cout << std::endl;
        }

        void partition_polygon()
        {
            std::vector<CGAL::Segment_2<Kernel>> segments;

            CGAL::Arrangement_2<CGAL::Arr_segment_traits_2<Kernel>> polygon_arr;
            CGAL::insert(polygon_arr, _polygon.edges_begin(), _polygon.edges_end());

            std::for_each(_polygon.edges_begin(), _polygon.edges_end(), [&segments](const typename CGAL::Polygon_2<Kernel>::Segment_2 &e)
            {
                segments.emplace_back(e.source(), e.target());
            });

            int vertex_index = 0;
            auto ci = _polygon.vertices_circulator();
            auto begin = _polygon.vertices_circulator();
            do {
                CGAL::Arr_naive_point_location<CGAL::Arrangement_2<CGAL::Arr_segment_traits_2<Kernel>>> pl(polygon_arr);
                typename CGAL::Arr_point_location_result<CGAL::Arrangement_2<CGAL::Arr_segment_traits_2<Kernel>>>::Type obj = pl.locate(*ci);

                CGAL::Arrangement_2<CGAL::Arr_segment_traits_2<Kernel>> vp_output;
                CGAL::Simple_polygon_visibility_2<CGAL::Arrangement_2<CGAL::Arr_segment_traits_2<Kernel>>, CGAL::Tag_false> non_regular_visibility(polygon_arr);
                CGAL::Triangular_expansion_visibility_2<CGAL::Arrangement_2<CGAL::Arr_segment_traits_2<Kernel>>, CGAL::Tag_false> tev(polygon_arr);

                typename CGAL::Arrangement_2<CGAL::Arr_segment_traits_2<Kernel>>::Halfedge_const_handle preceding_he =
                        std::find_if(polygon_arr.halfedges_begin(), polygon_arr.halfedges_end(),
                                     [&ci](const typename CGAL::Arrangement_2<CGAL::Arr_segment_traits_2<Kernel>>::Halfedge &e) {
                                         return !e.face()->is_unbounded() &&
                                                e.target()->point() == *ci;
                                     });
                assert(preceding_he != polygon_arr.halfedges_end());

                svg::Document vp_doc("verbose/visibility_polygons/vp_" + std::to_string(vertex_index) + ".svg");
                if (svg_verbose) vp_doc << svg::Polygon_(_polygon);

                tev.compute_visibility(*ci, preceding_he, vp_output);
                for (auto eit = vp_output.edges_begin(); eit != vp_output.edges_end(); ++eit) {
                    segments.push_back(eit->curve());
                    if (svg_verbose) vp_doc << svg::Line_(CGAL::Segment_2<Kernel>(eit->curve()), svg::Stroke(0.5, svg::Color::Red));
                }

                if (svg_verbose) vp_doc.save();

                FloodlightVertex<Kernel> fv;

                double current_angle = acos(CGAL::to_double(utils::cgal::cosine_angle<Kernel>(ci))); // TODO: inexact / doesn't work with Epeck
                if (CGAL::right_turn(*(ci - 1), *ci, *(ci + 1))) //  TODO: orientation of polygon is crucial
                {
                    current_angle = 2 * M_PI - current_angle;
                }

                int guards_num = (int)ceil(current_angle / _max_guard_angle);
                double specific_guard_angle = current_angle / guards_num;

                svg::Document guard_doc("verbose/floodlight_candidates/floodlight_candidates_" + std::to_string(vertex_index) + ".svg");
                if (svg_verbose) guard_doc << svg::Polygon_(_polygon);

                CGAL::Vector_2<Kernel> end_vector = utils::cgal::normalize_vector(CGAL::Vector_2<Kernel>(*ci, *(ci - 1)));
                CGAL::Vector_2<Kernel> old_vector = utils::cgal::normalize_vector(CGAL::Vector_2<Kernel>(*ci, *(ci + 1)));

                int i = 0;
                for (; i < guards_num - 1; ++i)
                {
                    CGAL::Vector_2<Kernel> new_vector = utils::cgal::rotate_vector(old_vector, specific_guard_angle);
                    fv.emplace_back(*ci, old_vector, new_vector, vertex_index);
                    old_vector = new_vector;

                    CGAL::Ray_2<Kernel> ray(*ci, new_vector);
                    CGAL::Point_2<Kernel> intersection = utils::cgal::primitive_polygon_ray_intersection(_polygon, ray);

                    CGAL::Segment_2<Kernel> seg(*ci, intersection);

                    if (svg_verbose) guard_doc << svg::Line_(seg, svg::Stroke(0.5, svg::Color::Red));

                    assert(*ci != intersection);
                    segments.push_back(seg);
                }

                fv.emplace_back(*ci, old_vector, utils::cgal::normalize_vector(CGAL::Vector_2<Kernel>(*ci, *(ci - 1))), vertex_index); // //  TODO: orientation of polygon is crucial
                total_num_floodlights += fv.size();
                this->floodlights.push_back(fv);


                if (svg_verbose) guard_doc.save();

                ++vertex_index;
            } while (++ci != begin);

            CGAL::insert(_arrangement, segments.begin(), segments.end());
        };

        void compute_cell_centroids()
        {
            if (_thread_num > 1)
            {
                //compute_cell_centroids_w_threading(); // TODO: multithreading leads to malloc error
                compute_cell_centroids_wo_threading();
            } else {
                compute_cell_centroids_wo_threading();
            }
        }

        void compute_cell_centroids_wo_threading()
        {
            for (auto fit = _arrangement.faces_begin(); fit != _arrangement.faces_end(); ++fit)
            {
                if (!fit->is_unbounded())
                {
                    auto centroid = utils::cgal::centroid<Kernel>(fit);
                    cell_centroids.emplace_back(centroid);
                }
            }
        }

        void compute_cell_centroids_w_threading()
        {
            // TODO: doesn't work:
            // AAGP(27869,0x1196755c0) malloc: tiny_free_list_remove_ptr: Internal invariant broken (next ptr of prev): ptr=0x7ffc15168f50, prev_next=0x7ffc15168f40
            // AAGP(27869,0x1196755c0) malloc: *** set a breakpoint in malloc_error_break to debug

            int num_faces = _arrangement.number_of_faces();

            std::vector<typename CGAL::Arrangement_2<CGAL::Arr_segment_traits_2<Epeck>>::Face_const_handle> faces;

            for (auto fit = _arrangement.faces_begin(); fit != _arrangement.faces_end(); ++fit)
                faces.push_back(fit);

            std::mutex mutex;
            std::vector<std::thread> threads;
            for (int i = 0; i < _thread_num; ++i)
            {
                threads.push_back(std::thread(
                    [&, i] () {
                        for (int f_index = i; f_index < num_faces; f_index += _thread_num)
                        {
                            if (!faces[f_index]->is_unbounded())
                            {
                                auto centroid = utils::cgal::centroid<Kernel>(faces[f_index]);
                                mutex.lock();
                                cell_centroids.emplace_back(centroid);
                                mutex.unlock();
                            }
                        }
                    }
                ));
            }

            for (int i = 0; i < _thread_num; ++i)
            {
                threads[i].join();
            }
        }

        void floodlight_cell_mapping()
        {
            if (_exact_kernel)
            {
                log("\tusing exact predicates, exact construction kernel");
                floodlight_cell_mapping_e();
            }
            else
            {
                log("\tusing exact predicates, inexact construction kernel");
                if (_thread_num > 1)
                {
                    floodlight_cell_mapping_ie_w_threading();
                } else {
                    floodlight_cell_mapping_ie_wo_threading();
                }
            }
        }

        void floodlight_cell_mapping_ie_w_threading()
        {
            CGAL::Cartesian_converter<Epeck, Epick> to_epick;

            visible_floodlights = std::vector<std::vector<std::pair<int,int>>>(cell_centroids.size());
            std::vector<CGAL::Point_2<Epick>> ie_cell_centroids(cell_centroids.size());

            CGAL::Polygon_2<Epick> ie_polygon;

            for (auto vit = _polygon.vertices_begin(); vit != _polygon.vertices_end(); ++vit)
            {
                ie_polygon.push_back(to_epick(*vit));
            }

            int c_index = 0;
            for (auto &c : cell_centroids)
            {
                ie_cell_centroids[c_index] = to_epick(c);
                ++c_index;
            }


            utils::ProgressBar pb(ie_polygon.size(), false);

            if (logging)
            {
                log("\t", false);
                pb.start();
            }

            std::mutex mutex;
            std::vector<std::thread> threads;
            for (int i = 0; i < _thread_num; ++i)
            {
                threads.push_back(std::thread([&, i]
                {
                    for (int v_index = i; v_index < ie_polygon.size(); v_index += _thread_num)
                    {
                        //mutex.lock();
                        //std::cout << i << " " << v_index << std::endl;
                        //mutex.unlock();
                        const CGAL::Point_2<Epick> * vit = &ie_polygon[v_index];

                       CGAL::Direction_2<Epick> dir_v1 =  to_epick(floodlights[v_index][0].v1.direction());
                       CGAL::Direction_2<Epick> dir_v2 =  to_epick(floodlights[v_index][floodlights[v_index].size() - 1].v2.direction());

                        auto next = v_index == (ie_polygon.size() - 1) ? &ie_polygon[0] : &ie_polygon[v_index + 1];
                        //mutex.unlock();

                        std::vector<std::pair<int, const CGAL::Point_2<Epick> * >> cell_candidates;
                        int c_index_2 = 0;
                        for (const CGAL::Point_2<Epick> &c : ie_cell_centroids)
                        {
                            auto dir_c = CGAL::Vector_2<Epick>(*vit, c).direction();
                            if (dir_c.counterclockwise_in_between(dir_v1, dir_v2)) {
                                if(!utils::cgal::primitive_polygon_segment_intersection(ie_polygon, CGAL::Segment_2<Epick>(*vit, c)))
                                {
                                    cell_candidates.push_back(std::make_pair(c_index_2, &c));
                                };
                            }
                            ++c_index_2;
                        }

                        utils::cgal::PolarAngleLess<Epick> pal(*vit, *next);

                        // sort by polar angle
                        std::sort(cell_candidates.begin(), cell_candidates.end(),
                                  [&pal](const std::pair<int, const CGAL::Point_2<Epick>*> &lhs, const std::pair<int, const CGAL::Point_2<Epick>*> &rhs)
                                {
                                    return pal(*lhs.second, *rhs.second);
                                }
                        );

                        int current_floodlight_index = 0;
                        for (auto &c : cell_candidates)
                        {
                            CGAL::Vector_2<Epick> v_p(*vit, *c.second);
                            while(current_floodlight_index < floodlights[v_index].size() && !v_p.direction().counterclockwise_in_between(to_epick(floodlights[v_index][current_floodlight_index].v1).direction(), to_epick(floodlights[v_index][current_floodlight_index].v2).direction()))
                            {
                                ++current_floodlight_index;
                            }

                            assert(current_floodlight_index < floodlights[v_index].size() && current_floodlight_index >= 0);

                            if (!v_p.direction().counterclockwise_in_between(to_epick(floodlights[v_index][current_floodlight_index].v1).direction(), to_epick(floodlights[v_index][current_floodlight_index].v2).direction()))
                                throw std::logic_error("err");

                            mutex.lock();
                            visible_floodlights[c.first].push_back(std::make_pair(v_index, current_floodlight_index));
                            floodlights[v_index][current_floodlight_index].visible_cells.push_back(c.first);
                            mutex.unlock();
                        }

                        mutex.lock();
                        ++pb;
                        mutex.unlock();
                    }
                }));
            }

            for (int i = 0; i < _thread_num; ++i)
            {
                threads[i].join();
            }
        }

        void floodlight_cell_mapping_ie_wo_threading()
        {
            CGAL::Cartesian_converter<Epeck, Epick> to_epick;

            visible_floodlights = std::vector<std::vector<std::pair<int,int>>>(cell_centroids.size());
            std::vector<CGAL::Point_2<Epick>> ie_cell_centroids(cell_centroids.size());

            CGAL::Polygon_2<Epick> ie_polygon;

            for (auto vit = _polygon.vertices_begin(); vit != _polygon.vertices_end(); ++vit)
            {
                ie_polygon.push_back(to_epick(*vit));
            }

            int c_index = 0;
            for (auto &c : cell_centroids)
            {
                ie_cell_centroids[c_index] = to_epick(c);
                ++c_index;
            }

            log("\t", false);
            utils::ProgressBar pb(ie_polygon.size(), logging);
            int v_index = 0;
            for (auto vit = ie_polygon.vertices_begin(); vit != ie_polygon.vertices_end(); ++vit)
            {
                std::vector<std::pair<int, CGAL::Point_2<Epick>*>> visible_cells;

                c_index = 0;
                for (auto &c : ie_cell_centroids)
                {
                    if (utils::cgal::counterclockwise_in_between(*vit, c, to_epick(floodlights[v_index][0].v1), to_epick(floodlights[v_index][floodlights[v_index].size() - 1].v2))){
                        if(!utils::cgal::primitive_polygon_segment_intersection(ie_polygon, CGAL::Segment_2<Epick>(*vit, c)))
                        {
                            visible_cells.push_back(std::make_pair(c_index, &c));
                        };
                    }
                    ++c_index;
                }

                auto next = (vit + 1) == ie_polygon.vertices_end() ? ie_polygon.vertices_begin() : (vit + 1);

                utils::cgal::PolarAngleLess<Epick> pal(*vit, *next);

                // sort by polar angle
                std::sort(visible_cells.begin(), visible_cells.end(), [&pal](const std::pair<int, CGAL::Point_2<Epick>*> &lhs, const std::pair<int, CGAL::Point_2<Epick>*> &rhs)
                {
                    return pal(*lhs.second, *rhs.second);
                });

                int current_floodlight_index = 0;
                for (auto &c : visible_cells)
                {
                    CGAL::Vector_2<Epick> v_p(*vit, *c.second);
                    while(current_floodlight_index < floodlights[v_index].size() && !v_p.direction().counterclockwise_in_between(to_epick(floodlights[v_index][current_floodlight_index].v1).direction(), to_epick(floodlights[v_index][current_floodlight_index].v2).direction()))
                    {
                        ++current_floodlight_index;
                    }

                    assert(current_floodlight_index < floodlights[v_index].size() && current_floodlight_index >= 0);

                    if (!v_p.direction().counterclockwise_in_between(to_epick(floodlights[v_index][current_floodlight_index].v1).direction(), to_epick(floodlights[v_index][current_floodlight_index].v2).direction()))
                        throw std::logic_error("err");

                    visible_floodlights[c.first].push_back(std::make_pair(v_index, current_floodlight_index));
                    floodlights[v_index][current_floodlight_index].visible_cells.push_back(c.first);
                }
                ++v_index;
                ++pb;
            }
        }

        void floodlight_cell_mapping_e()
        {
            visible_floodlights = std::vector<std::vector<std::pair<int,int>>>(cell_centroids.size());

            std::map<CGAL::Point_2<Kernel>, int> cell_index;
            for (int i = 0; i < cell_centroids.size(); ++i)
            {
                cell_index[cell_centroids[i]] = i;
            }

            CGAL::Arrangement_2<CGAL::Arr_segment_traits_2<Kernel>> polygon_arr;
            CGAL::insert_non_intersecting_curves(polygon_arr, _polygon.edges_begin(), _polygon.edges_end());

            log("\t", false);
            utils::ProgressBar pb(_polygon.size(), logging);
            for (int i = 0; i < _polygon.size(); ++i)
            {
                auto vertex = _polygon.vertex(i);

                CGAL::Arrangement_2<CGAL::Arr_segment_traits_2<Kernel>> vp_output;
                CGAL::Simple_polygon_visibility_2<CGAL::Arrangement_2<CGAL::Arr_segment_traits_2<Kernel>>, CGAL::Tag_false> non_regular_visibility(polygon_arr);

                auto preceding_halfedge = std::find_if(polygon_arr.halfedges_begin(), polygon_arr.halfedges_end(), [&vertex](const typename CGAL::Arrangement_2<CGAL::Arr_segment_traits_2<Kernel>>::Halfedge & e)
                {
                    return !e.face()->is_unbounded() && e.target()->point() == vertex;
                });
                assert(preceding_halfedge != polygon_arr.halfedges_end());
                non_regular_visibility.compute_visibility(vertex, preceding_halfedge, vp_output);


                auto current_it = vp_output.vertices_begin();
                CGAL::Point_2<Kernel> * xmin = &(current_it->point());
                CGAL::Point_2<Kernel> * xmax = &(current_it->point());
                CGAL::Point_2<Kernel> * ymin = &(current_it->point());
                CGAL::Point_2<Kernel> * ymax = &(current_it->point());

                for(current_it++; current_it != vp_output.vertices_end(); ++current_it)
                {
                    if (current_it->point().x() < xmin->x()) xmin = &(current_it->point());
                    if (current_it->point().x() > xmax->x()) xmax = &(current_it->point());
                    if (current_it->point().y() < ymin->y()) ymin = &(current_it->point());
                    if (current_it->point().y() > ymax->y()) ymax = &(current_it->point());
                }

                std::vector<CGAL::Point_2<Kernel>> candidates;

                for (auto &p : cell_centroids)
                {
                    if (p.x() >= xmin->x() && p.x() <= xmax->x() && p.y() >= ymin->y() && p.y() <= ymax->y() && utils::cgal::counterclockwise_in_between(vertex, p, floodlights[i][0].v1, floodlights[i][floodlights[i].size() - 1].v2))
                    {
                        candidates.push_back(p);
                    }
                }

                std::list<std::pair<CGAL::Point_2<Kernel>, typename CGAL::Arr_point_location_result<CGAL::Arrangement_2<CGAL::Arr_segment_traits_2<Kernel>>>::Type >> results;
                CGAL::locate(vp_output, candidates.begin(), candidates.end(), std::back_inserter(results));

                std::vector<CGAL::Point_2<Kernel>> filtered_result;

                for (auto it = results.begin(); it != results.end(); ++it)
                {
                    auto face = boost::get<typename CGAL::Arrangement_2<CGAL::Arr_segment_traits_2<Kernel>>::Face_const_handle>(&(it->second));
                    assert(face);

                    if (!((*face)->is_unbounded()))
                    {
                        filtered_result.push_back(it->first);
                    }
                }

                for (auto &point : filtered_result)
                {
                    int lb = 0;
                    int ub = floodlights[i].size() - 1;

                    CGAL::Vector_2<Kernel> v(vertex, point);

                    int pivot = floodlights[i].size() / 2 - 1;
                    while (lb < ub) {
                        assert(utils::cgal::counterclockwise_in_between(vertex, point, floodlights[i][lb].v1, floodlights[i][ub].v2));
                        if (utils::cgal::counterclockwise_in_between(v, floodlights[i][pivot].v2, floodlights[i][ub].v2)) {
                            lb = pivot + 1;
                        } else {
                            ub = pivot;
                        }
                        pivot = lb + ((ub - lb) / 2);
                    }
                    assert(utils::cgal::counterclockwise_in_between(vertex, point, floodlights[i][lb].v1, floodlights[i][ub].v2));

                    int c_index = cell_index[point];
                    visible_floodlights[c_index].push_back(std::make_pair(i, lb));
                    floodlights[i][lb].visible_cells.push_back(c_index);
                }
                ++pb;
            }
        }


        std::vector<Floodlight<Kernel>> merge_neighbored_floodlights(std::vector<Floodlight<Kernel>> &solution)
        {
            if (solution.empty())
                return solution;

            auto floodlight_sort = [](const Floodlight<Kernel> & lhs, const Floodlight<Kernel> & rhs) -> bool {
                return lhs.position < rhs.position || (lhs.position == rhs.position && lhs.v1.direction() < rhs.v1.direction());
            };

            std::sort(solution.begin(), solution.end(), floodlight_sort);

            std::vector<Floodlight<Kernel>> merged_solution;
            auto prev = solution.begin();
            auto it = prev + 1;
            int first_of_vertex = 0;
            auto merge_candidate = prev;

            do {
                if (it->position != prev->position)
                {
                    if (first_of_vertex < merged_solution.size() && merged_solution[first_of_vertex].v1.direction() == prev->v2.direction())
                    {
                        merged_solution[first_of_vertex].v1 = merge_candidate->v1;
                    } else {
                        merged_solution.emplace_back(prev->position, merge_candidate->v1, prev->v2, prev->vertex_index);
                    }
                    first_of_vertex = merged_solution.size();
                    merge_candidate = it;
                } else if (it->v1.direction() != prev->v2.direction())
                {
                    merged_solution.emplace_back(prev->position, merge_candidate->v1, prev->v2, prev->vertex_index);
                    merge_candidate = it;
                }
                ++prev;
            } while (++it != solution.end());

            if (first_of_vertex < merged_solution.size() && merged_solution[first_of_vertex].v1.direction() == prev->v2.direction())
            {
                merged_solution[first_of_vertex].v1 = merge_candidate->v1;
            } else {
                merged_solution.emplace_back(prev->position, merge_candidate->v1, prev->v2, prev->vertex_index);
            }

            return merged_solution;
        }
    };

}

#endif //ANGULARARTGALLERYPROBLEM_IP_APPROXIMATION_H
