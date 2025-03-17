/*!
 * \file WTSCollection.hpp
 * \project	WonderTrader
 *
 * \author Wesley
 * \date 2020/03/30
 * 
 * \brief Wt collection component definition file
 */
#pragma once
#include "WTSObject.hpp"
#include <vector>
#include <map>
#include <functional>
#include <algorithm>
#include "FasterDefs.h"

#include <deque>

NS_WTP_BEGIN

//////////////////////////////////////////////////////////////////////////
//WTSArray

/*
 *	Platform array container
 *	Implemented using vector internally
 *	Data uses WTSObject pointer objects
 *	All derived classes of WTSObject can use it
 *	Used within the platform
 */
class WTSArray : public WTSObject
{
public:
	/*
	 *	Array iterator
	 */
	typedef std::vector<WTSObject*>::iterator Iterator;
	typedef std::vector<WTSObject*>::const_iterator ConstIterator;

	typedef std::vector<WTSObject*>::reverse_iterator ReverseIterator;
	typedef std::vector<WTSObject*>::const_reverse_iterator ConstReverseIterator;

	typedef std::function<bool(WTSObject*, WTSObject*)>	SortFunc;

	/*
	 *	Create array object
	 */
	static WTSArray* create()
	{
		WTSArray* pRet = new WTSArray();
		return pRet;
	}

	/*
	 *	Read array length
	 */
	inline
	uint32_t size() const{ return (uint32_t)_vec.size(); }

	/*
	 *	Clear array and reallocate space
	 *	Calling this function will pre-allocate length
	 *	Pre-allocated data are all NULL
	 */
	void resize(uint32_t _size)
	{
		if(!_vec.empty())
			clear();

		_vec.resize(_size, NULL);
	}

	/*
	 *	Read data at the specified position of the array
	 *	Compared to the grab interface, the at interface only gets the data
	 *	Does not increase the reference count of the data
	 *	After the grab interface reads the data, the reference count is increased
	 */
	inline
	WTSObject* at(uint32_t idx)
	{
		if(idx <0 || idx >= _vec.size())
			return NULL;

		WTSObject* pRet = _vec.at(idx);
		return pRet;
	}

	inline
	uint32_t idxOf(WTSObject* obj)
	{
		if (obj == NULL)
			return -1;

		uint32_t idx = 0;
		auto it = _vec.begin();
		for (; it != _vec.end(); it++, idx++)
		{
			if (obj == (*it))
				return idx;
		}

		return -1;
	}

	template<typename T> 
	inline T* at(uint32_t idx)
	{
		if(idx <0 || idx >= _vec.size())
			return NULL;

		WTSObject* pRet = _vec.at(idx);
		return static_cast<T*>(pRet);
	}

	/*
	 *	[] operator overload
	 *	Usage is the same as the at function
	 */
	inline
	WTSObject* operator [](uint32_t idx)
	{
		if(idx <0 || idx >= _vec.size())
			return NULL;

		WTSObject* pRet = _vec.at(idx);
		return pRet;
	}

	/*
	 *	Read data at the specified position of the array
	 *	Increase reference count
	 */
	inline
	WTSObject*	grab(uint32_t idx)
	{
		if(idx <0 || idx >= _vec.size())
			return NULL;

		WTSObject* pRet = _vec.at(idx);
		if (pRet)
			pRet->retain();

		return pRet;
	}

	/*
	 *	Append data to the end of the array
	 *	Data automatically increases the reference count
	 */
	inline
	void append(WTSObject* obj, bool bAutoRetain = true)
	{
		if (bAutoRetain && obj)
			obj->retain();

		_vec.emplace_back(obj);
	}

	/*
	 *	Set the data at the specified position
	 *	If there is already data at this position, it will be released
	 *	The reference count of the new data is increased
	 */
	inline
	void set(uint32_t idx, WTSObject* obj, bool bAutoRetain = true)
	{
		if(idx >= _vec.size() || obj == NULL)
			return;

		if(bAutoRetain)
			obj->retain();

		WTSObject* oldObj = _vec.at(idx);
		if(oldObj)
			oldObj->release();

		_vec[idx] = obj;
	}

	inline
	void append(WTSArray* ay)
	{
		if(ay == NULL)
			return;

		_vec.insert(_vec.end(), ay->_vec.begin(), ay->_vec.end());
		ay->_vec.clear();
	}

