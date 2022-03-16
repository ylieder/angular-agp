//
// Created by Yannic Lieder on 02.10.19.
//

#ifndef ANGULARARTGALLERYPROBLEM_SVG_FLOODLIGHT_H
#define ANGULARARTGALLERYPROBLEM_SVG_FLOODLIGHT_H

#include <boost/filesystem.hpp>

#include "floodlight.h"
#include "simple_svg/simple_svg_cgal_extension.h"

#include "utils/random_utils.hpp"
#include "utils/conversion_utils.h"

namespace fs = boost::filesystem;

namespace AAGP
{
    template <typename Kernel>
    void svg_floodlight_placement(const fs::path &filename, const std::vector<Floodlight<Kernel>> & floodlights, const CGAL::Polygon_2<Kernel> & polygon, double opacity = 0.2)
    {
        if (!fs::exists(filename.parent_path()))
            fs::create_directories(filename.parent_path());

        svg::Document doc(filename.c_str());
        std::map<CGAL::Point_2<Kernel>, svg::Color> color_mapping;

        std::set<CGAL::Point_2<Kernel>> floodlight_vertices;
        for (auto &f : floodlights)
            floodlight_vertices.insert(f.position);
        int n = floodlight_vertices.size();

        std::vector<int> color_order(n);
        std::iota(color_order.begin(), color_order.end(), 0);
        std::shuffle(color_order.begin(), color_order.end(), std::random_device());

        double hue_interval = 1.0 / n;
        int i = 0;
        for (auto &f : floodlights)
        {
            if (color_mapping.find(f.position) == color_mapping.end())
            {
                auto color = utils::conversion::hsv_to_rgb(color_order[i++] * hue_interval, 1, 0.9);
                color_mapping.insert(std::make_pair(f.position, svg::Color(color.first, color.second, color.third)));
            }

            auto vp = f.visibility_polygon(polygon);
            doc << svg::Polygon_(vp, svg::Fill(color_mapping.at(f.position), opacity), svg::Stroke());
        }

        double radius = 5;
        for (auto &f : floodlights)
        {
            svg::Fill fill(color_mapping.at(f.position));

            double start_angle = utils::cgal::angle(f.v1);
            double end_angle = utils::cgal::angle(f.v2);
            doc << svg::PartialCircle(svg::Point_(f.position), radius, start_angle, end_angle, !f.convex, fill);
        }

        doc << svg::Polygon_(polygon, svg::Fill(), svg::Stroke(1, svg::Color::Black, true));

        /*
        for (auto &f : floodlights)
        {
            svg::Fill fill(color_mapping.at(f.position));
            doc << svg::Circle(svg::Point_(f.position), radius / 5, fill);
        }
        */

        doc.save();
    }
}

#endif //ANGULARARTGALLERYPROBLEM_SVG_FLOODLIGHT_H
