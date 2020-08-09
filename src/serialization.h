//
// Read and write AAGP instance files.
//
// Pitfall: Polygon instances cannot be written with exact coordinates due to the missing CGAL::exact method in
// kernel with sqrt. Maybe, instances could be cast to Epeck kernel withotu sqrt first, since we know that no square
// roots are in the polygon vertex coordinates.
//
// Created by Yannic Lieder on 03.10.19.
//

#ifndef ANGULARARTGALLERYPROBLEM_SERIALIZATION_H
#define ANGULARARTGALLERYPROBLEM_SERIALIZATION_H

#include <filesystem>
#include <fstream>
#include <regex>

namespace fs = std::filesystem;

namespace serialization {
    template<class Kernel>
    CGAL::Polygon_2<Kernel> read_file(fs::path const & path) {
        if (!fs::is_regular_file(path)) {
            throw std::invalid_argument("Not a file: " + path.string());
        }

        std::ifstream stream(path);
        CGAL::Polygon_2<Kernel> polygon;

        int size;
        stream >> size;

        std::string x_str, y_str;
        while (stream >> x_str >> y_str) {
            polygon.push_back(CGAL::Point_2<Kernel>(typename Kernel::FT(x_str), typename Kernel::FT(y_str)));
        }

        assert(polygon.size() == size);
        return polygon;
    }

    /**
     * Output maybe differs from original values, since exact output of sqrt-kernel is not possible.
     */
    template <typename Kernel>
    bool write_file(CGAL::Polygon_2<Kernel> const & polygon, fs::path const & directory, std::string const & filename) {
        fs::create_directories(directory);
        fs::path file = directory / (filename + ".pol");
        std::ofstream stream{file};

        if (!stream.good()) {
            return false;
        }

        stream << polygon.size() << std::endl;
        for (auto vit = polygon.vertices_begin(); vit != polygon.vertices_end(); ++vit) {
            stream << vit->x() << " " << vit->y() << std::endl;
        }
        stream.close();
        return true;
    }
}

#endif //ANGULARARTGALLERYPROBLEM_SERIALIZATION_H
