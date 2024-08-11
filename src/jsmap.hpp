#pragma once

// TODO: need test

#include <map>
#include <vector>
#include <iostream>

template<typename _Key, typename _Tp>
class jsmap
{
public:
    typedef _Key key_type;
    typedef _Tp mapped_type;
    typedef std::pair<const _Key, _Tp> value_type;

private:
    typedef std::map<key_type, mapped_type> map_type;
    typedef typename map_type::iterator map_iterator_type;
    map_type __map;
    std::vector<map_iterator_type> __iter;

public:
    class _jsmap_iterator {
    public:
        typedef value_type& reference;
        typedef value_type* pointer;

        typedef std::bidirectional_iterator_tag iterator_category;
        typedef std::ptrdiff_t difference_type;

        typedef _jsmap_iterator _Self;
        typedef typename std::vector<map_iterator_type>::iterator _Base_ptr;

    private:
        _Base_ptr __ptr;

    public:
        _jsmap_iterator() : __ptr() {}
        _jsmap_iterator(_Base_ptr __ptr) : __ptr(__ptr) {}

        reference operator*() const  {
            return **__ptr;
        }

        pointer operator->() const {
            return &**__ptr;
        }

        _Self& operator++() {
            __ptr++;
            return *this;
        }

        _Self operator++(int) {
            _Self __tmp = *this;
            __ptr++;
            return __tmp;
        }

        _Self& operator--() {
            __ptr--;
            return *this;
        }

        _Self operator--(int) {
            _Self __tmp = *this;
            __ptr--;
            return __tmp;
        }

      friend bool
      operator==(const _Self& __x, const _Self& __y)
      { return __x.__ptr == __y.__ptr; }

      friend bool
      operator!=(const _Self& __x, const _Self& __y)
      { return __x.__ptr != __y.__ptr; }
    };

public:
    typedef _jsmap_iterator iterator;
    typedef const _jsmap_iterator const_iterator;

public:
    mapped_type& operator[](const key_type& __k) {
        auto __i = __map.lower_bound(__k);
        if (__i == __map.end() || std::less<key_type>{}(__k, (*__i).first)) {
            __i = __map.emplace_hint(__i, std::piecewise_construct, std::tuple<const key_type&>(__k), std::tuple<>());
            __iter.emplace_back(__i);
        }
        return (*__i).second;
    }

    mapped_type& operator[](key_type&& __k) {
        auto __i = __map.lower_bound(__k);
        if (__i == __map.end() || std::less<key_type>{}(__k, (*__i).first)) {
            __i = __map.emplace_hint(__i, std::piecewise_construct, std::forward_as_tuple(std::move(__k)), std::tuple<>());
            __iter.emplace_back(__i);
        }
        return (*__i).second;
    }

    template <typename... _Args>
    std::pair<iterator, bool> try_emplace(const key_type& __k, _Args&&... __args) {
        auto __i = __map.lower_bound(__k);
        if (__i == __map.end() || std::less<key_type>{}(__k, (*__i).first)) {
            __i = __map.emplace_hint(__i, std::piecewise_construct, std::forward_as_tuple(__k), std::forward_as_tuple(std::forward<_Args>(__args)...));
            __iter.emplace_back(__i);
            return {iterator(__iter.end() - 1), true};
        }
        return {iterator(std::find(__iter.begin(), __iter.end(), __i)), false};
    }

    template <typename... _Args>
    std::pair<iterator, bool> try_emplace(key_type&& __k, _Args&&... __args) {
        auto __i = __map.lower_bound(__k);
        if (__i == __map.end() || std::less<key_type>{}(__k, (*__i).first)) {
            __i = __map.emplace_hint(__i, std::piecewise_construct, std::forward_as_tuple(std::move(__k)), std::forward_as_tuple(std::forward<_Args>(__args)...));
            __iter.emplace_back(__i);
            return {iterator(__iter.end() - 1), true};
        }
        return {iterator(std::find(__iter.begin(), __iter.end(), __i)), false};
    }

    iterator begin() {
        return iterator(__iter.begin());
    }

    const_iterator begin() const {
        return iterator(__iter.begin());
    }

    iterator end() {
        return iterator(__iter.end());
    }

    const_iterator end() const {
        return iterator(__iter.end());
    }

    iterator find(const key_type& __k) {
        const auto __i = __map.find(__k);
        return __i == __map.end() ? end() : iterator(std::find(__iter.begin(), __iter.end(), __i));
    }

    const_iterator find(const key_type& __k) const {
        const auto __i = __map.find(__k);
        return __i == __map.end() ? end() : iterator(std::find(__iter.begin(), __iter.end(), __i));
    }

    size_t erase(const key_type& __k) {
        const auto __i = __map.find(__k);
        if (__i == __map.end()) {
            return 0;
        } else {
            __iter.erase(std::find(__iter.begin(), __iter.end(), __i));
            __map.erase(__i);
            return 1;
        }
    }
};
