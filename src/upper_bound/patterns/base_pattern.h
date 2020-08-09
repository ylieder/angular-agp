//
// Created by Yannic Lieder on 02.08.20.
//

#ifndef ANGULAR_ART_GALLERY_PROBLEM_BASE_PATTERN_H
#define ANGULAR_ART_GALLERY_PROBLEM_BASE_PATTERN_H

#include "kernel_definitions.h"


class Visualizer; // Forward declaration to avoid circular includes

class BasePattern {
public:
    BasePattern(int value) : value_(value) {}
    virtual bool split(Polygon const & polygon, std::stack<Polygon const> & remaining_polygons,
            Visualizer & visualizer) = 0;

    virtual bool combine_visualizations() const {
        return false;
    };

    int value() const {
        return value_;
    };

    virtual std::string description() const = 0;

    virtual ~BasePattern() {};
private:
    int value_;
};

#endif //ANGULAR_ART_GALLERY_PROBLEM_BASE_PATTERN_H
