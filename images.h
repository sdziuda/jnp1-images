#ifndef IMAGES_H
#define IMAGES_H

#include "coordinate.h"
#include "color.h"
#include <functional>
#include <cmath>
#include <iostream>

using Fraction = double;

template<typename T>
using Base_image = std::function<T(const Point)>;

using Region = Base_image<bool>;

using Image = Base_image<Color>;

using Blend = Base_image<Fraction>;

template<typename T>
Base_image<T> constant(const T& t) {
    return [=](const Point&) { return t; };
}

template<typename T>
Base_image<T> rotate(Base_image<T> image, double phi) {
    return [=](const Point& p) {
        double result_rho = p.is_polar ? p.first : to_polar(p).first;
        double result_phi = p.is_polar ? p.second - phi : to_polar(p).second - phi;
        return image(from_polar(Point(result_rho, result_phi, true)));
    };
}

template<typename T>
Base_image<T> translate(Base_image<T> image, const Vector& v) {
    return [=](const Point& p) {
        return image(Point(p.first - v.first, p.second - v.second, p.is_polar));
    };
}

template<typename T>
Base_image<T> scale(Base_image<T> image, double s) {
    return [=](const Point& p) {
        double result_rho = p.is_polar ? p.first / s : to_polar(p).first / s;
        double result_phi = p.is_polar ? p.second : to_polar(p).second;
        return image(from_polar(Point(result_rho, result_phi, true)));
    };
}

template<typename T>
Base_image<T> circle(Point q, double r, T inner, T outer) {
    return [=](const Point& p) {
        return distance(p, q) <= r ? inner : outer;
    };
}

template<typename T>
Base_image<T> checker(double d, T this_way, T that_way) {
    return [=](const Point& p) {
        return (static_cast<int>(std::floor(p.first / d)) +
                static_cast<int>(std::floor(p.second / d))) % 2 == 0 ? this_way : that_way;
    };
}

template<typename T>
Base_image<T> polar_checker(double d, int n, T this_way, T that_way) {
    return [=](const Point& p) {
        auto p1 = Point(to_polar(p).second / (2 * M_PI) * n * d, distance(p));
        return checker(d, this_way, that_way)(p1);
    };
}

template<typename T>
Base_image<T> rings(Point q, double d, T this_way, T that_way) {
    return [=](const Point& p) {
        return std::fmod(std::abs(distance(p, q)), 2 * d) < d ? this_way : that_way;
    };
}

template<typename T>
Base_image<T> vertical_stripe(double d, T this_way, T that_way) {
    return [=](const Point& p) {
        return std::abs(p.first) <= (d / 2) ? this_way : that_way;
    };
}

inline Image cond(Region region, Image this_way, Image that_way) {
    return [=](const Point& p) {
        return region(p) ? this_way(p) : that_way(p);
    };
}

inline Image lerp(Blend blend, Image this_way, Image that_way) {
    return [=](const Point& p) {
        return this_way(p).weighted_mean(that_way(p), blend(p));
    };
}

inline Image darken(Image image, Blend blend) {
    return [=](const Point& p) {
        return image(p).weighted_mean(Colors::black, blend(p));
    };
}

inline Image lighten(Image image, Blend blend) {
    return [=](const Point& p) {
        return image(p).weighted_mean(Colors::white, blend(p));
    };
}

#endif