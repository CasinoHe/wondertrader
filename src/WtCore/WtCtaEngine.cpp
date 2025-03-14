/*!
 * \file WtCtaEngine.cpp
 * \project	WonderTrader
 *
 * \author Wesley
 * \date 2020/03/30
 * 
 * \brief 
 */
#define WIN32_LEAN_AND_MEAN

#include "WtCtaEngine.h"
#include "WtDtMgr.h"
#include "WtCtaTicker.h"
#include "WtHelper.h"
#include "TraderAdapter.h"
#include "EventNotifier.h"

#include "../Share/CodeHelper.hpp"
#include "../Includes/WTSVariant.hpp"
#include "../Share/TimeUtils.hpp"
#include "../Includes/IBaseDataMgr.h"
#include "../Includes/IHotMgr.h"
#include "../Includes/WTSContractInfo.hpp"
#include "../Includes/WTSRiskDef.hpp"
#include "../Share/decimal.h"

#include "../WTSTools/WTSLogger.h"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
namespace rj = rapidjson;

WtCtaEngine::WtCtaEngine()
	: _tm_ticker(NULL)
{
	
}


WtCtaEngine::~WtCtaEngine()
{
	if (_tm_ticker)
	{
		delete _tm_ticker;
		_tm_ticker = NULL;
	}

	if (_cfg)
		_cfg->release();
}

void WtCtaEngine::run()
{
	_tm_ticker = new WtCtaRtTicker(this);
	WTSVariant* cfgProd = _cfg->get("product");
	_tm_ticker->init(_data_mgr->reader(), cfgProd->getCString("session"));

	//Start before, first put the running strategy on the ground
	{
		rj::Document root(rj::kObjectType);
		rj::Document::AllocatorType &allocator = root.GetAllocator();

		rj::Value jStraList(rj::kArrayType);
		for (auto& m : _ctx_map)
		{
			const CtaContextPtr& ctx = m.second;
			jStraList.PushBack(rj::Value(ctx->name(), allocator), allocator);
		}

		root.AddMember("marks", jStraList, allocator);

		rj::Value jChnlList(rj::kArrayType);
		for (auto& m : _adapter_mgr->getAdapters())
		{
			const TraderAdapterPtr& adapter = m.second;
			jChnlList.PushBack(rj::Value(adapter->id(), allocator), allocator);
		}

		root.AddMember("channels", jChnlList, allocator);

		rj::Value jExecList(rj::kArrayType);
		_exec_mgr.enum_executer([&jExecList, &allocator](ExecCmdPtr executer) {
			if(executer)
				jExecList.PushBack(rj::Value(executer->name(), allocator), allocator);
		});

		root.AddMember("executers", jExecList, allocator);

		root.AddMember("engine", rj::Value("CTA", allocator), allocator);

		std::string filename = WtHelper::getBaseDir();
		filename += "marker.json";

		rj::StringBuffer sb;
		rj::PrettyWriter<rj::StringBuffer> writer(sb);
		root.Accept(writer);
		StdFile::write_file_content(filename.c_str(), sb.GetString());
	}

	_tm_ticker->run();

	if (_risk_mon)
		_risk_mon->self()->run();

}

void WtCtaEngine::init(WTSVariant* cfg, IBaseDataMgr* bdMgr, WtDtMgr* dataMgr, IHotMgr* hotMgr, EventNotifier* notifier /* = NULL */)
{
	WtEngine::init(cfg, bdMgr, dataMgr, hotMgr, notifier);

	_cfg = cfg;
	_cfg->retain();

	_exec_mgr.set_filter_mgr(&_filter_mgr);

	uint32_t poolsize = cfg->getUInt32("poolsize");
	if (poolsize > 0)
	{
		_pool.reset(new boost::threadpool::pool(poolsize));
	}
	WTSLogger::info("Engine task poolsize is {}", poolsize);
}

void WtCtaEngine::addContext(CtaContextPtr ctx)
{
	uint32_t sid = ctx->id();
	_ctx_map[sid] = ctx;
}

