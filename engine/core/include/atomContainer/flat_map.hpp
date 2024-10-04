#pragma once

#include "detail/flat_multimap_base.hpp"

#include "small_vector.hpp"

namespace atom
{
template <template <typename> class Vector, typename Key, typename T, typename Compare = std::less<Key>>
class flat_map : public detail::flat_multimap_base<flat_map<Vector, Key, Compare>, false, Vector, Key, T, Compare>
{
    using base_t = detail::flat_multimap_base<flat_map<Vector, Key, Compare>, false, Vector, Key, T, Compare>;

public:
    using base_t::base_t;
    using typename base_t::key_type;
    using typename base_t::mapped_type;
    using typename base_t::value_type;

    using typename base_t::const_iterator;
    using typename base_t::iterator;

    flat_map(std::initializer_list<value_type> ilist, const Compare& comp = Compare()) : base_t(ilist, comp) {}

    mapped_type& at(const key_type& key)
    {
        auto target = base_t::find(key);
        if (target == base_t::end()) ATOM_fatal("Invalid flat_map<K, V> subscript: Referring index is out of range!");
        return target->second;
    }

    const mapped_type& at(const key_type& key) const { return const_cast<flat_map*>(this)->at(key); }

    mapped_type& operator[](const key_type& key) { return try_emplace(key).first->second; }

    mapped_type& operator[](key_type&& key) { return try_emplace(std::move(key)).first->second; }

    template <typename M>
    std::pair<iterator, bool> insert_or_assign(const key_type& k, M&& m)
    {
        return insert_or_assign_impl(k, std::forward<M>(m));
    }

    template <typename M>
    std::pair<iterator, bool> insert_or_assign(key_type&& k, M&& m)
    {
        return insert_or_assign_impl(std::move(k), std::forward<M>(m));
    }

    template <typename M>
    iterator insert_or_assign(const_iterator hint, const key_type& k, M&& m)
    {
        return insert_or_assign_hint_impl(hint, k, std::forward<M>(m));
    }

    template <typename M>
    iterator insert_or_assign(const_iterator hint, key_type&& k, M&& m)
    {
        return insert_or_assign_hint_impl(hint, std::move(k), std::forward<M>(m));
    }

    template <typename... Args>
    std::pair<iterator, bool> try_emplace(const key_type& k, Args&&... args)
    {
        return try_emplace_impl(k, std::forward<Args>(args)...);
    }

    template <typename... Args>
    std::pair<iterator, bool> try_emplace(key_type&& k, Args&&... args)
    {
        return try_emplace_impl(std::move(k), std::forward<Args>(args)...);
    }

    template <typename... Args>
    iterator try_emplace(const_iterator hint, const key_type& k, Args&&... args)
    {
        return try_emplace_hint_impl(hint, k, std::forward<Args>(args)...);
    }

    template <typename... Args>
    iterator try_emplace(const_iterator hint, key_type&& k, Args&&... args)
    {
        return try_emplace_hint_impl(hint, std::move(k), std::forward<Args>(args)...);
    }

private:
    template <typename K, typename... Args>
    void storage_emplace_back(K&& k, Args&&... args)
    {
        if constexpr (std::is_constructible_v<value_type, std::piecewise_construct_t, std::tuple<K&&>, std::tuple<Args&&...>>) {
            base_t::storage.emplace_back(std::piecewise_construct_t{},
                                         std::forward_as_tuple(std::forward<K>(k)),
                                         std::forward_as_tuple(std::forward<Args>(args)...));
        } else
            base_t::storage.emplace_back(std::forward<K>(k), mapped_type(std::forward<Args>(args)...));
    }

    template <typename K, typename... Args>
    iterator storage_emplace(const_iterator hint, K&& k, Args&&... args)
    {
        if constexpr (std::is_constructible_v<value_type, std::piecewise_construct_t, std::tuple<K&&>, std::tuple<Args&&...>>) {
            return base_t::cast_iterator(base_t::storage.emplace(base_t::cast_iterator(hint),
                                                                 std::piecewise_construct_t{},
                                                                 std::forward_as_tuple(std::forward<K>(k)),
                                                                 std::forward_as_tuple(std::forward<Args>(args)...)));
        } else {
            return base_t::cast_iterator(base_t::storage.emplace(base_t::cast_iterator(hint),
                                                                 std::forward<K>(k),
                                                                 mapped_type(std::forward<Args>(args)...)));
        }
    }

