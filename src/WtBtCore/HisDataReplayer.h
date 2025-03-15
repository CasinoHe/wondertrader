/*!
 * \file HisDataReplayer.h
 * \project	WonderTrader
 *
 * \author Wesley
 * \date 2020/03/30
 * 
 * \brief 
 */
#pragma once
#include <string>
#include <set>
#include "HisDataMgr.h"
#include "../WtDataStorage/DataDefine.h"

#include "../Includes/FasterDefs.h"
#include "../Includes/WTSMarcos.h"
#include "../Includes/WTSTypes.h"

#include "../WTSTools/WTSHotMgr.h"
#include "../WTSTools/WTSBaseDataMgr.h"

NS_WTP_BEGIN
class WTSTickData;
class WTSVariant;
class WTSKlineSlice;
class WTSTickSlice;
class WTSOrdDtlSlice;
class WTSOrdQueSlice;
class WTSTransSlice;
class WTSSessionInfo;
class WTSCommodityInfo;

class WTSOrdDtlData;
class WTSOrdQueData;
class WTSTransData;

class EventNotifier;
NS_WTP_END

USING_NS_WTP;

class IDataSink
{
public:
	virtual void	handle_tick(const char* stdCode, WTSTickData* curTick, uint32_t pxType) = 0;
	virtual void	handle_order_queue(const char* stdCode, WTSOrdQueData* curOrdQue) {};
	virtual void	handle_order_detail(const char* stdCode, WTSOrdDtlData* curOrdDtl) {};
	virtual void	handle_transaction(const char* stdCode, WTSTransData* curTrans) {};
	virtual void	handle_bar_close(const char* stdCode, const char* period, uint32_t times, WTSBarStruct* newBar) = 0;
	virtual void	handle_schedule(uint32_t uDate, uint32_t uTime) = 0;

	virtual void	handle_init() = 0;
	virtual void	handle_session_begin(uint32_t curTDate) = 0;
	virtual void	handle_session_end(uint32_t curTDate) = 0;
	virtual void	handle_replay_done() {}

	virtual void	handle_section_end(uint32_t curTDate, uint32_t curTime) {}
};

/*
 *	Historical data loader callback function
 *	@obj	For returning, just return as is
 *	@bars	K-line data
 *	@count	Number of K-lines
 */
typedef void(*FuncReadBars)(void* obj, WTSBarStruct* firstBar, uint32_t count);

/*
 *	Load ex-right factor callback
 *	@obj	For returning, just return as is
 *	@stdCode	Instrument code
 *	@dates
 */
typedef void(*FuncReadFactors)(void* obj, const char* stdCode, uint32_t* dates, double* factors, uint32_t count);

/*
 *	Load tick data callback
 *	@firstItem	Data
 *	@count		Number of items
 */
typedef void(*FuncReadTicks)(void* obj, WTSTickStruct* firstItem, uint32_t count);

/*
 *	Load order detail data callback
 *	@firstItem	Data
 *	@count		Number of items
 */
typedef void(*FuncReadOrdDtl)(void* obj, WTSOrdDtlStruct* firstItem, uint32_t count);

/*
 *	Load order queue data callback
 *	@firstItem	Data
 *	@count		Number of items
 */
typedef void(*FuncReadOrdQue)(void* obj, WTSOrdQueStruct* firstItem, uint32_t count);

/*
 *	Load transaction data callback
 *	@firstItem	Data
 *	@count		Number of items
 */
typedef void(*FuncReadTrans)(void* obj, WTSTransStruct* firstItem, uint32_t count);

class IBtDataLoader
{
public:
	/*
	 *	Load final historical K-line data
	 *	The difference from loadRawHisBars is that loadFinalHisBars is considered by the system as the final required data and will not be processed, such as ex-right data and main contract data.
	 *	loadRawHisBars is the interface to load unprocessed raw data
	 *
	 *	@obj	For returning, just return as is
	 *	@stdCode	Instrument code
	 *	@period	K-line period
	 *	@cb		Callback function
	 */
	virtual bool loadFinalHisBars(void* obj, const char* stdCode, WTSKlinePeriod period, FuncReadBars cb) = 0;

