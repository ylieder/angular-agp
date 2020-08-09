//
// Interval arithmetic functions.
//
// Created by Yannic Lieder on 17.01.20.
//

#ifndef ANGULARARTGALLERYPROBLEM_INTERVAL_ARITHMETIC_H
#define ANGULARARTGALLERYPROBLEM_INTERVAL_ARITHMETIC_H

#include <boost/numeric/interval.hpp>
#include <CGAL/Interval_nt.h>


namespace ia {
    typedef boost::numeric::interval_lib::comparison_error IntervalComparisonError;

    typedef boost::numeric::interval<double,
            boost::numeric::interval_lib::policies<
                    boost::numeric::interval_lib::save_state<boost::numeric::interval_lib::rounded_transc_std<double>>,
                    boost::numeric::interval_lib::checking_base<double>>> DoubleInterval;

    static DoubleInterval min(DoubleInterval i1, DoubleInterval i2) {
        return boost::numeric::min(i1, i2);
    }

    static DoubleInterval max(DoubleInterval i1, DoubleInterval i2) {
        return boost::numeric::max(i1, i2);
    }

    static DoubleInterval pi() {
        return boost::numeric::interval_lib::pi<DoubleInterval>();
    }

    static DoubleInterval piTwice() {
        return boost::numeric::interval_lib::pi_twice<DoubleInterval>();
    }

    static DoubleInterval cos(DoubleInterval radians) {
        return boost::numeric::cos(DoubleInterval(radians));
    }

    static DoubleInterval acos(DoubleInterval x) {
        return boost::numeric::acos(x);
    }

    template<typename Kernel>
    static DoubleInterval get(typename Kernel::FT const &ft) {
        CGAL::Interval_nt i = CGAL::to_interval(ft);
        return DoubleInterval(i.inf(), i.sup());
    }

    static DoubleInterval sqrt(DoubleInterval interval) {
        return boost::numeric::sqrt(interval);
    }

    template<typename Kernel>
    static DoubleInterval sqrt(typename Kernel::FT const &ft) {
        return sqrt(get<Kernel>(ft));
    }
}

#endif //ANGULARARTGALLERYPROBLEM_INTERVAL_ARITHMETIC_H
