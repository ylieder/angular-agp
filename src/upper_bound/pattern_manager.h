//
// Manages the different pattern instances.
//
// Created by Yannic Lieder on 03.08.20.
//

#ifndef ANGULAR_ART_GALLERY_PROBLEM_PATTERN_MANAGER_H
#define ANGULAR_ART_GALLERY_PROBLEM_PATTERN_MANAGER_H

#include <vector>

#include "patterns/base_pattern.h"


enum class Pattern {
    SMALL_TRIANGLE,
    RADIUS,
    DUCT,
    HISTOGRAM,
    NON_CONVEX_VERTEX,
    CONVEX_SUBPOLYGON,
    EDGE_EXTENSION,
};

class PatternManager {
public:
    static BasePattern* get(Pattern pattern);
    static std::vector<BasePattern*> get(std::vector<Pattern> const & patterns);
    static std::vector<BasePattern*> get_all();
};

#endif //ANGULAR_ART_GALLERY_PROBLEM_PATTERN_MANAGER_H