	/*
	 *	Load raw historical K-line data
	 *
	 *	@obj	For returning, just return as is
	 *	@stdCode	Instrument code
	 *	@period	K-line period
	 *	@cb		Callback function
	 */
	virtual bool loadRawHisBars(void* obj, const char* stdCode, WTSKlinePeriod period, FuncReadBars cb) = 0;

	/*
	 *	Load all ex-right factors
	 */
	virtual bool loadAllAdjFactors(void* obj, FuncReadFactors cb) = 0;

	/*
	 *	Load ex-right factors by instrument
	 *
	 *	@stdCode	Instrument code
	 */
	virtual bool loadAdjFactors(void* obj, const char* stdCode, FuncReadFactors cb) = 0;

	/*
	 *	Load historical Tick data
	 */
	virtual bool loadRawHisTicks(void* obj, const char* stdCode, uint32_t uDate, FuncReadTicks cb) = 0;

	/*
	 *	Whether to automatically trans to dsb
	 */
	virtual bool isAutoTrans() { return true; }
};

class HisDataReplayer
{

private:
	template <typename T>
	class HftDataList
	{
	public:
		std::string		_code;
		uint32_t		_date;
		/*
		 * By Wesley @ 2022.03.21
		 * Cursor, used to mark the position of the next data, or the number of items that have been replayed
		 * When uninitialized, the cursor is UINT_MAX. Once initialized, the cursor must be greater than 0
		 */
		std::size_t		_cursor;
		std::size_t		_count;

		std::vector<T> _items;

		HftDataList() :_cursor(UINT_MAX), _count(0), _date(0){}
	};

	typedef wt_hashmap<std::string, HftDataList<WTSTickStruct>>		TickCache;
	typedef wt_hashmap<std::string, HftDataList<WTSOrdDtlStruct>>	OrdDtlCache;
	typedef wt_hashmap<std::string, HftDataList<WTSOrdQueStruct>>	OrdQueCache;
	typedef wt_hashmap<std::string, HftDataList<WTSTransStruct>>	TransCache;


	typedef struct _BarsList
	{
		std::string		_code;
		WTSKlinePeriod	_period;
		/*
		 * By Wesley @ 2022.03.21
		 * Cursor, used to mark the position of the next data, or the number of items that have been replayed
		 * When uninitialized, the cursor is UINT_MAX. Once initialized, the cursor must be greater than 0
		 */
		uint32_t		_cursor;
		uint32_t		_count;
		uint32_t		_times;

		std::vector<WTSBarStruct>	_bars;
		double			_factor;	//最后一条复权因子

		uint32_t		_untouch_days;	//未用到的天数

		inline void mark()
		{
			_untouch_days = 0;
		}

		inline std::size_t size()
		{
			return sizeof(WTSBarStruct)*_bars.size();
		}

		_BarsList() :_cursor(UINT_MAX), _count(0), _times(1), _factor(1), _untouch_days(0){}
	} BarsList;

	/*
	 *	By Wesley @ 2022.03.13
	 *	Here the cache is changed to a smart pointer
	 *	Because some users found that when getting K-lines that were not subscribed in oninit during oncalc
	 *	Because the reference of BarList is used, after the map of the K-line cache reinserts a new K-line
	 *	The place where the reference is invalid will refer to the wrong address
	 *	I suspect that the data may have been copied again here
	 *	Changing this to a smart pointer can avoid this problem, because no matter how the map's own memory is organized
	 *	The address pointed to by the smart pointer will not change
	 */
	typedef std::shared_ptr<BarsList> BarsListPtr;
	typedef wt_hashmap<std::string, BarsListPtr>	BarsCache;

	typedef enum tagTaskPeriodType
	{
		TPT_None,		//不重复
		TPT_Minute = 4,	//分钟线周期
		TPT_Daily = 8,	//每个交易日
		TPT_Weekly,		//每周,遇到节假日的话要顺延
		TPT_Monthly,	//每月,遇到节假日顺延
		TPT_Yearly		//每年,遇到节假日顺延
	}TaskPeriodType;

	typedef struct _TaskInfo
	{
		uint32_t	_id;
		char		_name[16];		//任务名
		char		_trdtpl[16];	//交易日模板
		char		_session[16];	//交易时间模板
		uint32_t	_day;			//日期,根据周期变化,每日为0,每周为0~6,对应周日到周六,每月为1~31,每年为0101~1231
		uint32_t	_time;			//时间,精确到分钟
		bool		_strict_time;	//是否是严格时间,严格时间即只有时间相等才会执行,不是严格时间,则大于等于触发时间都会执行

		uint64_t	_last_exe_time;	//上次执行时间,主要为了防止重复执行

		TaskPeriodType	_period;	//任务周期
	} TaskInfo;

