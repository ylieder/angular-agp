//
// Created by Yannic Lieder on 06.10.19.
//

#ifndef ANGULARARTGALLERYPROBLEM_TIMEOUT_H
#define ANGULARARTGALLERYPROBLEM_TIMEOUT_H

#include <boost/thread.hpp>

namespace utils
{
    class TimeoutException : public std::runtime_error {
    public:
        TimeoutException(int timeout_ms) : std::runtime_error("Timeout after " + std::to_string(timeout_ms) + "ms") { }
    };

    template <typename TFunc>
    static void timeout(int timeout_ms, const TFunc& code)
    {
        boost::thread t(code);
        if (t.try_join_for(boost::chrono::milliseconds(timeout_ms)))
        {
            return;
        } else {
            throw TimeoutException(timeout_ms);
        }
    }
}

#endif //ANGULARARTGALLERYPROBLEM_TIMEOUT_H