CtaContextPtr WtCtaEngine::getContext(uint32_t id)
{
	auto it = _ctx_map.find(id);
	if (it == _ctx_map.end())
		return CtaContextPtr();

	return it->second;
}

void WtCtaEngine::on_init()
{
	//wt_hashmap<std::string, double> target_pos;
	_exec_mgr.clear_cached_targets();
	for (auto it = _ctx_map.begin(); it != _ctx_map.end(); it++)
	{
		CtaContextPtr& ctx = (CtaContextPtr&)it->second;
		ctx->on_init();

		const auto& exec_ids = _exec_mgr.get_route(ctx->name());

		ctx->enum_position([this, ctx, exec_ids](const char* stdCode, double qty){

			double oldQty = qty;
			bool bFilterd = _filter_mgr.is_filtered_by_strategy(ctx->name(), qty);
			if (!bFilterd)
			{
				if (!decimal::eq(qty, oldQty))
				{
					 //Output log
					WTSLogger::info("[Filters] Target position of {} of strategy {} reset by strategy filter: {} -> {}", 
						stdCode, ctx->name(), oldQty, qty);
				}

				std::string realCode = stdCode;
				CodeHelper::CodeInfo cInfo = CodeHelper::extractStdCode(stdCode, _hot_mgr);
				if(strlen(cInfo._ruletag) > 0)
				{
					std::string code = _hot_mgr->getCustomRawCode(cInfo._ruletag, cInfo.stdCommID(), _cur_tdate);
					realCode = CodeHelper::rawMonthCodeToStdCode(code.c_str(), cInfo._exchg);
				}

				for(auto& execid : exec_ids)
					_exec_mgr.add_target_to_cache(realCode.c_str(), qty, execid.c_str());
			}
			else
			{
				 //Output log
				WTSLogger::info("[Filters] Target position of {} of strategy {} ignored by strategy filter", stdCode, ctx->name());
			}
		}, true);
	}

	bool bRiskEnabled = false;
	if (!decimal::eq(_risk_volscale, 1.0) && _risk_date == _cur_tdate)
	{
		WTSLogger::log_by_cat("risk", LL_INFO, "Risk scale of portfolio is {:.2f}", _risk_volscale);
		bRiskEnabled = true;
	}

	////Initialize the position and print it out
	//for (auto it = target_pos.begin(); it != target_pos.end(); it++)
	//{
	//	const auto& stdCode = it->first;
	//	double& pos = (double&)it->second;

	//	if (bRiskEnabled && !decimal::eq(pos, 0))
	//	{
	//		double symbol = pos / abs(pos);
	//		pos = decimal::rnd(abs(pos)*_risk_volscale)*symbol;
	//	}

	//	WTSLogger::info("Portfolio initial position of {} is {}", stdCode.c_str(), pos);
	//}

	_exec_mgr.commit_cached_targets(bRiskEnabled?_risk_volscale:1.0);

	if (_evt_listener)
		_evt_listener->on_initialize_event();
}

void WtCtaEngine::on_session_begin()
{
	WTSLogger::info("Trading day {} begun", _cur_tdate);
	for (auto it = _ctx_map.begin(); it != _ctx_map.end(); it++)
	{
		CtaContextPtr& ctx = (CtaContextPtr&)it->second;
		ctx->on_session_begin(_cur_tdate);
	}

	if (_evt_listener)
		_evt_listener->on_session_event(_cur_tdate, true);

	_ready = true;
}

void WtCtaEngine::on_session_end()
{
	WtEngine::on_session_end();

	for (auto it = _ctx_map.begin(); it != _ctx_map.end(); it++)
	{
		CtaContextPtr& ctx = (CtaContextPtr&)it->second;
		ctx->on_session_end(_cur_tdate);
	}

	WTSLogger::info("Trading day {} ended", _cur_tdate);
	if (_evt_listener)
		_evt_listener->on_session_event(_cur_tdate, false);
}

