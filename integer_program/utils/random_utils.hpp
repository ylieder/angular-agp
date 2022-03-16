#ifndef MWT_RANDOM_UTILS_HPP
#define MWT_RANDOM_UTILS_HPP
#include  <random>
#include  <iterator>

namespace utils {
    namespace random {
        template<typename Iter, typename RandomGenerator>
        Iter select_randomly(Iter start, Iter end, RandomGenerator &g) {
            std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
            std::advance(start, dis(g));
            return start;
        }

        template<typename Iter>
        Iter select_randomly(Iter start, Iter end) {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            return select_randomly(start, end, gen);
        }

        template<typename Iter, typename RandomGenerator>
        Iter select_randomly_weighted(Iter start, Iter end, RandomGenerator &g) {
            auto distributions = std::vector<int>(std::distance(start, end), 0);
            std::iota(distributions.begin(), distributions.end(), 1);
            std::reverse(distributions.begin(), distributions.end());
            std::discrete_distribution<int> dis(distributions.begin(), distributions.end());
            std::advance(start, dis(g));
            return start;
        }

        template<typename Iter>
        Iter select_randomly_biased(Iter start, Iter end) {
            std::random_device rd;
            std::mt19937 gen(rd());
            return select_randomly_weighted(start, end, gen);
        }

        /**
         * returns random double in range [lb, ub)
         */
        double random_double(double lb = 0, double ub = 1) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dist(lb, ub);

            return dist(gen);
        }

        /**
         * returns random int in range [lb, ub]
         */
        int random_int(int lb, int ub) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dist(lb, ub);

            return dist(gen);
        }
    }
}

#endif //MWT_RANDOM_UTILS_HPP
