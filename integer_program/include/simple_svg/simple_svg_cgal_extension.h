/******************************************************************************
 Edited by Yannic Lieder, 30.09.2019
 Local changes:
 * add support for CGAL object as described in simple_svg.hpp description
 * classes "<Shape>_" extend classes "<Shape>" by CGAL object constructor
*******************************************************************************/

#ifndef ANGULARARTGALLERYPROBLEM_SIMPLE_SVG_CGAL_EXTENSION_H
#define ANGULARARTGALLERYPROBLEM_SIMPLE_SVG_CGAL_EXTENSION_H

#include "simple_svg_1.0.0.hpp"
#include <CGAL/Polygon_2.h>

namespace svg
{
    class Point_ : public Point
    {
    public:
        template <typename Kernel>
        Point_(const CGAL::Point_2<Kernel> &point)
        {
            x = CGAL::to_double(point.x());
            y = CGAL::to_double(point.y());
        }
        Point_(double x, double y) : Point(x, y) { }
    };

    class Line_ : public Line
    {
    public:
        template <typename Kernel>
        Line_(const CGAL::Segment_2<Kernel> &segment, const Stroke & stroke = Stroke())
            : Line(svg::Point_(segment.source()), svg::Point_(segment.target()), stroke) { }
    };

    class Polygon_ : public Polygon
    {
    public:
        template <typename Kernel>
        Polygon_(const CGAL::Polygon_2<Kernel> &polygon, Fill const & fill = Fill(), Stroke const & stroke = Stroke(0.1, Color::Black))
                : Polygon(fill, stroke)
        {
            for (auto vit = polygon.vertices_begin(); vit != polygon.vertices_end(); ++vit)
                add(Point_(*vit));
        }
        Polygon_(Fill const & fill = Fill(), Stroke const & stroke = Stroke())
        : Polygon(fill, stroke) { }
        Polygon_(Stroke const & stroke = Stroke()) : Polygon(stroke) { }
        void add(Point const & point)
        {
            points.push_back(point);
            _bbox.extend(point);
        }
        Polygon_ & operator<<(Point const & point)
        {
            add(point);
            return *this;
        }
    };

    class Arrangement_ : public Shape
    {
    public:
        template <typename Kernel>
        Arrangement_(const CGAL::Arrangement_2<CGAL::Arr_segment_traits_2<Kernel>> & arrangement, Fill const & fill = Fill(), Stroke const & stroke = Stroke(0.1, Color::Black, true))
            : Shape(fill, stroke)
        {
                for (auto eit = arrangement.edges_begin(); eit != arrangement.edges_end(); ++eit)
                {
                    _edges.push_back(std::make_pair(Point_(eit->source()->point()), Point_(eit->target()->point())));
                }

                for (auto vit = arrangement.vertices_begin(); vit != arrangement.vertices_end(); ++vit)
                {
                    _vertices.push_back(Point_(vit->point()));
                    _bbox.extend(_vertices.back());
                }
        }

        std::string toString(Layout const & layout) const
        {
            std::stringstream ss;

            for (auto & e : _edges)
            {
                ss << svg::Line(e.first, e.second, stroke).toString(layout);
            }

            for (auto & v : _vertices)
            {
                ss << svg::Circle(v, 0.05, Fill(svg::Color::Black)).nonScalingSize().toString(layout); // todo: scale
            }

            return ss.str();
        }

        void offset(Point const & offset)
        {
            for (unsigned i = 0; i < _vertices.size(); ++i) {
                _vertices[i].x += offset.x;
                _vertices[i].y += offset.y;
            }
        }
    private:
        std::vector<std::pair<svg::Point, svg::Point>> _edges;
        std::vector<svg::Point> _vertices;
    };
}

#endif //ANGULARARTGALLERYPROBLEM_SIMPLE_SVG_CGAL_EXTENSION_H
