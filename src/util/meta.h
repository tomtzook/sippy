#pragma once


namespace sippy::util {

template<typename T, typename T2>
bool is_any_of(T t, T2 t2) {
    return t == t2;
}

template<typename T, typename T2, typename... Args>
bool is_any_of(T t, T2 t2, Args... args) {
    return (t == t2) || is_any_of(t, args...);
}

}
