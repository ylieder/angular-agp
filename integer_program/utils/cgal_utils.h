#ifndef ANGULARARTGALLERYPROBLEM_CGAL_UTILS_H
#define ANGULARARTGALLERYPROBLEM_CGAL_UTILS_H

#include <CGAL/Triangulation_2.h>
#include <CGAL/Polygon_with_holes_2.h>

namespace utils
{
    namespace cgal
    {
        template <typename Kernel>
        struct PolarAngleLess
        {
            PolarAngleLess(const CGAL::Point_2<Kernel> &center, const CGAL::Point_2<Kernel> &start)
                : center(&center), start(&start) { }

            bool operator() (const CGAL::Point_2<Kernel> & lhs, const CGAL::Point_2<Kernel> & rhs)
            {
                auto orientation_lhs = CGAL::orientation(*center, *start, lhs);
                auto orientation_rhs = CGAL::orientation(*center, *start, rhs);

                if (orientation_lhs >= 0 && orientation_rhs < 0)
                    return true;

                if (orientation_lhs < 0 && orientation_rhs >= 0)
                    return false;

                if (orientation_rhs == 0 && orientation_lhs == 0)
                    return CGAL::squared_distance(*center, lhs) < CGAL::squared_distance(*center, rhs);

                auto orientation = CGAL::orientation(*center, lhs, rhs);

                if (orientation > 0)
                    return true;

                if (orientation < 0)
                    return false;

                return CGAL::squared_distance(*center, lhs) < CGAL::squared_distance(*center, rhs);
            }
        private:
            const CGAL::Point_2<Kernel> * center;
            const CGAL::Point_2<Kernel> * start;
        };

        template <typename Kernel>
        static int quadrant(const CGAL::Vector_2<Kernel> & v)
        {
            if (v.x() > 0 && v.y() >= 0)
                return 0;

            if (v.x() <= 0 && v.y() > 0)
                return 1;

            if (v.x() < 0 && v.y() <= 0)
                return 2;

            if (v.x() >= 0 && v.y() < 0)
                return 3;

            assert(false);
            return -1;
        }

        template <typename Kernel>
        static typename Kernel::FT pwh_area(const CGAL::Polygon_with_holes_2<Kernel> & pwh)
        {
           typename Kernel::FT hole_area(0);

            for (auto hit = pwh.holes_begin(); hit != pwh.holes_end(); ++hit)
            {
                hole_area += hit->area();
            }

            return pwh.outer_boundary().area() - hole_area;
        }

        template <typename Kernel>
        bool counterclockwise_in_between(const CGAL::Vector_2<Kernel> &v, const CGAL::Vector_2<Kernel> &v1, const CGAL::Vector_2<Kernel> &v2)
        {
            return v.direction().counterclockwise_in_between(v1.direction(), v2.direction());
        }

        template <typename Kernel>
        bool counterclockwise_in_between(const CGAL::Point_2<Kernel> &origin, const CGAL::Point_2<Kernel> &p, const CGAL::Vector_2<Kernel> &v1, const CGAL::Vector_2<Kernel> &v2)
        {
            CGAL::Vector_2<Kernel> v(origin, p);
            return counterclockwise_in_between(v, v1, v2);
        }

        template <typename Kernel>
        static CGAL::Point_2<Kernel> centroid(const typename CGAL::Arrangement_2<CGAL::Arr_segment_traits_2<Kernel>>::Face_const_handle face)
        {
            CGAL::Triangulation_2<Kernel> triangulation;

            auto begin = face->outer_ccb();
            auto current_he = begin;
            do {
                triangulation.insert(current_he->source()->point());
            } while (++current_he != begin);

            typename Kernel::FT total_weight(0);
            CGAL::Vector_2<Kernel> partial_centroid(0, 0);

            for (auto it = triangulation.finite_faces_begin(); it != triangulation.finite_faces_end(); ++it)
            {
                auto triangle = triangulation.triangle(it);
                typename Kernel::FT area = triangle.area();
                auto centroid = CGAL::centroid(triangle);
                CGAL::Vector_2<Kernel> centroid_vector = CGAL::Vector_2<Kernel>(centroid.x(), centroid.y());
                total_weight += area;
                partial_centroid += area * centroid_vector;
            }

            partial_centroid *= (1 / total_weight);
            return CGAL::Point_2<Kernel>(partial_centroid.x(), partial_centroid.y());
        }

