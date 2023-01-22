#ifndef FUNCTIONAL_H
#define FUNCTIONAL_H

template<typename F, typename... Fs>
auto compose(F f, Fs... fs) {
    return [=](auto x) { return compose(fs...)(f(x)); };
}

template <typename F>
auto compose(F f) {
    return f;
}

auto compose() {
    return [](auto x) { return x; };
}

template <typename H, typename... Fs>
auto lift(H h, Fs... fs) {
    return [=](auto x) { return h(fs(x)...); };
}

template <typename F>
auto lift(F f) {
    return f;
}

#endif