void WtCtaEngine::on_schedule(uint32_t curDate, uint32_t curTime)
{
	//Go check the filter
	_filter_mgr.load_filters();
	_exec_mgr.clear_cached_targets();
	wt_hashmap<std::string, double> target_pos;
	if(_pool)
	{
		/*
		 *	By Wesley @ 2023.06.27
		 *	If concurrent through the thread pool
		 *	First concurrent all on_schedule
		 *	Then wait for all tasks to end
		 *	Finally, uniformly read all positions
		 */
		for (auto it = _ctx_map.begin(); it != _ctx_map.end(); it++)
		{
			CtaContextPtr& ctx = (CtaContextPtr&)it->second;
			_pool->schedule([ctx, curDate, curTime] (){
				ctx->on_schedule(curDate, curTime);
			});
		}

		/*
		 *	By Wesley @ 2023.06.27
		 *	Wait for all on_schedule to complete
		 */
		_pool->wait();
		
		for (auto it = _ctx_map.begin(); it != _ctx_map.end(); it++)
		{
			CtaContextPtr& ctx = (CtaContextPtr&)it->second;
			const auto& exec_ids = _exec_mgr.get_route(ctx->name());
			ctx->enum_position([this, ctx, exec_ids, &target_pos](const char* stdCode, double qty) {

				double oldQty = qty;
				bool bFilterd = _filter_mgr.is_filtered_by_strategy(ctx->name(), qty);
				if (!bFilterd)
				{
					if (!decimal::eq(qty, oldQty))
					{
						//Output log
						WTSLogger::info("[Filters] Target position of {} of strategy {} reset by strategy filter: {} -> {}",
							stdCode, ctx->name(), oldQty, qty);
					}

					std::string realCode = stdCode;
					CodeHelper::CodeInfo cInfo = CodeHelper::extractStdCode(stdCode, _hot_mgr);
					if (strlen(cInfo._ruletag) > 0)
					{
						std::string code = _hot_mgr->getCustomRawCode(cInfo._ruletag, cInfo.stdCommID(), _cur_tdate);
						realCode = CodeHelper::rawMonthCodeToStdCode(code.c_str(), cInfo._exchg);
					}

					double& vol = target_pos[realCode];
					vol += qty;
					for (auto& execid : exec_ids)
						_exec_mgr.add_target_to_cache(realCode.c_str(), qty, execid.c_str());
				}
				else
				{
					//Output log
					WTSLogger::info("[Filters] Target position of {} of strategy {} ignored by strategy filter", stdCode, ctx->name());
				}
			}, true);
		}
	}
	else
	{
		for (auto it = _ctx_map.begin(); it != _ctx_map.end(); it++)
		{
			CtaContextPtr& ctx = (CtaContextPtr&)it->second;
			ctx->on_schedule(curDate, curTime);
			const auto& exec_ids = _exec_mgr.get_route(ctx->name());
			ctx->enum_position([this, ctx, exec_ids, &target_pos](const char* stdCode, double qty) {

				double oldQty = qty;
				bool bFilterd = _filter_mgr.is_filtered_by_strategy(ctx->name(), qty);
				if (!bFilterd)
				{
					if (!decimal::eq(qty, oldQty))
					{
						//Output log
						WTSLogger::info("[Filters] Target position of {} of strategy {} reset by strategy filter: {} -> {}",
							stdCode, ctx->name(), oldQty, qty);
					}

					std::string realCode = stdCode;
					CodeHelper::CodeInfo cInfo = CodeHelper::extractStdCode(stdCode, _hot_mgr);
					if (strlen(cInfo._ruletag) > 0)
					{
						std::string code = _hot_mgr->getCustomRawCode(cInfo._ruletag, cInfo.stdCommID(), _cur_tdate);
						realCode = CodeHelper::rawMonthCodeToStdCode(code.c_str(), cInfo._exchg);
					}

					double& vol = target_pos[realCode];
					vol += qty;
					for (auto& execid : exec_ids)
						_exec_mgr.add_target_to_cache(realCode.c_str(), qty, execid.c_str());
				}
				else
				{
					//Output log
					WTSLogger::info("[Filters] Target position of {} of strategy {} ignored by strategy filter", stdCode, ctx->name());
				}
			}, true);
		}
	}
	

	bool bRiskEnabled = false;
	if(!decimal::eq(_risk_volscale, 1.0) && _risk_date == _cur_tdate)
	{
		WTSLogger::log_by_cat("risk", LL_INFO, "Risk scale of strategy group is {:.2f}", _risk_volscale);
		bRiskEnabled = true;
	}

	//Process portfolio theoretical positions
	for (auto it = target_pos.begin(); it != target_pos.end(); it++)
	{
		const auto& stdCode = it->first;
		double& pos = (double&)it->second;

		if (bRiskEnabled && !decimal::eq(pos, 0))
		{
			double symbol = pos / abs(pos);
			pos = decimal::rnd(abs(pos)*_risk_volscale)*symbol;
		}

		append_signal(stdCode.c_str(), pos, true);
	}

	for(auto& m : _pos_map)
	{
		const auto& stdCode = m.first;
		if (target_pos.find(stdCode) == target_pos.end())
		{
			if(!decimal::eq(m.second->_volume, 0))
			{
				//Here is to notify WtEngine to update the portfolio position data
				append_signal(stdCode.c_str(), 0, true);

				WTSLogger::error("Instrument {} not in target positions, setup to 0 automatically", stdCode.c_str());
			}

			//Because there will be expired contract codes in the portfolio position, a check must be done here before throwing it to execution
			auto cInfo = get_contract_info(stdCode.c_str());
			if (cInfo != NULL)
			{
				//target_pos[stdCode] = 0;
				_exec_mgr.add_target_to_cache(stdCode.c_str(), 0);
			}
		}
	}

	push_task([this](){
		update_fund_dynprofit();
		/*
		 *	By Wesley @ 2023.01.30
		 *	Add an entry to refresh the trading account funds regularly
		 */
		_adapter_mgr->refresh_funds();
	});

	//_exec_mgr.set_positions(target_pos);
	_exec_mgr.commit_cached_targets(bRiskEnabled ? _risk_volscale : 1);

	save_datas();

	if (_evt_listener)
		_evt_listener->on_schedule_event(curDate, curTime);
}