	/*
	 *	Array clearing
	 *	All data in the array releases references
	 */
	void clear()
	{
		{
			std::vector<WTSObject*>::iterator it = _vec.begin();
			for (; it != _vec.end(); it++)
			{
				WTSObject* obj = (*it);
				if (obj)
					obj->release();
			}
		}
		
		_vec.clear();
	}

	/*
	 *	Release array object, usage is the same as WTSObject
	 *	The difference is that if the reference count is 1
	 *	Release all data
	 */

	virtual void release()
	{
		if (m_uRefs == 0)
			return;

		try
		{
			m_uRefs--;
			if (m_uRefs == 0)
			{
				clear();
				delete this;
			}
		}
		catch(...)
		{

		}
	}

	/*
	 *	Get the iterator of the starting position of the array object
	 */
	inline
	Iterator begin()
	{
		return _vec.begin();
	}

	inline
	ConstIterator begin() const
	{
		return _vec.begin();
	}

	inline
	ReverseIterator rbegin()
	{
		return _vec.rbegin();
	}

	inline
	ConstReverseIterator rbegin() const
	{
		return _vec.rbegin();
	}

	/*
	 *	Get the iterator of the ending position of the array object
	 */
	inline
	Iterator end()
	{
		return _vec.end();
	}

	inline
	ConstIterator end() const
	{
		return _vec.end();
	}

	inline
	ReverseIterator rend()
	{
		return _vec.rend();
	}

	inline
	ConstReverseIterator rend() const
	{
		return _vec.rend();
	}

	inline
	void	sort(SortFunc func)
	{
		std::sort(_vec.begin(), _vec.end(), func);
	}

protected:
	WTSArray():_holding(false){}
	virtual ~WTSArray(){}

	std::vector<WTSObject*>	_vec;
	std::atomic<bool>		_holding;
};


/*
 *	map container
 *	Implemented using std:map internally
 *	The template type is the key type
 *	Data uses WTSObject pointer objects
 *	All derived classes of WTSObject are applicable
 */
template <class T>
class WTSMap : public WTSObject
{
public:
	/*
	 *	Definition of container iterators
	 */
	typedef typename std::map<T, WTSObject*>	_MyType;
	typedef typename _MyType::iterator			Iterator;
	typedef typename _MyType::const_iterator	ConstIterator;
	typedef typename _MyType::reverse_iterator			ReverseIterator;
	typedef typename _MyType::const_reverse_iterator	ConstReverseIterator;

	/*
	 *	Create map container
	 */
	static WTSMap<T>*	create()
	{
		WTSMap<T>* pRet = new WTSMap<T>();
		return pRet;
	}

	/*
	 *	Returns the size of the map container
	 */
	inline
	uint32_t size() const{ return (uint32_t)_map.size(); }

	/*
	 *	Read the data corresponding to the specified key
	 *	Does not increase the reference count of the data
	 *	Returns NULL if not found
	 */
	inline
	WTSObject* get(const T &_key)
	{
		Iterator it = _map.find(_key);
		if(it == _map.end())
			return NULL;

		WTSObject* pRet = it->second;
		return pRet;
	}

	/*
	 *	[] operator overload
	 *	Usage is the same as the get function
	 */
	inline
	WTSObject* operator[](const T &_key)
	{
		Iterator it = _map.find(_key);
		if(it == _map.end())
			return NULL;

		WTSObject* pRet = it->second;
		return pRet;
	}

	/*
	 *	Read the data corresponding to the specified key
	 *	Increase the reference count of the data
	 *	Returns NULL if not found
	 */
	inline
	WTSObject* grab(const T &_key)
	{
		Iterator it = _map.find(_key);
		if(it == _map.end())
			return NULL;

		WTSObject* pRet = it->second;
		if (pRet)
			pRet->retain();

		return pRet;
	}

	/*
	 *	Add a new data, and increase the data reference count
	 *	If the key exists, the original data will be released
	 */
	inline
	void add(T _key, WTSObject* obj, bool bAutoRetain = true)
	{
		if(bAutoRetain && obj)
			obj->retain();

		WTSObject* pOldObj = NULL;
		Iterator it = _map.find(_key);
		if(it != _map.end())
		{
			pOldObj = it->second;
		}

		_map[_key] = obj;

		if (pOldObj) pOldObj->release();
	}