        template <typename Kernel>
        static CGAL::Vector_2<Kernel> normalize_vector(const CGAL::Vector_2<Kernel> & v)
        {
            auto length = std::sqrt(CGAL::to_double(v.squared_length())); // TODO: inexact
            return v / length;
        }

        template <typename Kernel>
        static CGAL::Vector_2<Kernel> rotate_vector(const CGAL::Vector_2<Kernel> & v, double angle_rad)
        {
            //              / cos θ  -sin θ \
            // v_rotated = |                | * v
            //              \ sin θ   cos θ /

            float s = sin(angle_rad);
            float c = cos(angle_rad);

            auto x = v.x() * c - v.y() * s;
            auto y = v.x() * s + v.y() * c;


            return CGAL::Vector_2<Kernel>(x, y);
        }
        template <typename Kernel>
        static CGAL::Point_2<Kernel> rotate_point(const CGAL::Point_2<Kernel> & a, const CGAL::Point_2<Kernel> & b, double angle_rad)
        {
            //              / cos θ  -sin θ \
            // a_rotated = |                | * (a - b) + b
            //              \ sin θ   cos θ /

            double x = CGAL::to_double(a.x()) - CGAL::to_double(b.x());
            double y = CGAL::to_double(a.y()) - CGAL::to_double(b.y());

            float s = sin(angle_rad);
            float c = cos(angle_rad);

            double x_new = x * c - y * s;
            double y_new = x * s + y * c;

            x_new += CGAL::to_double(b.x());
            y_new += CGAL::to_double(b.y());

            return CGAL::Point_2<Kernel>(x_new, y_new);
        }

        template <typename Kernel>
        static CGAL::Point_2<Kernel> primitive_polygon_ray_intersection(const CGAL::Polygon_2<Kernel> & polygon, const CGAL::Ray_2<Kernel> & ray)
        {
            typename Kernel::FT t(0);
            CGAL::Point_2<Kernel> intersection_point;
            bool found_intersection = false;

            for (auto e = polygon.edges_begin(); e != polygon.edges_end(); ++e)
            {
                auto intersection = CGAL::intersection(ray, *e);

                if (intersection)
                {
                    if (const CGAL::Segment_2<Kernel>* s = boost::get<CGAL::Segment_2<Kernel>>(&*intersection)) {
                        auto sqrt_distance_1 = CGAL::squared_distance(s->source(), ray.source());
                        auto sqrt_distance_2 = CGAL::squared_distance(s->target(), ray.source());
                        auto sqrt_distance = CGAL::min(sqrt_distance_1, sqrt_distance_2);
                        CGAL::Point_2<Kernel> p = sqrt_distance_1 < sqrt_distance_2 ? s->source() : s->target();

                        if (ray.source() != p && (!found_intersection || sqrt_distance < t))
                        {
                            intersection_point = p;
                            t = sqrt_distance;
                            found_intersection = true;
                        }
                        // TODO: evt. keine intersection!!

                        //throw std::runtime_error("degenerated case in primitive_polygon_intersection"); // TODO: return nearest point or farest?
                    } else if (const CGAL::Point_2<Kernel>* p = boost::get<CGAL::Point_2<Kernel>>(&*intersection)) {
                        //auto next = (e + 1) == polygon.edges_end() ? polygon.edges_begin() : (e + 1);
                        //if ((*p == e->source() || *p == e->target()) && (CGAL::orientation()))

                        // TODO: evt. keine intersection

                        auto sqrt_distance = CGAL::squared_distance(*p, ray.source());

                        if (ray.source() != *p && (!found_intersection || sqrt_distance < t))
                        {
                            intersection_point = *p;
                            t = sqrt_distance;
                            found_intersection = true;
                        }
                    } else {
                        throw std::logic_error("Error: cannot compute polygon ray intersection");
                        assert(false);
                    }
                }
            }

            if (!found_intersection)
            {
                throw std::logic_error("ray does not intersect polygon");
            }

            return intersection_point;
        }