	typedef std::shared_ptr<TaskInfo> TaskInfoPtr;



public:
	HisDataReplayer();
	~HisDataReplayer();

private:
	/*
	 *	Cache historical data from custom data files
	 */
	bool		cacheRawBarsFromBin(const std::string& key, const char* stdCode, WTSKlinePeriod period, bool bForBars = true);

	/*
	 *	Cache historical data from csv files
	 */
	bool		cacheRawBarsFromCSV(const std::string& key, const char* stdCode, WTSKlinePeriod period, bool bSubbed = true);

	/*
	 *	Cache historical tick data from custom data files
	 */
	bool		cacheRawTicksFromBin(const std::string& key, const char* stdCode, uint32_t uDate);

	/*
	 *	Cache historical order detail data from custom data files
	 */
	bool		cacheRawOrdDtlFromBin(const std::string& key, const char* stdCode, uint32_t uDate);

	/*
	 *	Cache historical order queue data from custom data files
	 */
	bool		cacheRawOrdQueFromBin(const std::string& key, const char* stdCode, uint32_t uDate);

	/*
	 *	Cache historical transaction data from custom data files
	 */
	bool		cacheRawTransFromBin(const std::string& key, const char* stdCode, uint32_t uDate);

	/*
	 *	Cache historical tick data from csv files
	 */
	bool		cacheRawTicksFromCSV(const std::string& key, const char* stdCode, uint32_t uDate);

	/*
	 *	Cache historical data from external loader
	 */
	bool		cacheFinalBarsFromLoader(const std::string& key, const char* stdCode, WTSKlinePeriod period, bool bSubbed = true);

	/*
	 *	Cache historical tick data from external loader
	 */
	bool		cacheRawTicksFromLoader(const std::string& key, const char* stdCode, uint32_t uDate);

	/*
	 *	Cache integrated futures contract historical K-line (for .HOT//2ND)
	 */
	bool		cacheIntegratedFutBarsFromBin(void* codeInfo, const std::string& key, const char* stdCode, WTSKlinePeriod period, bool bSubbed = true);

	/*
	 *	Cache adjusted stock K-line data
	 */
	bool		cacheAdjustedStkBarsFromBin(void* codeInfo, const std::string& key, const char* stdCode, WTSKlinePeriod period, bool bSubbed = true);

	void		onMinuteEnd(uint32_t uDate, uint32_t uTime, uint32_t endTDate = 0, bool tickSimulated = true);

	void		loadFees(const char* filename);

	bool		replayHftDatas(uint64_t stime, uint64_t etime);

	uint64_t	replayHftDatasByDay(uint32_t curTDate);

	void		simTickWithUnsubBars(uint64_t stime, uint64_t etime, uint32_t endTDate = 0, int pxType = 0);

	void		simTicks(uint32_t uDate, uint32_t uTime, uint32_t endTDate = 0, int pxType = 0);

	inline bool		checkTicks(const char* stdCode, uint32_t uDate);

	inline bool		checkOrderDetails(const char* stdCode, uint32_t uDate);

	inline bool		checkOrderQueues(const char* stdCode, uint32_t uDate);

	inline bool		checkTransactions(const char* stdCode, uint32_t uDate);

	void		checkUnbars();

	bool		loadStkAdjFactorsFromFile(const char* adjfile);

	bool		loadStkAdjFactorsFromLoader();

	bool		checkAllTicks(uint32_t uDate);

	inline	uint64_t	getNextTickTime(uint32_t curTDate, uint64_t stime = UINT64_MAX);
	inline	uint64_t	getNextOrdQueTime(uint32_t curTDate, uint64_t stime = UINT64_MAX);
	inline	uint64_t	getNextOrdDtlTime(uint32_t curTDate, uint64_t stime = UINT64_MAX);
	inline	uint64_t	getNextTransTime(uint32_t curTDate, uint64_t stime = UINT64_MAX);

	void		reset();


