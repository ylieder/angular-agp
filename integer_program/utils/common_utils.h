//
// Created by Yannic Lieder on 26.09.19.
//

#ifndef ANGULARARTGALLERYPROBLEM_COMMON_UTILS_H
#define ANGULARARTGALLERYPROBLEM_COMMON_UTILS_H

namespace utils
{
    namespace common
    {
        template <typename T1, typename T2, typename T3>
        struct Triple
        {
            T1 first;
            T2 second;
            T3 third;
        };

        template <typename T1, typename T2, typename T3>
        static Triple<T1, T2, T3> make_triple(const T1 & first,const T2 & second, const T3 & third)
        {
            return Triple<T1, T2, T3>{first, second, third};
        }
    }
}

#endif //ANGULARARTGALLERYPROBLEM_COMMON_UTILS_H
