#pragma once
#include "WtEngine.h"
#include "WtExecMgr.h"

#include "../Includes/FasterDefs.h"
#include "../Includes/ISelStraCtx.h"

#include <memory>

NS_WTP_BEGIN

typedef enum tagTaskPeriodType
{
	TPT_None,			//Not repeat
	TPT_Minute = 4,		//Minute cycle
	TPT_Daily = 8,		//Every trading day
	TPT_Weekly,			//Weekly, postponed in case of holidays
	TPT_Monthly,		//Monthly, postponed in case of holidays
	TPT_Yearly			//Yearly, postponed in case of holidays
}TaskPeriodType;

typedef struct _TaskInfo
{
	uint32_t	_id;
	char		_name[16];		//Task name
	char		_trdtpl[16];	//Trading day template
	char		_session[16];	//Trading time template
	uint32_t	_day;			//Date, changes according to the cycle, 0 for daily, 0~6 for weekly, corresponding to Sunday to Saturday, 1~31 for monthly, 0101~1231 for yearly
	uint32_t	_time;			//Time, accurate to the minute
	bool		_strict_time;	//Whether it is strict time, strict time means that it will only be executed when the time is equal, not strict time, then greater than or equal to the trigger time will be executed

	uint64_t	_last_exe_time;	//Last execution time, mainly to prevent repeated execution

	TaskPeriodType	_period;	//Task cycle
} TaskInfo;

typedef std::shared_ptr<TaskInfo> TaskInfoPtr;

typedef std::shared_ptr<ISelStraCtx> SelContextPtr;
class WtSelRtTicker;


class WtSelEngine : public WtEngine, public IExecuterStub
{
public:
	WtSelEngine();
	~WtSelEngine();

public:
	//////////////////////////////////////////////////////////////////////////
	//WtEngine接口
	virtual void init(WTSVariant* cfg, IBaseDataMgr* bdMgr, WtDtMgr* dataMgr, IHotMgr* hotMgr, EventNotifier* notifier) override;

	virtual void run() override;

	virtual void on_tick(const char* stdCode, WTSTickData* curTick) override;

	virtual void on_bar(const char* stdCode, const char* period, uint32_t times, WTSBarStruct* newBar) override;

	virtual void handle_push_quote(WTSTickData* newTick) override;

	virtual void on_init() override;

	virtual void on_session_begin() override;

	virtual void on_session_end() override;

	///////////////////////////////////////////////////////////////////////////
	//IExecuterStub 接口
	virtual uint64_t get_real_time() override;
	virtual WTSCommodityInfo* get_comm_info(const char* stdCode) override;
	virtual WTSSessionInfo* get_sess_info(const char* stdCode) override;
	virtual IHotMgr* get_hot_mon() { return _hot_mgr; }
	virtual uint32_t get_trading_day() { return _cur_tdate; }

public:
	//uint32_t	register_task(const char* name, uint32_t date, uint32_t time, TaskPeriodType period, bool bStrict = true, const char* trdtpl = "CHINA");
	void			addContext(SelContextPtr ctx, uint32_t date, uint32_t time, TaskPeriodType period, bool bStrict = true, const char* trdtpl = "CHINA", const char* sessionID="TRADING");

	SelContextPtr	getContext(uint32_t id);

	inline void addExecuter(ExecCmdPtr& executer)
	{
		_exec_mgr.add_executer(executer);
		executer->setStub(this);
	}

	void	on_minute_end(uint32_t uDate, uint32_t uTime);

	void	handle_pos_change(const char* straName, const char* stdCode, double diffQty);

private:
	wt_hashmap<uint32_t, TaskInfoPtr>	_tasks;

	typedef wt_hashmap<uint32_t, SelContextPtr> ContextMap;
	ContextMap		_ctx_map;

	WtExecuterMgr	_exec_mgr;

	bool	_terminated;

	WtSelRtTicker*	_tm_ticker;
	WTSVariant*		_cfg;
};

NS_WTP_END
