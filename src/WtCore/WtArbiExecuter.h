/*!
 * \file WtExecuter.h
 * \project	WonderTrader
 *
 * \author Wesley
 * \date 2020/03/30
 *
 * \brief
 */
#pragma once
#include "ITrdNotifySink.h"
#include "IExecCommand.h"
#include "WtExecuterFactory.h"
#include "../Includes/ExecuteDefs.h"
#include "../Share/threadpool.hpp"
#include "../Share/SpinMutex.hpp"

NS_WTP_BEGIN
class WTSVariant;
class IDataManager;
class TraderAdapter;
class IHotMgr;

//Local executor
class WtArbiExecuter : public ExecuteContext,
	public ITrdNotifySink, public IExecCommand
{
public:
	WtArbiExecuter(WtExecuterFactory* factory, const char* name, IDataManager* dataMgr);
	virtual ~WtArbiExecuter();

public:
	/*
	 *	Initialize executer
	 *	Pass in initialization parameters
	 */
	bool init(WTSVariant* params);

	void setTrader(TraderAdapter* adapter);

private:
	ExecuteUnitPtr	getUnit(const char* code, bool bAutoCreate = true);

public:
	//////////////////////////////////////////////////////////////////////////
	//ExecuteContext
	virtual WTSTickSlice*	getTicks(const char* code, uint32_t count, uint64_t etime = 0) override;

	virtual WTSTickData*	grabLastTick(const char* code) override;

	virtual double		getPosition(const char* stdCode, bool validOnly = true, int32_t flag = 3) override;
	virtual OrderMap*	getOrders(const char* code) override;
	virtual double		getUndoneQty(const char* code) override;

	virtual OrderIDs	buy(const char* code, double price, double qty, bool bForceClose = false) override;
	virtual OrderIDs	sell(const char* code, double price, double qty, bool bForceClose = false) override;
	virtual bool		cancel(uint32_t localid) override;
	virtual OrderIDs	cancel(const char* code, bool isBuy, double qty) override;
	virtual void		writeLog(const char* message) override;

	virtual WTSCommodityInfo*	getCommodityInfo(const char* stdCode) override;
	virtual WTSSessionInfo*		getSessionInfo(const char* stdCode) override;

	virtual uint64_t	getCurTime() override;

public:
	/*
	 *	Set target position
	 */
	virtual void set_position(const wt_hashmap<std::string, double>& targets) override;


	/*
	 *	Contract position change
	 */
	virtual void on_position_changed(const char* stdCode, double diffPos) override;

	/*
	 *	Real-time market callback
	 */
	virtual void on_tick(const char* stdCode, WTSTickData* newTick) override;

	/*
	 *	Transaction report
	 */
	virtual void on_trade(uint32_t localid, const char* stdCode, bool isBuy, double vol, double price) override;

	/*
	 *	Order report
	 */
	virtual void on_order(uint32_t localid, const char* stdCode, bool isBuy, double totalQty, double leftQty, double price, bool isCanceled = false) override;

	/*
	 *
	 */
	virtual void on_position(const char* stdCode, bool isLong, double prevol, double preavail, double newvol, double newavail, uint32_t tradingday) override;

	/*
	 *
	 */
	virtual void on_entrust(uint32_t localid, const char* stdCode, bool bSuccess, const char* message) override;

	/*
	 *	Trading channel ready
	 */
	virtual void on_channel_ready() override;

	/*
	 *	Trading channel lost
	 */
	virtual void on_channel_lost() override;

	/*
	 *	Capital report
	 */
	virtual void on_account(const char* currency, double prebalance, double balance, double dynbalance, 
		double avaliable, double closeprofit, double dynprofit, double margin, double fee, double deposit, double withdraw) override;

private:
	ExecuteUnitMap		_unit_map;
	TraderAdapter*		_trader;
	WtExecuterFactory*	_factory;
	IDataManager*		_data_mgr;
	WTSVariant*			_config;

	double				_scale;				//Amplification factor
	bool				_auto_clear;		//Whether to automatically clear the main contract position of the previous period
	bool				_strict_sync;		//Whether to strictly synchronize the target position
	bool				_channel_ready;

	SpinMutex			_mtx_units;

	typedef struct _CodeGroup
	{
		char	_name[32] = { 0 };
		wt_hashmap<std::string, double>	_items;
	} CodeGroup;
	typedef std::shared_ptr<CodeGroup> CodeGroupPtr;
	typedef wt_hashmap<std::string, CodeGroupPtr>	CodeGroups;
	CodeGroups				_groups;			//Contract combination (mapping from combination name to combination)
	CodeGroups				_code_to_groups;	//Mapping from contract code to combination

	wt_hashset<std::string>	_clear_includes;	//Automatically clear included varieties
	wt_hashset<std::string>	_clear_excludes;	//Automatically clear excluded varieties

	wt_hashset<std::string> _channel_holds;		//Channel holdings

	wt_hashmap<std::string, double> _target_pos;

	typedef std::shared_ptr<boost::threadpool::pool> ThreadPoolPtr;
	ThreadPoolPtr		_pool;
};

typedef std::shared_ptr<IExecCommand> ExecCmdPtr;

NS_WTP_END
