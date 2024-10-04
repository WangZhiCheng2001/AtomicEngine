#pragma once

#include "detail/flat_multimap_base.hpp"

#include "small_vector.hpp"

namespace atom
{
template <template <typename> class Vector, typename Key, typename T, typename Compare = std::less<Key>>
class flat_multimap : public detail::flat_multimap_base<flat_multimap<Vector, Key, Compare>, true, Vector, Key, T, Compare>
{
    using base_t = detail::flat_multimap_base<flat_multimap<Vector, Key, Compare>, true, Vector, Key, T, Compare>;

public:
    using base_t::base_t;
    using typename base_t::value_type;

    flat_multimap(std::initializer_list<value_type> ilist, const Compare& comp = Compare()) : base_t(ilist, comp) {}
};

template <typename Key, typename T, typename Compare = std::less<Key>>
using stl_flat_multimap = flat_multimap<detail::stl_vector_allocator_bind, Key, T, Compare>;

template <typename Key, typename T, size_t N = 16, typename Compare = std::less<Key>>
using static_flat_multimap = flat_multimap<detail::static_vector_bind<N>::template type, Key, T, Compare>;

template <typename Key, typename T, size_t N = 16, typename Compare = std::less<Key>>
using small_flat_multimap = flat_multimap<detail::small_vector_bind<N>::template type, Key, T, Compare>;
} // namespace atom