	/*
	 *	Delete a data according to the key
	 *	If the key exists, the corresponding data reference count -1
	 */
	inline
	void remove(T _key)
	{
		Iterator it = _map.find(_key);
		if(it != _map.end())
		{
			WTSObject* obj = it->second;
			_map.erase(it);
			if (obj) obj->release();
		}
	}

	/*
	 *	Get the iterator of the starting position of the container
	 */
	Iterator begin()
	{
		return _map.begin();
	}

	ConstIterator begin() const
	{
		return _map.begin();
	}

	/*
	 *	Get the iterator of the ending position of the container
	 */
	Iterator end()
	{
		return _map.end();
	}

	ConstIterator end() const
	{
		return _map.end();
	}

	/*
	 *	Get the iterator of the starting position of the container
	 */
	ReverseIterator rbegin()
	{
		return _map.rbegin();
	}

	ConstReverseIterator rbegin() const
	{
		return _map.rbegin();
	}

	/*
	 *	Get the iterator of the ending position of the container
	 */
	ReverseIterator rend()
	{
		return _map.rend();
	}

	ConstReverseIterator rend() const
	{
		return _map.rend();
	}

	inline
	Iterator find(const T& key)
	{
		return _map.find(key);
	}

	inline
	ConstIterator find(const T& key) const
	{
		return _map.find(key);
	}

	inline
	void erase(ConstIterator it)
	{
		_map.erase(it);
	}

	inline
	void erase(Iterator it)
	{
		_map.erase(it);
	}

	inline
	void erase(T key)
	{
		_map.erase(key);
	}

	Iterator lower_bound(const T& key)
	{
		 return _map.lower_bound(key);
	}

	ConstIterator lower_bound(const T& key) const
	{
		return _map.lower_bound(key);
	}

	Iterator upper_bound(const T& key)
	{
	 	 return _map.upper_bound(key);
	}
	 
	ConstIterator upper_bound(const T& key) const
	{
		return _map.upper_bound(key);
	}

	inline
	WTSObject* last() 
	{
		if(_map.empty())
			return NULL;
		
		return _map.rbegin()->second;
	}
	

	/*
	 *	Clear the container
	 *	All data in the container reference count -1
	 */
	void clear()
	{
		Iterator it = _map.begin();
		for(; it != _map.end(); it++)
		{
			it->second->release();
		}
		_map.clear();
	}

	/*
	 *	Release the container object
	 *	If the container reference count is 1, clear all data
	 */
	virtual void release()
	{
		if (m_uRefs == 0)
			return;

		try
		{
			m_uRefs--;
			if (m_uRefs == 0)
			{
				clear();
				delete this;
			}
		}
		catch(...)
		{

		}
	}

protected:
	WTSMap(){}
	~WTSMap(){}

	std::map<T, WTSObject*>	_map;
};

/*
 *	map container
 *	Implemented using std:map internally
 *	The template type is the key type
 *	Data uses WTSObject pointer objects
 *	All derived classes of WTSObject are applicable
 */
template <typename T, class Hash = std::hash<T>>
class WTSHashMap : public WTSObject
{
protected:
	WTSHashMap() {}
	virtual ~WTSHashMap() {}

	//std::unordered_map<T, WTSObject*>	_map;
	wt_hashmap<T, WTSObject*, Hash>	_map;

public:
	/*
	 *	Container iterator definition
	 */
	typedef wt_hashmap<T, WTSObject*, Hash>		_MyType;
	typedef typename _MyType::const_iterator	ConstIterator;

	/*
	 *	Create map container
	 */
	static WTSHashMap<T, Hash>*	create() noexcept
	{
		WTSHashMap<T, Hash>* pRet = new WTSHashMap<T, Hash>();
		return pRet;
	}

	/*
	 *	Returns the size of the map container
	 */
	inline uint32_t size() const noexcept {return (uint32_t)_map.size();}

	/*
	 *	Read the data corresponding to the specified key
	 *	Does not increase the reference count of the data
	 *	Returns NULL if not found
	 */
	inline WTSObject* get(const T &_key) noexcept
	{
		auto it = _map.find(_key);
		if(it == _map.end())
			return NULL;

		WTSObject* pRet = it->second;
		return pRet;
	}

