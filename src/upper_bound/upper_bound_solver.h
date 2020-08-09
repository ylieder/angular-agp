//
// Main AAGP solver algorithm.
//
// Created by Yannic Lieder on 02.08.20.
//

#ifndef ANGULAR_ART_GALLERY_PROBLEM_ALGORITHM_H
#define ANGULAR_ART_GALLERY_PROBLEM_ALGORITHM_H

#include "cgal_helpers/polygon_normalization.h"
#include "kernel_definitions.h"
#include "pattern_manager.h"
#include "visualizer.h"


class UpperBoundSolver {
public:
    explicit UpperBoundSolver(Polygon const & polygon, std::vector<Pattern> const & patterns)
            : patterns(PatternManager::get(patterns)), visualizer(polygon) {
        initialize(polygon);
    };

    void set_visualize(bool value) {
        visualizer.set_visualize(value);
    }

    void set_output(std::string const & base_dir, std::string const & filename, std::string const & rel_dir = "") {
        output.base_dir = base_dir;
        output.filename = filename;
        output.rel_dir = rel_dir;
        visualizer.set_output(base_dir, filename, rel_dir);
    }

    /**
     * \pre Polygon is simple
     * \pre Polygon has at least three vertices
     */
    std::pair<bool, Polygon> solve() {
        visualizer.draw_initial_polygon();

        while (!remaining_polygons.empty()) {
            pattern_idx = 0; // reset patterns, start with first one
            auto top = remaining_polygons.top();
            assert(top.is_simple() && top.size() > 2);

#ifdef DEBUG_LOG
                std::cout << "Current polygon: " << top << std::endl;
#endif

            if (base_case(top)) {
#ifdef DEBUG_LOG
                std::cout << "Base case" << std::endl;
#endif

                remaining_polygons.pop();
                visualizer.draw_base_case(top);
                continue;
            }

            bool success = false;
            while (pattern_idx < patterns.size()) {
                BasePattern* pattern = patterns[pattern_idx++];

#ifdef DEBUG_LOG
                std::cout << "Test " << pattern->description() << std::endl;
#endif

                if (pattern->split(top, remaining_polygons, visualizer)) {
                    success = true;
                    break;
                }
            }

            if (!success) {
                visualizer.draw_unsolved_polygon(top);
                visualizer.close();
                return std::make_pair(false, top);
            }
        }

        visualizer.close();
        return std::make_pair(true, Polygon());
    }

private:
    std::stack<Polygon const> remaining_polygons;
    std::vector<BasePattern*> patterns;
    int pattern_idx = 0;
    Visualizer visualizer;

    struct {
        std::string base_dir;
        std::string rel_dir;
        std::string filename;
    } output;

    static bool base_case(Polygon const & polygon) {
        return polygon.is_convex() || polygon.size() < 6;
    }

    void initialize(Polygon const & polygon) {
        if (!polygon.is_counterclockwise_oriented()) {
            throw std::runtime_error("Polygon needs to be counterclockwise oriented");
        }

        if (!is_normalized(polygon)) {
            throw std::runtime_error("Polygon is not normalized");
        }
        remaining_polygons.push(polygon);
    }
};

#endif //ANGULAR_ART_GALLERY_PROBLEM_ALGORITHM_H
