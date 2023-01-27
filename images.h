#ifndef IMAGES_H
#define IMAGES_H

#include "coordinate.h"
#include "color.h"
#include "functional.h"
#include <functional>
#include <cmath>
#include <iostream>

using Fraction = double;

template<typename T>
using Base_image = std::function<T(const Point)>;

using Region = Base_image<bool>;

using Image = Base_image<Color>;

using Blend = Base_image<Fraction>;

namespace Detail {
    inline auto make_polar(const Point p) {
        return p.is_polar ? p : to_polar(p);
    }

    inline auto make_cartesian(const Point p) {
        return p.is_polar ? from_polar(p) : p;
    }

    inline auto cond(Region r, auto this_way, auto that_way) {
        return [=](const Point p) {
            return r(p) ? this_way(p) : that_way(p);
        };
    }

    /**
     * Wrapper for std::fmod that can be used in compose and lift.
     */
    inline auto mod(double x, double y) {
        return std::fmod(x, y);
    }

    /**
     * Wrapper for std::abs that can be used in compose and lift.
     */
    inline auto abs(double x) {
        return std::abs(x);
    }
}

template<typename T>
Base_image<T> constant(T t) {
    return [=](const Point) {return t;};
}

template<typename T>
Base_image<T> rotate(Base_image<T> image, double phi) {
    auto rot = [](const Point p, double phi) {
        return Point(Detail::make_polar(p).first, Detail::make_polar(p).second - phi, true);
    };
    auto rot_phi = std::bind(rot, std::placeholders::_1, phi);

    return compose(rot_phi, Detail::make_cartesian, image);
}

template<typename T>
Base_image<T> translate(Base_image<T> image, Vector v) {
    auto move = [](const Point p, Vector v) {
        return Point(Detail::make_cartesian(p).first - v.first,
                     Detail::make_cartesian(p).second - v.second);
    };
    auto move_v = std::bind(move, std::placeholders::_1, v);

    return compose(move_v, image);
}

template<typename T>
Base_image<T> scale(Base_image<T> image, double s) {
    auto scale = [](const Point p, double s) {
        return Point(p.first / s, p.second / s);
    };
    auto scale_s = std::bind(scale, std::placeholders::_1, s);

    return compose(Detail::make_cartesian, scale_s, image);
}

template<typename T>
Base_image<T> circle(Point q, double r, T inner, T outer) {
    auto cart_p = std::bind(Detail::make_cartesian, std::placeholders::_1);
    auto dist_q = std::bind(distance, cart_p, Detail::make_cartesian(q));
    auto less_r = lift(std::less_equal<>(), dist_q, constant(r));

    return Detail::cond(less_r, constant(inner), constant(outer));
}

template<typename T>
Base_image<T> checker(double d, T this_way, T that_way) {
    auto checker = [](const Point p, double d) {
        return (static_cast<int>(std::floor(p.first / d)) +
                static_cast<int>(std::floor(p.second / d))) % 2;
    };
    auto cart_p = std::bind(Detail::make_cartesian, std::placeholders::_1);
    auto checker_d = std::bind(checker, cart_p, d);
    auto equal_0 = lift(std::equal_to<>(), checker_d, constant(0));

    return Detail::cond(equal_0, constant(this_way), constant(that_way));
}

template<typename T>
Base_image<T> polar_checker(double d, int n, T this_way, T that_way) {
    auto change_first = [](const Point p) {
        return p.first;
    };
    auto change_second = [](const Point p, double d, int n) {
        return d * n * p.second / (2.0 * M_PI);
    };
    auto change_second_d_n = std::bind(change_second, std::placeholders::_1, d, n);
    auto make_point = [](double x, double y) {
        return Point(x, y);
    };
    auto change_p = compose(Detail::make_polar, lift(make_point, change_first, change_second_d_n));

    return compose(change_p, checker(d, this_way, that_way));
}

template<typename T>
Base_image<T> rings(Point q, double d, T this_way, T that_way) {
    auto cart_p = std::bind(Detail::make_cartesian, std::placeholders::_1);
    auto dist_q = std::bind(distance, cart_p, Detail::make_cartesian(q));
    auto mod_d = std::bind(Detail::mod, std::placeholders::_1, 2 * d);
    auto less_d = lift(std::less_equal<>(), compose(dist_q, Detail::abs, mod_d), constant(d));

    return Detail::cond(less_d, constant(this_way), constant(that_way));
}

template<typename T>
Base_image<T> vertical_stripe(double d, T this_way, T that_way) {
    auto abs_first = [](const Point p) {
        return std::abs(Detail::make_cartesian(p).first);
    };
    auto less_d = lift(std::less_equal<>(), abs_first, constant(d / 2.0));

    return Detail::cond(less_d, constant(this_way), constant(that_way));
}

inline Image cond(Region region, Image this_way, Image that_way) {
    return Detail::cond(region, this_way, that_way);
}

inline Image lerp(Blend blend, Image this_way, Image that_way) {
    return [=](const Point p) {
        return this_way(p).weighted_mean(that_way(p), blend(p));
    };
}

inline Image darken(Image image, Blend blend) {
    return [=](const Point p) {
        return image(p).weighted_mean(Colors::black, blend(p));
    };
}

inline Image lighten(Image image, Blend blend) {
    return [=](const Point p) {
        return image(p).weighted_mean(Colors::white, blend(p));
    };
}

#endif