#ifndef BWGAME_CONTAINERS_H
#define BWGAME_CONTAINERS_H

#include <array>
#include <memory>
#include <vector>
#include <deque>
#include <list>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <string>

#include "static_vector.h"
#include "intrusive_list.h"
#include "circular_vector.h"

namespace bwgame {

template<typename T>
using alloc = std::allocator<T>;

template<typename T>
using a_vector = std::vector<T, alloc<T>>;

template<typename T>
using a_deque = std::deque<T, alloc<T>>;
template<typename T>
using a_list = std::list<T, alloc<T>>;

template<typename T, typename pred_T = std::less<T>>
using a_set = std::set<T, pred_T, alloc<T>>;
template<typename T, typename pred_T = std::less<T>>
using a_multiset = std::multiset<T, pred_T, alloc<T>>;

template<typename key_T, typename value_T, typename pred_T = std::less<key_T>>
using a_map = std::map<key_T, value_T, pred_T, alloc<std::pair<const key_T, value_T>>>;
template<typename key_T, typename value_T, typename pred_T = std::less<key_T>>
using a_multimap = std::multimap<key_T, value_T, pred_T, alloc<std::pair<const key_T, value_T>>>;

template<typename T, typename hash_t = std::hash<T>, typename equal_to_t = std::equal_to<T>>
using a_unordered_set = std::unordered_set<T, hash_t, equal_to_t, alloc<T>>;
template<typename T, typename hash_t = std::hash<T>, typename equal_to_t = std::equal_to<T>>
using a_unordered_multiset = std::unordered_multiset<T, hash_t, equal_to_t, alloc<T>>;

template<typename key_T, typename value_T, typename hash_t = std::hash<key_T>, typename equal_to_t = std::equal_to<key_T>>
using a_unordered_map = std::unordered_map<key_T, value_T, hash_t, equal_to_t, alloc<std::pair<const key_T, value_T>>>;
template<typename key_T, typename value_T, typename hash_t = std::hash<key_T>, typename equal_to_t = std::equal_to<key_T>>
using a_unordered_multimap = std::unordered_multimap<key_T, value_T, hash_t, equal_to_t, alloc<std::pair<const key_T, value_T>>>;

using a_string = std::basic_string<char, std::char_traits<char>, alloc<char>>;

template<typename T>
using a_circular_vector = circular_vector<T, alloc<T>>;

}

#endif
