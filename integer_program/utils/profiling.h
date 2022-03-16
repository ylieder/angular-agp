#ifndef PROFILING_H
#define PROFILING_H

#include <functional>
#include <iostream>
#include <chrono>
#include <type_traits>


template<class Tp>
struct is_duration : std::false_type
{ };

template<class Rep, class Period>
struct is_duration<typename std::chrono::duration<Rep, Period>> : std::true_type
{ };


template<typename ToDur, typename TFunc>
typename std::enable_if<is_duration<ToDur>::value, ToDur>::type
measure_time(const TFunc& code) {

    auto start = std::chrono::system_clock::now();

    // Run code
    code();

    auto end = std::chrono::system_clock::now();

    auto time = std::chrono::duration_cast<ToDur>(end-start);

    return time;
}


#endif //PROFILING_H