	/*
	 *	Read the data corresponding to the specified key
	 *	Increase the reference count of the data
	 *	Returns NULL if not found
	 */
	inline WTSObject* grab(const T &_key) noexcept
	{
		auto it = _map.find(_key);
		if(it == _map.end())
			return NULL;

		WTSObject* pRet = it->second;
		pRet->retain();
		return pRet;
	}

	/*
	 *	Add a new data, and increase the data reference count
	 *	If the key exists, the original data will be released
	 */
	inline void add(const T &_key, WTSObject* obj, bool bAutoRetain = true) noexcept
	{
		if (bAutoRetain && obj)
			obj->retain();

		WTSObject* pOldObj = NULL;
		auto it = _map.find(_key);
		if (it != _map.end())
		{
			pOldObj = it->second;
		}

		_map[_key] = obj;

		if (pOldObj) pOldObj->release();
	}

	/*
	 *	Delete a data according to the key
	 *	If the key exists, the corresponding data reference count -1
	 */
	inline void remove(const T &_key) noexcept
	{
		auto it = _map.find(_key);
		if (it != _map.end())
		{
			WTSObject* obj = it->second;
			_map.erase(it);
			if (obj) obj->release();
		}
	}


	/*
	 *	Get the iterator of the starting position of the container
	 */
	inline ConstIterator begin() const noexcept
	{
		return _map.begin();
	}

	/*
	 *	Get the iterator of the ending position of the container
	 */
	inline ConstIterator end() const noexcept
	{
		return _map.end();
	}

	inline ConstIterator find(const T& key) const noexcept
	{
		return _map.find(key);
	}

	/*
	 *	Clear the container
	 *	All data in the container reference count -1
	 */
	inline void clear() noexcept
	{
		ConstIterator it = _map.begin();
		for(; it != _map.end(); it++)
		{
			it->second->release();
		}
		_map.clear();
	}

	/*
	 *	Release the container object
	 *	If the container reference count is 1, clear all data
	 */
	virtual void release() 
	{
		if (m_uRefs == 0)
			return;

		try
		{
			m_uRefs--;
			if (m_uRefs == 0)
			{
				clear();
				delete this;
			}
		}
		catch (...)
		{

		}
	}
};

//////////////////////////////////////////////////////////////////////////
//WTSQueue
class WTSQueue : public WTSObject
{
public:
	typedef std::deque<WTSObject*>::iterator Iterator;
	typedef std::deque<WTSObject*>::const_iterator ConstIterator;

	static WTSQueue* create()
	{
		WTSQueue* pRet = new WTSQueue();
		return pRet;
	}

	void pop()
	{
		_queue.pop_front();
	}

	void push(WTSObject* obj, bool bAutoRetain = true)
	{
		if (obj && bAutoRetain)
			obj->retain();

		_queue.emplace_back(obj);
	}

	WTSObject* front(bool bRetain = true)
	{
		if(_queue.empty())
			return NULL;

		WTSObject* obj = _queue.front();
		if(bRetain)
			obj->retain();

		return obj;
	}

	WTSObject* back(bool bRetain = true)
	{
		if(_queue.empty())
			return NULL;

		WTSObject* obj = _queue.back();
		if(bRetain)
			obj->retain();

		return obj;
	}

	uint32_t size() const{ return (uint32_t)_queue.size(); }

	bool	empty() const{return _queue.empty();}

	void release()
	{
		if (m_uRefs == 0)
			return;

		try
		{
			m_uRefs--;
			if (m_uRefs == 0)
			{
				clear();
				delete this;
			}
		}
		catch (...)
		{

		}
	}

	void clear()
	{
		Iterator it = begin();
		for(; it != end(); it++)
		{
			(*it)->release();
		}
		_queue.clear();
	}

	/*
	 *	Get the iterator of the starting position of the array object
	 */
	Iterator begin()
	{
		return _queue.begin();
	}

	ConstIterator begin() const
	{
		return _queue.begin();
	}

	void swap(WTSQueue* right)
	{
		_queue.swap(right->_queue);
	}

	/*
	 *	Get the iterator of the ending position of the array object
	 */
	Iterator end()
	{
		return _queue.end();
	}

	ConstIterator end() const
	{
		return _queue.end();
	}

protected:
	WTSQueue(){}
	virtual ~WTSQueue(){}

	std::deque<WTSObject*>	_queue;
};

NS_WTP_END