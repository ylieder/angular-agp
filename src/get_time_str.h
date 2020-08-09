//
// Defines the funtion get_time_str, returning the current time in a human-readable format as string.
//
// Created by Yannic Lieder on 06.08.20.
//

#ifndef ANGULAR_ART_GALLERY_PROBLEM_GET_TIME_STR_H
#define ANGULAR_ART_GALLERY_PROBLEM_GET_TIME_STR_H

std::string get_time_str(std::string const & format = "%Y-%m-%d-%H%M%S") {
    auto time = std::time(nullptr);
    auto tm = *std::localtime(&time);

    std::ostringstream oss;
    oss << std::put_time(&tm, format.c_str());
    return oss.str();
}

#endif //ANGULAR_ART_GALLERY_PROBLEM_GET_TIME_STR_H