    template <typename K, typename M>
    std::pair<iterator, bool> insert_or_assign_impl(K&& k, M&& m)
    {
        auto lb = base_t::lower_bound(k);                      // k <= lb
        if (lb == base_t::end() || base_t::key_comp()(k, *lb)) // k < lb
            return {base_t::cast_iterator(
                        base_t::storage.insert(base_t::cast_iterator(lb), value_type(std::forward<K>(k), std::forward<M>(m)))),
                    true};
        else { // k == lb
            lb->second = std::forward<M>(m);
            return {lb, false};
        }
    }

    template <typename K, typename M>
    iterator insert_or_assign_hint_impl(const_iterator hint, K&& k, M&& m)
    {
        assert(base_t::begin() <= hint && hint <= base_t::end());

        const_iterator first;
        const_iterator last;

        if (hint == base_t::begin() || base_t::key_comp()(*std::prev(hint), k)) { // k > hint - 1
            if (hint < base_t::end()) {
                if (base_t::key_comp()(k, *hint))                                 // k < hint
                    return storage_emplace(hint, std::forward<K>(k), mapped_type(std::forward<M>(m)));
                else {                                                            // k >= hint
                    first = hint;
                    last  = base_t::cend();
                }
            } else { // hint == end()
                storage_emplace_back(std::forward<K>(k), mapped_type(std::forward<M>(m)));
                return std::prev(base_t::end());
            }
        } else { // k <= hint - 1
            first = base_t::cbegin();
            last  = hint;
        }

        auto lb = std::lower_bound(first, last, k, base_t::key_comp()); // value <= lb


        if (lb == base_t::end() || base_t::key_comp()(k, *lb))          // value < lb
            return storage_emplace(lb, std::forward<K>(k), mapped_type(std::forward<M>(m)));
        else {                                                          // value == lb
            auto iter    = base_t::begin() + std::distance(base_t::cbegin(), lb);
            iter->second = std::forward<M>(m);
            return iter;
        }
    }

    template <typename K, typename... Args>
    std::pair<iterator, bool> try_emplace_impl(K&& k, Args&&... args)
    {
        auto lb = base_t::lower_bound(k);                      // key <= lb
        if (lb == base_t::end() || base_t::key_comp()(k, *lb)) // key < lb
            return {storage_emplace(lb, std::forward<K>(k), mapped_type(std::forward<Args>(args)...)), true};
        else
            return {lb, false};                                // key == lb
    }

    template <typename K, typename... Args>
    iterator try_emplace_hint_impl(const_iterator hint, K&& k, Args&&... args)
    {
        assert(base_t::begin() <= hint && hint <= base_t::end());

        const_iterator first;
        const_iterator last;

        if (hint == base_t::begin() || base_t::key_comp()(*std::prev(hint), k)) { // k > hint - 1
            if (hint < base_t::end()) {
                if (base_t::key_comp()(k, *hint))                                 // k < hint
                    return storage_emplace(hint, std::forward<K>(k), mapped_type(std::forward<Args>(args)...));
                else {                                                            // k >= hint
                    first = hint;
                    last  = base_t::cend();
                }
            } else { // hint == end()
                storage_emplace_back(std::forward<K>(k), mapped_type(std::forward<Args>(args)...));
                return std::prev(base_t::end());
            }
        } else { // k <= hint - 1
            first = base_t::cbegin();
            last  = hint;
        }

        auto lb = std::lower_bound(first, last, k, base_t::key_comp());   // value <= lb


        if (lb == base_t::end() || base_t::key_comp()(k, *lb))            // value < lb
            return storage_emplace(lb, std::forward<K>(k), mapped_type(std::forward<Args>(args)...));
        else
            return base_t::begin() + std::distance(base_t::cbegin(), lb); // value == lb
    }
};

template <typename Key, typename T, typename Compare = std::less<Key>>
using stl_flat_map = flat_map<detail::stl_vector_allocator_bind, Key, T, Compare>;

template <typename Key, typename T, size_t N = 16, typename Compare = std::less<Key>>
using static_flat_map = flat_map<detail::static_vector_bind<N>::template type, Key, T, Compare>;

template <typename Key, typename T, size_t N = 16, typename Compare = std::less<Key>>
using small_flat_map = flat_map<detail::small_vector_bind<N>::template type, Key, T, Compare>;
} // namespace atom