#pragma once

#include <parallel_hashmap/btree.h>

namespace atom
{
template <typename Key, typename Compare = phmap::Less<Key>, typename Alloc = std::allocator<Key>>
using btree_set = phmap::btree_set<Key, Compare, Alloc>;

template <typename Key, typename Compare = phmap::Less<Key>, typename Alloc = std::allocator<Key>>
using btree_multiset = phmap::btree_multiset<Key, Compare, Alloc>;

template <typename Key,
          typename Value,
          typename Compare = phmap::Less<Key>,
          typename Alloc   = std::allocator<std::pair<const Key, Value>>>
using btree_map = phmap::btree_map<Key, Value, Compare, Alloc>;

template <typename Key,
          typename Value,
          typename Compare = phmap::Less<Key>,
          typename Alloc   = std::allocator<std::pair<const Key, Value>>>
using btree_multimap = phmap::btree_multimap<Key, Value, Compare, Alloc>;
} // namespace atom