void WtCtaEngine::handle_push_quote(WTSTickData* newTick)
{
	if (_tm_ticker)
		_tm_ticker->on_tick(newTick);
}

void WtCtaEngine::handle_pos_change(const char* straName, const char* stdCode, double diffPos)
{
	//Here is the position increment, so there is no need to process the unfiltered situation, because in the incremental situation, the target diffQty will not be changed
	if(_filter_mgr.is_filtered_by_strategy(straName, diffPos, true))
	{
		//Output log
		WTSLogger::info("[Filters] Target position of {} of strategy {} ignored by strategy filter", stdCode, straName);
		return;
	}

	std::string realCode = stdCode;
	//const char* ruleTag = _hot_mgr->getRuleTag(stdCode);
	CodeHelper::CodeInfo cInfo = CodeHelper::extractStdCode(stdCode, _hot_mgr);
	if (strlen(cInfo._ruletag) > 0)
	{
		std::string code = _hot_mgr->getCustomRawCode(cInfo._ruletag, cInfo.stdCommID(), _cur_tdate);
		realCode = CodeHelper::rawMonthCodeToStdCode(code.c_str(), cInfo._exchg);
	}

	/*
	 *	Here you must calculate a total target position
	 */
	PosInfoPtr& pInfo = _pos_map[realCode];	
	if (pInfo == NULL)
		pInfo.reset(new PosInfo);

	bool bRiskEnabled = false;
	if (!decimal::eq(_risk_volscale, 1.0) && _risk_date == _cur_tdate)
	{
		WTSLogger::log_by_cat("risk", LL_INFO, "Risk scale of portfolio is {:.2f}", _risk_volscale);
		bRiskEnabled = true;
	}

	if (bRiskEnabled && !decimal::eq(diffPos, 0))
	{
		double symbol = diffPos / abs(diffPos);
		diffPos = decimal::rnd(abs(diffPos)*_risk_volscale)*symbol;
	}

	double targetPos = pInfo->_volume + diffPos;

	append_signal(realCode.c_str(), targetPos, false);
	save_datas();

	/*
	 *	If the strategy is bound to the execution channel
	 *	Then only submit the increment
	 *	If the strategy is not bound to the execution channel, submit the full amount
	 */
	const auto& exec_ids = _exec_mgr.get_route(straName);
	for(auto& execid : exec_ids)
		_exec_mgr.handle_pos_change(realCode.c_str(), targetPos, diffPos, execid.c_str());
}