	void		dump_btstate(const char* stdCode, WTSKlinePeriod period, uint32_t times, uint64_t stime, uint64_t etime, double progress, int64_t elapse);
	void		notify_state(const char* stdCode, WTSKlinePeriod period, uint32_t times, uint64_t stime, uint64_t etime, double progress);

	uint32_t	locate_barindex(const std::string& key, uint64_t curTime, bool bUpperBound = false);

	/*
	 *	Run backtest by K-line
	 *
	 *	@bNeedDump	Whether to dump the backtest progress to a file
	 */
	void	run_by_bars(bool bNeedDump = false);

	/*
	 *	Run backtest by scheduled tasks
	 *
	 *	@bNeedDump	Whether to dump the backtest progress to a file
	 */
	void	run_by_tasks(bool bNeedDump = false);

	/*
	 *	Run backtest by tick
	 *
	 *	@bNeedDump	Whether to dump the backtest progress to a file
	 */
	void	run_by_ticks(bool bNeedDump = false);

	void	check_cache_days();

public:
	bool init(WTSVariant* cfg, EventNotifier* notifier = NULL, IBtDataLoader* dataLoader = NULL);

	bool prepare();

	/*
	 *	Run backtest
	 *
	 *	@bNeedDump	Whether to dump the backtest progress to a file
	 */
	void run(bool bNeedDump = false);
	
	void stop();

	void clear_cache();

	inline void set_time_range(uint64_t stime, uint64_t etime)
	{
		_begin_time = stime;
		_end_time = etime;
	}

	inline void enable_tick(bool bEnabled = true)
	{
		_tick_enabled = bEnabled;
	}

	inline void register_sink(IDataSink* listener, const char* sinkName) 
	{
		_listener = listener; 
		_stra_name = sinkName;
	}

	/*
	 *	Register task
	 *	@date Date, changes according to the period, daily is 0, weekly is 0~6, corresponding to Sunday to Saturday, monthly is 1~31, yearly is 0101~1231
	 *	@time Time, accurate to the minute
	 *	@period	Time period, can be minute, day, week, month, year
	 */
	void register_task(uint32_t taskid, uint32_t date, uint32_t time, const char* period, const char* trdtpl = "CHINA", const char* session = "TRADING");

	WTSKlineSlice* get_kline_slice(const char* stdCode, const char* period, uint32_t count, uint32_t times = 1, bool isMain = false);

	WTSTickSlice* get_tick_slice(const char* stdCode, uint32_t count, uint64_t etime = 0);

	WTSOrdDtlSlice* get_order_detail_slice(const char* stdCode, uint32_t count, uint64_t etime = 0);

	WTSOrdQueSlice* get_order_queue_slice(const char* stdCode, uint32_t count, uint64_t etime = 0);

	WTSTransSlice* get_transaction_slice(const char* stdCode, uint32_t count, uint64_t etime = 0);

	WTSTickData* get_last_tick(const char* stdCode);

	uint32_t get_date() const{ return _cur_date; }
	uint32_t get_min_time() const{ return _cur_time; }
	uint32_t get_raw_time() const{ return _cur_time; }
	uint32_t get_secs() const{ return _cur_secs; }
	uint32_t get_trading_date() const{ return _cur_tdate; }

	double calc_fee(const char* stdCode, double price, double qty, uint32_t offset);
	WTSSessionInfo*		get_session_info(const char* sid, bool isCode = false);
	WTSCommodityInfo*	get_commodity_info(const char* stdCode);
	double get_cur_price(const char* stdCode);
	double get_day_price(const char* stdCode, int flag = 0);

	std::string get_rawcode(const char* stdCode);

	void sub_tick(uint32_t sid, const char* stdCode);
	void sub_order_queue(uint32_t sid, const char* stdCode);
	void sub_order_detail(uint32_t sid, const char* stdCode);
	void sub_transaction(uint32_t sid, const char* stdCode);

	inline bool	is_tick_enabled() const{ return _tick_enabled; }

	inline bool	is_tick_simulated() const { return _tick_simulated; }

	inline void update_price(const char* stdCode, double price)
	{
		_price_map[stdCode] = price;
	}

	inline IHotMgr*	get_hot_mgr() { return &_hot_mgr; }

private:
	IDataSink*		_listener;
	IBtDataLoader*	_bt_loader;
	std::string		_stra_name;

