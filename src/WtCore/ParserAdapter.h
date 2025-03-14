/*!
 * \file ParserAdapter.h
 * \project	WonderTrader
 *
 * \author Wesley
 * \date 2020/03/30
 * 
 * \brief 
 */
#pragma once
#include <memory>
#include <boost/core/noncopyable.hpp>

#include "../Includes/FasterDefs.h"
#include "../Includes/IParserApi.h"


NS_WTP_BEGIN
class WTSVariant;
class IHotMgr;

class IParserStub
{
public:
	virtual void			handle_push_quote(WTSTickData* curTick){}

	virtual void			handle_push_order_detail(WTSOrdDtlData* curOrdDtl){}
	virtual void			handle_push_order_queue(WTSOrdQueData* curOrdQue) {}
	virtual void			handle_push_transaction(WTSTransData* curTrans) {}
};

class ParserAdapter : public IParserSpi,
					private boost::noncopyable
{
public:
	ParserAdapter();
	~ParserAdapter();

public:
	bool	init(const char* id, WTSVariant* cfg, IParserStub* stub, IBaseDataMgr* bgMgr, IHotMgr* hotMgr = NULL);

	bool	initExt(const char* id, IParserApi* api, IParserStub* stub, IBaseDataMgr* bgMgr, IHotMgr* hotMgr = NULL);

	void	release();

	bool	run();

	const char* id() const{ return _id.c_str(); }

public:
	virtual void handleSymbolList(const WTSArray* aySymbols) override {}

	/*
	 *	Process real-time market data
	 *	@quote		Real-time market data
	 *	@bNeedSlice	Whether slicing is required. If it is a snapshot market data accessed from the outside, slicing is required. If it is an internal broadcast, slicing is not required.
	 */
	virtual void handleQuote(WTSTickData *quote, uint32_t procFlag) override;

	/*
	 *	Process order queue data (stock level2)
	 *	@ordQueData	Order queue data
	 */
	virtual void handleOrderQueue(WTSOrdQueData* ordQueData) override;

	/*
	 *	Process tick-by-tick order data (stock level2)
	 *	@ordDetailData	Tick-by-tick order data
	 */
	virtual void handleOrderDetail(WTSOrdDtlData* ordDetailData) override;

	/*
		*	Process tick-by-tick transaction data
		*	@transData	Tick-by-tick transaction data
		*/
	virtual void handleTransaction(WTSTransData* transData) override;

	virtual void handleParserLog(WTSLogLevel ll, const char* message) override;

	virtual IBaseDataMgr* getBaseDataMgr() override { return _bd_mgr; }


private:
	IParserApi*			_parser_api;
	FuncDeleteParser	_remover;

	bool				_stopped;

	/*
	 *	Check time settings
	 *	If true, the time check is performed when the market data is received
	 *	Mainly applicable to direct access from the market data source
	 *	Because direct access from the market data source is likely to have incorrect timestamp data coming in
	 *	This option defaults to false
	 */
	bool				_check_time;

	typedef wt_hashset<std::string>	ExchgFilter;
	ExchgFilter			_exchg_filter;
	ExchgFilter			_code_filter;
	IBaseDataMgr*		_bd_mgr;
	IHotMgr*			_hot_mgr;
	IParserStub*		_stub;
	WTSVariant*			_cfg;
	std::string			_id;
};

typedef std::shared_ptr<ParserAdapter>	ParserAdapterPtr;
typedef wt_hashmap<std::string, ParserAdapterPtr>	ParserAdapterMap;

class ParserAdapterMgr : private boost::noncopyable
{
public:
	void	release();

	void	run();

	ParserAdapterPtr getAdapter(const char* id);

	bool	addAdapter(const char* id, ParserAdapterPtr& adapter);


public:
	ParserAdapterMap _adapters;
};

NS_WTP_END