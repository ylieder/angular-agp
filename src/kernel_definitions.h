//
// Defines the used CGAL kernel and introduces typdefs for different CGAL objects.
//
// Created by Yannic Lieder on 02.08.20.
//

#ifndef ANGULAR_ART_GALLERY_PROBLEM_KERNEL_DEFINITIONS_H
#define ANGULAR_ART_GALLERY_PROBLEM_KERNEL_DEFINITIONS_H

#include "CGAL/Exact_predicates_exact_constructions_kernel_with_sqrt.h"
#include "CGAL/Polygon_2.h"

using Kernel = CGAL::Exact_predicates_exact_constructions_kernel_with_sqrt;

using Circle = CGAL::Circle_2<Kernel>;
using FT = Kernel::FT;
using Point = CGAL::Point_2<Kernel>;
using Polygon = CGAL::Polygon_2<Kernel>;
using Ray = CGAL::Ray_2<Kernel>;
using Segment = CGAL::Segment_2<Kernel>;
using Triangle = CGAL::Triangle_2<Kernel>;
using Vector = CGAL::Vector_2<Kernel>;

#endif //ANGULAR_ART_GALLERY_PROBLEM_KERNEL_DEFINITIONS_H
