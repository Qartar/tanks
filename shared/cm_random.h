// cm_random.h
//

#pragma once

#include <random>

////////////////////////////////////////////////////////////////////////////////
template<typename engine_type> class random_base
{
public:
    random_base() : _engine({0}) {}

    //! Construct using the given sequence as a seed
    explicit random_base(std::seed_seq seed) : _engine(seed) {}

    //! Construct using the given generator as a seed
    template<typename other_type> explicit random_base(random_base<other_type>& seed) : random_base({seed._engine()}) {}

    //! Returns internal number generator
    engine_type& engine() { return _engine; }

    //! Returns a uniformly distributed real number in the range [0, 1]
    template<typename value_type = float> value_type uniform_real() { return uniform_real(value_type(0), value_type(1)); }

    //! Returns a uniformly distributed real number in the range [0, max]
    template<typename value_type> value_type uniform_real(value_type max) { return uniform_real(value_type(0), max); }

    //! Returns a uniformly distributed real number in the range [min, max]
    template<typename value_type> value_type uniform_real(value_type min, value_type max) {
        static_assert(std::is_floating_point_v<value_type>, "value_type must be a floating-point type");
        return std::uniform_real_distribution<value_type>(min, max)(_engine);
    }

    //! Returns a uniformly distributed integer in the range [0, max)
    template<typename value_type> value_type uniform_int(value_type max) { return uniform_int(static_cast<value_type>(0), max); }

    //! Returns a uniformly distributed integer in the range [min, max)
    template<typename value_type> value_type uniform_int(value_type min, value_type max) {
        static_assert(std::is_integral_v<value_type>, "value_type must be an integer type");
        return std::uniform_int_distribution<value_type>(min, max - 1)(_engine);
    }

    //! Returns a normal-distributed real number with mean value of 0 and standard deviation of 1
    template<typename value_type = float> value_type normal_real() { return normal_real(value_type(1), value_type(0)); }

    //! Returns a normal-distributed real number with mean value of 0 and standard deviation of `sigma`
    template<typename value_type> value_type normal_real(value_type sigma) { return normal_real(sigma, value_type(0)); }

    //! Returns a normal-distributed real number with mean value of `mu` and standard deviation of `sigma`
    template<typename value_type> value_type normal_real(value_type sigma, value_type mu) {
        static_assert(std::is_floating_point_v<value_type>, "value_type must be a floating-point type");
        return std::normal_distribution<value_type>(mu, sigma)(_engine);
    }

protected:
    engine_type _engine;
};

//------------------------------------------------------------------------------
using random = random_base<std::minstd_rand>;