	TickCache		_ticks_cache;	//tick缓存
	OrdDtlCache		_orddtl_cache;	//order detail缓存
	OrdQueCache		_ordque_cache;	//order queue缓存
	TransCache		_trans_cache;	//transaction缓存

	BarsCache		_bars_cache;	//K线缓存
	BarsCache		_unbars_cache;	//未订阅的K线缓存
	wt_hashset<std::string> _codes_in_subbed;
	wt_hashset<std::string> _codes_in_unsubbed;

	TaskInfoPtr		_task;

	std::string		_main_key;
	std::string		_min_period;	//最小K线周期,这个主要用于未订阅品种的信号处理上
	std::string		_main_period;	//主周期
	bool			_tick_enabled;	//是否开启了tick回测
	bool			_tick_simulated;	//是否需要模拟tick
	bool			_align_by_section;	//重采样分钟线是否按小节对齐
	
	/*
	 *	By Wesley @ 2023.05.05
	 *	If the K-line has no volume, do not simulate tick
	 *	The default is false, mainly for limit-up and limit-down markets, and also for inactive contracts
	 */
	bool			_nosim_if_notrade;
	std::map<std::string, WTSTickStruct>	_day_cache;	//每日Tick缓存,当tick回放未开放时,会用到该缓存
	std::map<std::string, std::string>		_ticker_keys;

	//By Wesley @ 2022.06.01
	//This is mainly for scenarios where orders are placed for contracts that are not subscribed directly
	wt_hashset<std::string>		_unsubbed_in_need;	//K-lines that are not subscribed but needed

	//By Wesley @ 2022.08.15
	//Ex-right flag, expressed by bit operation, 1|2|4, 1 means volume ex-right, 2 means turnover ex-right, 4 means total holding ex-right, others to be determined
	uint32_t		_adjust_flag; 

	uint32_t		_cur_date;
	uint32_t		_cur_time;
	uint32_t		_cur_secs;
	uint32_t		_cur_tdate;
	uint32_t		_closed_tdate;
	uint32_t		_opened_tdate;

	WTSBaseDataMgr	_bd_mgr;
	WTSHotMgr		_hot_mgr;

	std::string		_base_dir;
	std::string		_mode;
	uint64_t		_begin_time;
	uint64_t		_end_time;

	//缓存自动清理天数
	uint32_t		_cache_clear_days;

	bool			_running;
	bool			_terminated;
	//////////////////////////////////////////////////////////////////////////
	//手续费模板
	typedef struct _FeeItem
	{
		double	_open;
		double	_close;
		double	_close_today;
		bool	_by_volume;

		_FeeItem()
		{
			memset(this, 0, sizeof(_FeeItem));
		}
	} FeeItem;
	typedef wt_hashmap<std::string, FeeItem>	FeeMap;
	FeeMap		_fee_map;

	//////////////////////////////////////////////////////////////////////////
	//
	typedef wt_hashmap<std::string, double> PriceMap;
	PriceMap		_price_map;

	//////////////////////////////////////////////////////////////////////////
	//
	//By Wesley @ 2022.02.07
	//tick data subscription item, first is contextid, second is subscription option, 0-original subscription, 1-forward ex-right, 2-backward ex-right
	typedef std::pair<uint32_t, uint32_t> SubOpt;
	typedef wt_hashmap<uint32_t, SubOpt> SubList;
	typedef wt_hashmap<std::string, SubList>	StraSubMap;
	StraSubMap		_tick_sub_map;		//tick数据订阅表
	StraSubMap		_ordque_sub_map;	//orderqueue数据订阅表
	StraSubMap		_orddtl_sub_map;	//orderdetail数据订阅表
	StraSubMap		_trans_sub_map;		//transaction数据订阅表

	//除权因子
	typedef struct _AdjFactor
	{
		uint32_t	_date;
		double		_factor;
	} AdjFactor;
	typedef std::vector<AdjFactor> AdjFactorList;
	typedef wt_hashmap<std::string, AdjFactorList>	AdjFactorMap;
	AdjFactorMap	_adj_factors;

	const AdjFactorList& getAdjFactors(const char* code, const char* exchg, const char* pid);

	EventNotifier*	_notifier;

	HisDataMgr		_his_dt_mgr;
};

