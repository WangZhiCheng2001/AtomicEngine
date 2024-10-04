#pragma once

#include "detail/flat_multiset_base.hpp"

#include "small_vector.hpp"

namespace atom
{
template <template <typename> class Vector, typename Key, typename Compare = std::less<Key>>
class flat_set : public detail::flat_multiset_base<flat_set<Vector, Key, Compare>, false, Vector, Key, Compare>
{
    using base_t = detail::flat_multiset_base<flat_set<Vector, Key, Compare>, false, Vector, Key, Compare>;

public:
    using base_t::base_t;

    flat_set(std::initializer_list<Key> ilist, const Compare& comp = Compare()) : base_t(ilist, comp) {}
};

template <typename Key, typename Compare = std::less<Key>>
using stl_flat_set = flat_set<detail::stl_vector_allocator_bind, Key, Compare>;

template <typename Key, size_t N = 16, typename Compare = std::less<Key>>
using static_flat_set = flat_set<detail::static_vector_bind<N>::template type, Key, Compare>;

template <typename Key, size_t N = 16, typename Compare = std::less<Key>>
using small_flat_set = flat_set<detail::small_vector_bind<N>::template type, Key, Compare>;
} // namespace atom