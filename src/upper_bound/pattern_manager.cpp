//
// Created by Yannic Lieder on 08.08.20.
//

#include "pattern_manager.h"

#include "upper_bound/patterns/convex_subpolygon_pattern.h"
#include "upper_bound/patterns/duct_pattern.h"
#include "upper_bound/patterns/edge_extension_pattern.h"
#include "upper_bound/patterns/histogram_pattern.h"
#include "upper_bound/patterns/non_convex_vertex_pattern.h"
#include "upper_bound/patterns/radius_pattern.h"
#include "upper_bound/patterns/small_triangle_pattern.h"

static ConvexSubpolygonPattern convex_subpolygon_pattern((int)Pattern::CONVEX_SUBPOLYGON);
static DuctPattern duct_pattern((int)Pattern::DUCT);
static EdgeExtensionPattern edge_extension_pattern((int)Pattern::EDGE_EXTENSION);
static HistogramPattern histogram_pattern((int)Pattern::HISTOGRAM);
static NonConvexVertexPattern non_convex_vertex_pattern((int)Pattern::NON_CONVEX_VERTEX);
static RadiusPattern radius_pattern((int)Pattern::RADIUS);
static SmallTrianglePattern small_triangle_pattern((int)Pattern::SMALL_TRIANGLE);

BasePattern * PatternManager::get(Pattern pattern) {
    switch (pattern) {
        case Pattern::CONVEX_SUBPOLYGON:
            return &convex_subpolygon_pattern;
        case Pattern::DUCT:
            return &duct_pattern;
        case Pattern::EDGE_EXTENSION:
            return &edge_extension_pattern;
        case Pattern::HISTOGRAM:
            return &histogram_pattern;
        case Pattern::NON_CONVEX_VERTEX:
            return &non_convex_vertex_pattern;
        case Pattern::RADIUS:
            return &radius_pattern;
        case Pattern::SMALL_TRIANGLE:
            return &small_triangle_pattern;
    }
}

std::vector<BasePattern*> PatternManager::get(std::vector<Pattern> const & patterns) {
    std::vector<BasePattern*> pattern_vec;
    for (Pattern const & pattern : patterns) {
        pattern_vec.push_back(PatternManager::get(pattern));
    }
    return pattern_vec;
}

std::vector<BasePattern*> PatternManager::get_all() {
    return std::vector<BasePattern*> {
        &small_triangle_pattern,
        &radius_pattern,
        &duct_pattern,
        &histogram_pattern,
        &non_convex_vertex_pattern,
        &convex_subpolygon_pattern,
        &edge_extension_pattern,
    };
}