        template <typename Kernel>
        static bool primitive_polygon_segment_intersection(const CGAL::Polygon_2<Kernel> & polygon, const CGAL::Segment_2<Kernel> & segment, const double epsilon = 10e-10)
        {
            for (auto e = polygon.edges_begin(); e != polygon.edges_end(); ++e)
            {
                auto intersection = CGAL::intersection(segment, *e);

                if (intersection)
                {
                    if (const CGAL::Segment_2<Kernel>* s = boost::get<CGAL::Segment_2<Kernel> >(&*intersection)) {
                        throw std::runtime_error("degenerated case in primitive_polygon_intersection");
                        assert(false);
                    } else {
                        const CGAL::Point_2<Kernel>* p = boost::get<CGAL::Point_2<Kernel> >(&*intersection);

                        auto sqrt_distance_1 = CGAL::squared_distance(*p, segment.source());
                        auto sqrt_distance_2 = CGAL::squared_distance(*p, segment.target());

                        if (sqrt_distance_1 > epsilon && sqrt_distance_2 > epsilon) // TODO: handle epsilon errors
                        {                                                         // Problem: with Epick, *p != segment.source()
                            return true;                                          // is always true
                        }
                    }
                }
            }

            return false;
        }

        template <typename Kernel>
        double angle(const CGAL::Vector_2<Kernel> &v)
        {
            long double a = atan2l((long double) CGAL::to_double(v.y()), (long double) CGAL::to_double(v.x()));

            if (a < 0)
                a = 2 * M_PI + a;

            return a;
        }

        template <typename Kernel>
        double angle(const CGAL::Vector_2<Kernel> &v1, const CGAL::Vector_2<Kernel> &v2)
        {
            double a = angle(v1);
            double b = angle(v2);

            if (b >= a)
            {
                return b - a;
            } else {
                return 360 + b - a;
            }
        }

        template <typename Kernel>
        double cosine_angle(const typename CGAL::Point_2<Kernel> & p1, const typename CGAL::Point_2<Kernel> & p2, const typename CGAL::Point_2<Kernel> & p3) {
            CGAL::Vector_2<Kernel> v1(p2, p1);
            CGAL::Vector_2<Kernel> v2(p2, p3);

            typename Kernel::FT dot_product = v1.x() * v2.x() + v1.y() * v2.y();
            double cosine_angle = CGAL::to_double(dot_product) / (std::sqrt(CGAL::to_double(v1.squared_length())) * std::sqrt(CGAL::to_double(v2.squared_length())));

            return cosine_angle;
        }

        template <typename Kernel>
        typename Kernel::FT cosine_angle(const typename CGAL::Polygon_2<Kernel>::Vertex_circulator & ci) {
            return cosine_angle(*(ci - 1), *ci, *(ci + 1));
        }

        template <typename Kernel>
        bool is_convex(const typename CGAL::Polygon_2<Kernel>::Vertex_circulator & ci, bool counterclockwise_orientation = true) {
            if (counterclockwise_orientation)
            {
                return CGAL::left_turn(*ci, *(ci + 1), *(ci - 1));
            } else {
                return CGAL::left_turn(*ci, *(ci - 1), *(ci + 1));
            }
        }
    }
}

#endif //ANGULARARTGALLERYPROBLEM_CGAL_UTILS_H
