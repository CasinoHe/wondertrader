/*!
 * \file WTSObject.hpp
 * \project	WonderTrader
 *
 * \author Wesley
 * \date 2020/03/30
 * 
 * \brief Wt Base Object Definition
 */
#pragma once
#include <stdint.h>
#include <atomic>
#include <boost/smart_ptr/detail/spinlock.hpp>

#include "WTSMarcos.h"
#include "../Share/ObjectPool.hpp"
#include "../Share/SpinMutex.hpp"

NS_WTP_BEGIN
class WTSObject
{
public:
	WTSObject() :m_uRefs(1){}
	virtual ~WTSObject(){}

public:
	inline uint32_t		retain(){ return m_uRefs.fetch_add(1) + 1; }

	virtual void	release()
	{
		if (m_uRefs == 0)
			return;

		try
		{
			uint32_t cnt = m_uRefs.fetch_sub(1);
			if (cnt == 1)
			{
				delete this;
			}
		}
		catch(...)
		{

		}
	}

	inline bool			isSingleRefs() { return m_uRefs == 1; }

	inline uint32_t		retainCount() { return m_uRefs; }

protected:
	volatile std::atomic<uint32_t>	m_uRefs;
};

template<typename T>
class WTSPoolObject : public WTSObject
{
private:
	typedef ObjectPool<T> MyPool;
	MyPool*			_pool;
	SpinMutex*	_mutex;

public:
	WTSPoolObject():_pool(NULL){}
	virtual ~WTSPoolObject() {}

public:
	static T*	allocate()
	{
		/*
		 *	By Wesley @ 2022.06.14
		 *	Some users reported that thread_local is used here, and the memory pool is also destroyed when the thread is destroyed.
		 *	The user reproduced this bug in Trader. If Trader destroys an API object instance at the bottom layer
		 *	Then the memory pool has been destructed here. If there is storage (retain) of objects created by Trader (WTSOrderInfo, etc.) in the system, an out-of-bounds access problem will occur.
		 *	If thread_local is removed here and changed to pure static, there may be some problems in multi-threaded concurrent scenarios.
		 *	In short, if you want to be completely safe, you may need to add a lock, but this will bring performance overhead.
		 *	So comment it out, if there is a problem, you can refer to it
		 */
		thread_local static MyPool		pool;
		thread_local static SpinMutex	mtx;

		mtx.lock();
		T* ret = pool.construct();
		mtx.unlock();
		ret->_pool = &pool;
		ret->_mutex = &mtx;
		return ret;
	}

public:
	virtual void release() override
	{
		if (m_uRefs == 0)
			return;

		try
		{
			uint32_t cnt = m_uRefs.fetch_sub(1);
			if (cnt == 1)
			{
				_mutex->lock();
				_pool->destroy((T*)this);
				_mutex->unlock();
			}
		}
		catch (...)
		{

		}
	}
};
NS_WTP_END
