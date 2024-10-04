#pragma once

#include <vector>

#include "detail/small_vector_base.hpp"

namespace atom
{
namespace detail
{
template <typename T>
using stl_vector_allocator_bind = std::vector<T>;
} // namespace detail

template <typename T, std::size_t N = 16>
using small_vector = detail::small_vector_base<detail::stl_vector_allocator_bind, T, N>;

namespace detail
{
template <size_t N>
struct small_vector_bind {
    template <typename T>
    using type = small_vector<T, N>;
};
} // namespace detail
} // namespace atom