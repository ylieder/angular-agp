//
// Created by Yannic Lieder on 03.08.20.
//

#ifndef ANGULAR_ART_GALLERY_PROBLEM_VISUALIZER_H
#define ANGULAR_ART_GALLERY_PROBLEM_VISUALIZER_H

#include <filesystem>

#include <simple-svg/simple_svg_1.0.0.hpp>
#include <upper_bound/patterns/base_pattern.h>

#include "patterns/base_pattern.h"
#include "kernel_definitions.h"

class Visualizer {
public:
    explicit Visualizer(Polygon const & polygon, bool unsolved_polygon = false)
        : initial_polygon_ptr(&polygon), bbox(polygon.bbox()), unsolved_polygon(unsolved_polygon) {
        initialize_doc();
    }

    void set_visualize(bool value) {
        visualize = value;
    }

    void set_output(std::string const & base_dir, std::string const & filename, std::string const & rel_dir) {
        output.base_dir = base_dir;
        output.filename = filename;
        output.rel_dir = rel_dir;
        initialize_doc();
    }

    void draw_initial_polygon() {
        if (visualize) {
            if (last_pattern != UNKNOWN_PATTERN) {
                save();
            }
            add_cts_polygon(*initial_polygon_ptr);
            add_text("Initial polygon");
            save();
        }
    }

    void draw_unsolved_polygon(Polygon const & polygon) {
        if (unsolved_polygon) {
            add_cts_polygon(polygon);
            add_text("Unsolved");
            save();
        } else {
            if (last_pattern != UNKNOWN_PATTERN) {
                save();
            }
            Visualizer unsolved_visualizer(polygon, true);
            unsolved_visualizer.set_output(output.base_dir, output.filename, output.rel_dir);
            unsolved_visualizer.draw_unsolved_polygon(polygon);
        }
    }

    void draw_base_case(Polygon const & polygon) {
        if (visualize) {
            if (last_pattern != UNKNOWN_PATTERN) {
                save();
            }
            doc << cts_polygon(*initial_polygon_ptr, svg::Stroke(1, svg::Color::Silver));
            add_covered_areas();
            add_cts_polygon(polygon);
            add_text("Base case");
            save();

            extend_covered_areas(polygon);
        }
    }

    void split_step(
            Polygon const & polygon,
            Polygon * covered_polygon,
            Segment const * split_segment,
            Segment const * split_segment_2,
            BasePattern const * pattern) {
        if (visualize) {
            int lp = last_pattern; // keep local copy of last pattern, since  last_pattern is overriden in changed
                                      // method.
            if (lp != UNKNOWN_PATTERN && pattern->value() != lp) {
                add_cts_polygon(current_polygon);
                save();
            }

            if (lp == UNKNOWN_PATTERN || pattern->value() != lp) {
                doc << cts_polygon(*initial_polygon_ptr, svg::Stroke(1, svg::Color::Silver));
                current_polygon = polygon;
                add_covered_areas();
                add_text(pattern->description());
            }

            if (covered_polygon) { extend_covered_areas(*covered_polygon); }
            add_split_segment(*split_segment);
            if (split_segment_2) { add_split_segment(*split_segment_2); }

            if (pattern->combine_visualizations()) {
                last_pattern = pattern->value();
            } else {
                add_cts_polygon(current_polygon);
                save();
                last_pattern = UNKNOWN_PATTERN;
            }
        }
    }

    void close() {
        if (last_pattern != UNKNOWN_PATTERN) {
            save();
        }
    }
private:
    const static int SVG_DIMENSIONS = 500;
    const static int SVG_MARGIN = 10;
    const static int TEXT_OFFSET = 20;

    bool visualize = false;
    bool unsolved_polygon = false;

    const int UNKNOWN_PATTERN = -1;
    int last_pattern = UNKNOWN_PATTERN;

    Polygon const * initial_polygon_ptr = nullptr;
    Polygon current_polygon;
    CGAL::Bbox_2 bbox;

    struct {
        std::string base_dir;
        std::string rel_dir;
        std::string filename;
    } output;
    int counter = 0;
    svg::Document doc = svg::Document("");
    double scale = 1;
    struct {
        double x = 0;
        double y = 0;
    } offset;

    std::vector<svg::Polygon> covered_areas;