void WtCtaEngine::on_tick(const char* stdCode, WTSTickData* curTick)
{
	WtEngine::on_tick(stdCode, curTick);

	_data_mgr->handle_push_quote(stdCode, curTick);

	//If it is a real code, it must be passed to the executor
	/*
	 *	Here no longer make judgments, directly pass all to the executor manager, because the executor may process unsubscribed contracts
	 *	The main scenario is during the main contract month change period
	 *	By Wesley @ 2021.08.19
	 */
	{
		//Is it the main contract code mark, mainly used to send data to the executor
		_exec_mgr.handle_tick(stdCode, curTick);
	}

	/*
	 *	By Wesley @ 2022.02.07
	 *	Here is a thorough adjustment
	 *	First, check the subscription mark. If the mark is 0, that is, the non-right mode, then directly trigger ontick according to the original code
	 *	Second, if the mark is 1, that is, the pre-right mode, then convert the code to xxxx-, and then trigger ontick
	 *	Third, if the mark is 2, that is, the post-right mode, then convert the code to xxxx+, then correct the tick data, and then trigger ontick
	 */
	if(_ready)
	{
		auto sit = _tick_sub_map.find(stdCode);
		if (sit == _tick_sub_map.end())
			return;

		uint32_t flag = get_adjusting_flag();
		WTSTickData* adjTick = nullptr;

		//By Wesley
		//Here to make a copy, although there is some overhead, it can avoid some problems, such as subscribing to tick during ontick
		SubList sids = sit->second;
		for (auto it = sids.begin(); it != sids.end(); it++)
		{
			uint32_t sid = it->first;
				

			auto cit = _ctx_map.find(sid);
			if (cit != _ctx_map.end())
			{
				CtaContextPtr& ctx = (CtaContextPtr&)cit->second;
				uint32_t opt = it->second.second;
					
				if (opt == 0)
				{
					/*
					 *	By Wesley @ 2023.06.27
					 *	If using a thread pool, schedule it to the thread pool
					 */
					if(_pool)
					{
						_pool->schedule([ctx, stdCode, curTick]() {
							ctx->on_tick(stdCode, curTick);
						});
					}
					else
						ctx->on_tick(stdCode, curTick);
				}
				else
				{
					std::string wCode = stdCode;
					wCode = fmt::format("{}{}", stdCode, opt == 1 ? SUFFIX_QFQ : SUFFIX_HFQ);
					if (opt == 1)
					{
						if (_pool)
						{
							_pool->schedule([ctx, wCode, curTick]() {
								ctx->on_tick(wCode.c_str(), curTick);
							});
						}
						else
							ctx->on_tick(wCode.c_str(), curTick);
					}
					else //(opt == 2)
					{
						if (adjTick == nullptr)
						{
							WTSTickData* adjTick = WTSTickData::create(curTick->getTickStruct());
							WTSTickStruct& adjTS = adjTick->getTickStruct();
							adjTick->setContractInfo(curTick->getContractInfo());

							//Here to do a processing of the right factor
							double factor = get_exright_factor(stdCode);
							adjTS.open *= factor;
							adjTS.high *= factor;
							adjTS.low *= factor;
							adjTS.price *= factor;

							adjTS.settle_price *= factor;

							adjTS.pre_close *= factor;
							adjTS.pre_settle *= factor;

							/*
							 *	By Wesley @ 2022.08.15
							 *	Here to do a perfecting of the tick right
							 */
							if (flag & 1)
							{
								adjTS.total_volume /= factor;
								adjTS.volume /= factor;
							}

							if (flag & 2)
							{
								adjTS.total_turnover *= factor;
								adjTS.turn_over *= factor;
							}

							if (flag & 4)
							{
								adjTS.open_interest /= factor;
								adjTS.diff_interest /= factor;
								adjTS.pre_interest /= factor;
							}

							_price_map[wCode] = adjTS.price;
						}

						if (_pool)
						{
							_pool->schedule([ctx, wCode, adjTick]() {
								ctx->on_tick(wCode.c_str(), adjTick);
							});
						}
						else
							ctx->on_tick(wCode.c_str(), adjTick);

					}
				}
			}				
		}

		if(nullptr != adjTick)
			adjTick->release();
		/*
		 *	By Wesley @ 223.06.27
		 *	Here, you must wait for all thread pools to complete scheduling
		 */
		if (_pool)
			_pool->wait();
	}
	
}

