#pragma once

#include <type_traits>
#include <concepts>

template <typename T>
concept is_container_type = requires(T x) { x[0]; x.begin(); x.end(); x.size(); };

template <typename T>
    requires is_container_type<T>
using container_element_type = typename T::value_type;