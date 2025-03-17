#pragma once
#include <string.h>
#include "WTSMarcos.h"
#include "../FasterLibs/tsl/robin_map.h"
#include "../FasterLibs/tsl/robin_set.h"

#include "../FasterLibs/ankerl/unordered_dense.h"

/*
 *	By Wesley @ 2023.08.15
 *	Unfortunately, robin_map with std::string will have a bad allocate exception when the amount of data is large (tested at 13106 data, the specific value may be different for different test machines)
 *	I guess std::string cannot be automatically optimized like string
 *	So when the amount of data is large, it will take up a lot of memory. When the running environment has less memory, an exception will occur
 *	So this time, LongKey and LongKey are commented out and changed to std::string
 */

 /*
  *	By Wesley @ 2023.08.16
  *	ankerl's writing speed is much better than robin, about 1/3 faster, especially when the amount of data is within 40w
  *	However, robin's reading speed is better than robin's, but the difference is not big when the data is within 30w
  *	According to the wondertrader scenario, ankerl is much better
  * You can refer to the following page for performance comparison
  * https://martin.ankerl.com/2022/08/27/hashmap-bench-01/#benchmark-results-table
  */

NS_WTP_BEGIN

struct string_hash
{
	//BKDRHash算法
	std::size_t operator()(const std::string& key) const
	{
		size_t seed = 131; // 31 131 1313 13131 131313 etc..
		size_t hash = 0;

		char* str = (char*)key.c_str();
		while (*str)
		{
			hash = hash * seed + (*str++);
		}

		return (hash & 0x7FFFFFFF);
	}
};

template<class Key, class T>
class fastest_hashmap : public tsl::robin_map<Key, T>
{
public:
	typedef tsl::robin_map<Key, T>	Container;
	fastest_hashmap():Container(){}
};

template<class T>
class fastest_hashmap<std::string, T> : public tsl::robin_map<std::string, T, string_hash>
{
public:
	typedef tsl::robin_map<std::string, T, string_hash>	Container;
	fastest_hashmap() :Container() {}
};

template<class Key>
class fastest_hashset : public tsl::robin_set<Key>
{
public:
	typedef tsl::robin_set<Key>	Container;
	fastest_hashset() :Container() {}
};

template<>
class fastest_hashset<std::string> : public tsl::robin_set<std::string, string_hash>
{
public:
	typedef tsl::robin_set<std::string, string_hash>	Container;
	fastest_hashset() :Container() {}
};

typedef fastest_hashset<std::string> CodeSet;

//////////////////////////////////////////////////////////////////////////
// user unordered_dense

template<class Key, class T, class Hash = std::hash<Key>>
class wt_hashmap : public ankerl::unordered_dense::map<Key, T, Hash>
{
public:
	typedef ankerl::unordered_dense::map<Key, T, Hash>	Container;
	wt_hashmap() :Container() {}
};

template<class T>
class wt_hashmap<std::string, T, string_hash> : public ankerl::unordered_dense::map<std::string, T, string_hash>
{
public:
	typedef ankerl::unordered_dense::map<std::string, T, string_hash>	Container;
	wt_hashmap() :Container() {}
};

template<class Key, class Hash = std::hash<Key>>
class wt_hashset : public ankerl::unordered_dense::set<Key, Hash>
{
public:
	typedef ankerl::unordered_dense::set<Key, Hash>	Container;
	wt_hashset() :Container() {}
};

template<>
class wt_hashset<std::string, string_hash> : public ankerl::unordered_dense::set<std::string, string_hash>
{
public:
	typedef ankerl::unordered_dense::set<std::string, string_hash>	Container;
	wt_hashset() :Container() {}
};

NS_WTP_END