void WtCtaEngine::on_bar(const char* stdCode, const char* period, uint32_t times, WTSBarStruct* newBar)
{
	thread_local static char key[64] = { 0 };
	fmtutil::format_to(key, "{}-{}-{}", stdCode, period, times);

	const SubList& sids = _bar_sub_map[key];
	for (auto it = sids.begin(); it != sids.end(); it++)
	{
		uint32_t sid = it->first;
		auto cit = _ctx_map.find(sid);
		if(cit != _ctx_map.end())
		{
			CtaContextPtr& ctx = (CtaContextPtr&)cit->second;
			if (_pool)
			{
				_pool->schedule([ctx, stdCode, period, times, newBar]() {
					ctx->on_bar(stdCode, period, times, newBar);
				});
			}
			else
				ctx->on_bar(stdCode, period, times, newBar);
		}
	}

	/*
	 *	By Wesley @ 223.06.27
	 *	Here, you must wait for all thread pools to complete scheduling
	 */
	if (_pool)
		_pool->wait();

	WTSLogger::info("KBar [{}] @ {} closed", key, period[0] == 'd' ? newBar->date : newBar->time);
}

bool WtCtaEngine::isInTrading()
{
	return _tm_ticker->is_in_trading();
}

uint32_t WtCtaEngine::transTimeToMin(uint32_t uTime)
{
	return _tm_ticker->time_to_mins(uTime);
}

WTSCommodityInfo* WtCtaEngine::get_comm_info(const char* stdCode)
{
	CodeHelper::CodeInfo codeInfo = CodeHelper::extractStdCode(stdCode, _hot_mgr);
	return _base_data_mgr->getCommodity(codeInfo._exchg, codeInfo._product);
}

WTSSessionInfo* WtCtaEngine::get_sess_info(const char* stdCode)
{
	CodeHelper::CodeInfo codeInfo = CodeHelper::extractStdCode(stdCode, _hot_mgr);
	WTSCommodityInfo* cInfo = _base_data_mgr->getCommodity(codeInfo._exchg, codeInfo._product);
	if (cInfo == NULL)
		return NULL;

	return cInfo->getSessionInfo();
}

uint64_t WtCtaEngine::get_real_time()
{
	return TimeUtils::makeTime(_cur_date, _cur_raw_time * 100000 + _cur_secs);
}

void WtCtaEngine::notify_chart_marker(uint64_t time, const char* straId, double price, const char* icon, const char* tag)
{
	if (_notifier)
		_notifier->notify_chart_marker(time, straId, price, icon, tag);
}

void WtCtaEngine::notify_chart_index(uint64_t time, const char* straId, const char* idxName, const char* lineName, double val)
{
	if (_notifier)
		_notifier->notify_chart_index(time, straId, idxName, lineName, val);
}

void WtCtaEngine::notify_trade(const char* straId, const char* stdCode, bool isLong, bool isOpen, uint64_t curTime, double price, const char* userTag)
{
	if (_notifier)
		_notifier->notify_trade(straId, stdCode, isLong, isOpen, curTime, price, userTag);
}