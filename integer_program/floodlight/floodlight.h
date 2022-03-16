//
// Created by Yannic Lieder on 26.09.19.
//

#ifndef ANGULARARTGALLERYPROBLEM_FLOODLIGHT_H
#define ANGULARARTGALLERYPROBLEM_FLOODLIGHT_H

#include <CGAL/Arrangement_2.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Simple_polygon_visibility_2.h>
#include <CGAL/Arr_naive_point_location.h>

#include "utils/cgal_utils.h"

namespace AAGP
{
    template <typename Kernel>
    class Floodlight {
    public:
        CGAL::Point_2<Kernel> position;
        CGAL::Vector_2<Kernel> v1, v2;
        std::vector<int> visible_cells;
        size_t vertex_index;
        bool convex; // TODO: no support for non-convex floodlights yet

        Floodlight(const CGAL::Point_2<Kernel> & position, const CGAL::Vector_2<Kernel> &v1, const CGAL::Vector_2<Kernel> &v2, size_t vertex_index)
            : position(position), v1(v1), v2(v2), vertex_index(vertex_index)
        {
            convex = CGAL::left_turn(position, position + v1, position + v2);
        };

        double angle()
        {
            typename Kernel::FT dot_product = v1.x() * v2.x() + v1.y() * v2.y();
            double cosine = CGAL::to_double(dot_product) / (std::sqrt(CGAL::to_double(v1.squared_length())) * std::sqrt(CGAL::to_double(v2.squared_length())));
            double rad = std::acos(cosine);

            if (!convex)
            {
                rad = 2 * M_PI - rad;
            }

            return rad;
        }

        CGAL::Polygon_2<Kernel> visibility_polygon(const CGAL::Polygon_2<Kernel> &polygon) const
        {
            CGAL::Arrangement_2<CGAL::Arr_segment_traits_2<Kernel>> arr;
            CGAL::insert_non_intersecting_curves(arr, polygon.edges_begin(), polygon.edges_end());

            auto preceding_halfedge = std::find_if(arr.halfedges_begin(), arr.halfedges_end(), [this](const typename CGAL::Arrangement_2<CGAL::Arr_segment_traits_2<Kernel>>::Halfedge & e)
            {
                return !e.face()->is_unbounded() && e.target()->point() == position;
            });

            if (preceding_halfedge == arr.halfedges_end())
                throw std::runtime_error("error: cannot compute visibility polygon");

            auto succeeding_halfedge = preceding_halfedge->next();
            auto succ_direction = CGAL::Direction_2<Kernel>(CGAL::Segment_2<Kernel>(succeeding_halfedge->source()->point(), succeeding_halfedge->target()->point()));
            if (!(v1.direction() == succ_direction || succ_direction.counterclockwise_in_between(v1.direction(), v2.direction())))
            {
                try {
                    auto intersection = utils::cgal::primitive_polygon_ray_intersection(polygon, CGAL::Ray_2<Kernel>(position, position + v1));
                    CGAL::insert(arr, CGAL::Segment_2<Kernel>(position, intersection));
                    succeeding_halfedge = preceding_halfedge->next();
                } catch (std::runtime_error &e)
                {
                    assert(false);
                }
            }

            auto prec_direction = CGAL::Direction_2<Kernel>(CGAL::Segment_2<Kernel>(preceding_halfedge->target()->point(), preceding_halfedge->source()->point()));
            if(!(v2.direction() == prec_direction || prec_direction.counterclockwise_in_between(v1.direction(), v2.direction())))
            {
                assert(succeeding_halfedge->source()->point() == position);

                try {
                    auto intersection = utils::cgal::primitive_polygon_ray_intersection(polygon, CGAL::Ray_2<Kernel>(position, position + v2));
                    CGAL::insert(arr, CGAL::Segment_2<Kernel>(position, intersection));
                    preceding_halfedge = succeeding_halfedge->prev();
                } catch (std::runtime_error &e)
                {
                    assert(false);
                }
            }
;
            typedef CGAL::Simple_polygon_visibility_2<CGAL::Arrangement_2<CGAL::Arr_segment_traits_2<Kernel>>, CGAL::Tag_false> NSPV;
            CGAL::Arrangement_2<CGAL::Arr_segment_traits_2<Kernel>> non_regular_output;
            NSPV non_regular_visibility(arr);

            non_regular_visibility.compute_visibility(position, preceding_halfedge, non_regular_output);

            auto start_edge = std::find_if(non_regular_output.halfedges_begin(), non_regular_output.halfedges_end(), [](const typename CGAL::Arrangement_2<CGAL::Arr_segment_traits_2<Kernel>>::Halfedge &e) {
                return !e.face()->is_unbounded();
            });

            CGAL::Polygon_2<Kernel> vp;

            auto current_halfedge = start_edge;
            do
            {
                vp.push_back(current_halfedge->source()->point());
                current_halfedge = current_halfedge->next();
            } while (current_halfedge != start_edge);

            //vp.push_back(position);
            //vp.push_back(position + v1);
            //vp.push_back(position + v2);

            return vp;
        }
    };
}
#endif //ANGULARARTGALLERYPROBLEM_FLOODLIGHT_H