    std::string output_directory() const {
        if (unsolved_polygon) {
            return output.base_dir + "/unsolved";
        } else {
            return output.base_dir + "/vis/" + output.rel_dir + "/" + output.filename;
        }
    }

    std::string output_path() const {
        std::string filename;
        if (unsolved_polygon) {
            filename += output.rel_dir + "_" + output.filename + ".svg";
            std::replace(filename.begin(), filename.end(), '/', '_');
            std::replace(filename.begin(), filename.end(), '\\', '_');
        } else {
            filename += output.filename + "_" + std::to_string(counter) + ".svg";
        }

        return output_directory() + "/" + filename;
    }

    void add_text(std::string const & text) {
        doc << svg::Text(svg::Point(5, 5), text, svg::Fill(svg::Color::Black));
    }

    void add_split_segment(Segment const & segment) {
        doc << cts_segment(segment, svg::Stroke(1, svg::Color::Red));
    }

    void add_covered_areas() {
        for (auto const & area: covered_areas) {
            doc << area;
        }
    }

    void extend_covered_areas(Polygon const & polygon) {
        covered_areas.push_back(cts_polygon(polygon, svg::Stroke(1, svg::Color::Silver), svg::Fill(svg::Color::Silver)));
    }

    void initialize_doc() {
        auto canvas_size = SVG_DIMENSIONS - 2 * SVG_MARGIN;
        auto x_scale = canvas_size / (bbox.xmax() - bbox.xmin());
        auto y_scale = canvas_size / (bbox.ymax() - bbox.ymin());

        scale = std::min(y_scale, x_scale);
        offset.x = SVG_MARGIN - scale * bbox.xmin();
        offset.y = SVG_MARGIN + TEXT_OFFSET - scale * bbox.ymin();

        svg::Dimensions dimensions(SVG_DIMENSIONS, SVG_DIMENSIONS + TEXT_OFFSET);
        doc = svg::Document(output_path(), svg::Layout(dimensions, svg::Layout::BottomLeft));
    }

    double transformX(double value) const {
        return value * scale + offset.x;
    }

    double transformY(double value) const {
        return value * scale + offset.y;
    }

    svg::Point cts_point(Point const & point) {
        return svg::Point(transformX(CGAL::to_double(point.x())), transformY(CGAL::to_double(point.y())));
    }

    svg::Polygon cts_polygon(
            Polygon const & polygon,
            svg::Stroke const & stroke = svg::Stroke(1, svg::Color::Black),
            svg::Fill const & fill = svg::Fill()
    ) {
        svg::Polygon svg_polygon(fill, stroke);
        for (const auto & i : polygon) {
            svg_polygon << cts_point(i);
        }
        return svg_polygon;
    }

    void add_cts_polygon(
            Polygon const & polygon,
            svg::Stroke const & stroke = svg::Stroke(1, svg::Color::Black),
            svg::Fill const & fill = svg::Fill(),
            bool highlight_vertices = true
    ) {
        doc << cts_polygon(polygon, stroke, fill);

        if (highlight_vertices) {
            const float MARKER_SIZE = 3;
            svg::Fill marker_fill(svg::Color::Black);
            for (size_t i = 0; i < polygon.size(); ++i) {
                if (i == 0) {
                    // marker of first vertex is squared
                    svg::Point marker = cts_point(polygon[i]);
                    marker = svg::Point(marker.x - MARKER_SIZE / 2, marker.y + MARKER_SIZE / 2);
                    doc << svg::Rectangle(marker, MARKER_SIZE,
                            MARKER_SIZE, marker_fill);
                } else {
                    // all other markers are round
                    doc << svg::Circle(cts_point(polygon[i]), MARKER_SIZE, marker_fill);
                }
            }
        }
    }

    svg::Line cts_segment(Segment const & segment, svg::Stroke const & stroke = svg::Stroke(1, svg::Color::Black)) {
        return svg::Line(cts_point(segment.source()), cts_point(segment.target()), stroke);
    }

    void save() {
        std::filesystem::create_directories(output_directory());

        if(!doc.save()) {
            throw std::runtime_error("Couldn't write file " + output_path());
        };
        ++counter;
        initialize_doc();
        last_pattern = UNKNOWN_PATTERN;
    }
};

#endif //ANGULAR_ART_GALLERY_PROBLEM_VISUALIZER_H
