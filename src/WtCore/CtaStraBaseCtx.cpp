/*!
 * \file CtaStraBaseCtx.cpp
 * \project WonderTrader
 *
 * \author Wesley
 * \date 2020/03/30
 *
 * \brief
 */
#include "CtaStraBaseCtx.h"
#include "WtCtaEngine.h"
#include "WtHelper.h"

#include <exception>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
namespace rj = rapidjson;

#include "../Share/StrUtil.hpp"
#include "../Includes/WTSContractInfo.hpp"
#include "../Includes/WTSSessionInfo.hpp"
#include "../Includes/IHotMgr.h"
#include "../Includes/WTSTradeDef.hpp"
#include "../Share/decimal.h"
#include "../Share/CodeHelper.hpp"

#include "../WTSTools/WTSLogger.h"

const char* CMP_ALG_NAMES[] =
{
	"＝",
	">",
	"<",
	">=",
	"<="
};

const char* ACTION_NAMES[] =
{
	"OL",
	"CL",
	"OS",
	"CS",
	"SYN"
};


inline uint32_t makeCtaCtxId()
{
	static std::atomic<uint32_t> _auto_context_id{ 1 };
	return _auto_context_id.fetch_add(1);
}


CtaStraBaseCtx::CtaStraBaseCtx(WtCtaEngine* engine, const char* name, int32_t slippage)
	: ICtaStraCtx(name)
	, _engine(engine)
	, _total_calc_time(0)
	, _emit_times(0)
	, _last_cond_min(0)
	, _is_in_schedule(false)
	, _ud_modified(false)
	, _last_barno(0)
	, _slippage(slippage)
{
	_context_id = makeCtaCtxId();
}


CtaStraBaseCtx::~CtaStraBaseCtx()
{
}

void CtaStraBaseCtx::init_outputs()
{
	std::string folder = WtHelper::getOutputDir();
	folder += _name;
	folder += "//";
	BoostFile::create_directories(folder.c_str());	

	std::string filename = folder + "trades.csv";
	_trade_logs.reset(new BoostFile());
	{
		bool isNewFile = !BoostFile::exists(filename.c_str());
		_trade_logs->create_or_open_file(filename.c_str());
		if (isNewFile)
		{
			_trade_logs->write_file("code,time,direct,action,price,qty,tag,fee,barno\n");
		}
		else
		{
			_trade_logs->seek_to_end();
		}
	}

	filename = folder + "closes.csv";
	_close_logs.reset(new BoostFile());
	{
		bool isNewFile = !BoostFile::exists(filename.c_str());
		_close_logs->create_or_open_file(filename.c_str());
		if (isNewFile)
		{
			_close_logs->write_file("code,direct,opentime,openprice,closetime,closeprice,qty,profit,totalprofit,entertag,exittag,openbarno,closebarno\n");
		}
		else
		{
			_close_logs->seek_to_end();
		}
	}

	filename = folder + "funds.csv";
	_fund_logs.reset(new BoostFile());
	{
		bool isNewFile = !BoostFile::exists(filename.c_str());
		_fund_logs->create_or_open_file(filename.c_str());
		if (isNewFile)
		{
			_fund_logs->write_file("date,closeprofit,positionprofit,dynbalance,fee\n");
		}
		else
		{
			_fund_logs->seek_to_end();
		}
	}

	filename = folder + "signals.csv";
	_sig_logs.reset(new BoostFile());
	{
		bool isNewFile = !BoostFile::exists(filename.c_str());
		_sig_logs->create_or_open_file(filename.c_str());
		if (isNewFile)
		{
			_sig_logs->write_file("code,target,sigprice,gentime,usertag\n");
		}
		else
		{
			_sig_logs->seek_to_end();
		}
	}

	filename = folder + "positions.csv";
	_pos_logs.reset(new BoostFile());	
	{
		bool isNewFile = !BoostFile::exists(filename.c_str());
		_pos_logs->create_or_open_file(filename.c_str());
		if (isNewFile)
		{
			_pos_logs->write_file("date,code,volume,closeprofit,dynprofit\n");
		}
		else
		{
			_pos_logs->seek_to_end();
		}
	}

	filename = folder + "indice.csv";
	_idx_logs.reset(new BoostFile());
	{
		bool isNewFile = !BoostFile::exists(filename.c_str());
		_idx_logs->create_or_open_file(filename.c_str());
		if (isNewFile)
		{
			_idx_logs->write_file("bartime,index_name,line_name,value\n");
		}
		else
		{
			_idx_logs->seek_to_end();
		}
	}

	filename = folder + "marks.csv";
	_mark_logs.reset(new BoostFile());
	{
		bool isNewFile = !BoostFile::exists(filename.c_str());
		_mark_logs->create_or_open_file(filename.c_str());
		if (isNewFile)
		{
			_mark_logs->write_file("bartime,price,icon,tag\n");
		}
		else
		{
			_mark_logs->seek_to_end();
		}
	}
}

void CtaStraBaseCtx::log_signal(const char* stdCode, double target, double price, uint64_t gentime, const char* usertag /* = "" */)
{
	if (_sig_logs)
	{
		std::stringstream ss;
		ss << stdCode << "," << target << "," << price << "," << gentime << "," << usertag << "\n";
		_sig_logs->write_file(ss.str());
	}
}

void CtaStraBaseCtx::log_trade(const char* stdCode, bool isLong, bool isOpen, uint64_t curTime, double price, double qty, const char* userTag /* = "" */, double fee /* = 0.0 */, uint32_t barNo /* = 0 */)
{
	if (_trade_logs)
	{
		std::stringstream ss;
		ss << stdCode << "," << curTime << "," << (isLong ? "LONG" : "SHORT") << "," << (isOpen ? "OPEN" : "CLOSE") << "," << price << "," << qty << "," << userTag << "," << fee << "," << barNo << "\n";
		_trade_logs->write_file(ss.str());
	}

	_engine->notify_trade(this->name(),stdCode, isLong, isOpen, curTime, price, userTag);
}

void CtaStraBaseCtx::log_close(const char* stdCode, bool isLong, uint64_t openTime, double openpx, uint64_t closeTime, double closepx, double qty, double profit, double totalprofit /* = 0 */, 
	const char* enterTag /* = "" */, const char* exitTag /* = "" */, uint32_t openBarNo /* = 0 */, uint32_t closeBarNo /* = 0 */)
{
	if (_close_logs)
	{
		std::stringstream ss;
		ss << stdCode << "," << (isLong ? "LONG" : "SHORT") << "," << openTime << "," << openpx
			<< "," << closeTime << "," << closepx << "," << qty << "," << profit << "," 
			<< totalprofit << "," << enterTag << "," << exitTag << "," << openBarNo << "," << closeBarNo << "\n";
		_close_logs->write_file(ss.str());
	}
}
void CtaStraBaseCtx::save_userdata()
{
	rj::Document root(rj::kObjectType);
	rj::Document::AllocatorType &allocator = root.GetAllocator();
	for (auto it = _user_datas.begin(); it != _user_datas.end(); it++)
	{
		root.AddMember(rj::Value(it->first.c_str(), allocator), rj::Value(it->second.c_str(), allocator), allocator);
	}

	{
		std::string filename = WtHelper::getStraUsrDatDir();
		filename += "ud_";
		filename += _name;
		filename += ".json";

		BoostFile bf;
		if (bf.create_new_file(filename.c_str()))
		{
			rj::StringBuffer sb;
			rj::PrettyWriter<rj::StringBuffer> writer(sb);
			root.Accept(writer);
			bf.write_file(sb.GetString());
			bf.close_file();
		}
	}
}

void CtaStraBaseCtx::load_userdata()
{
	std::string filename = WtHelper::getStraUsrDatDir();
	filename += "ud_";
	filename += _name;
	filename += ".json";

	if (!StdFile::exists(filename.c_str()))
	{
		return;
	}

	std::string content;
	StdFile::read_file_content(filename.c_str(), content);
	if (content.empty())
		return;

	rj::Document root;
	root.Parse(content.c_str());

	if (root.HasParseError())
		return;

	for (auto iter = root.MemberBegin(); iter != root.MemberEnd(); iter++)
	{
		const char* key = iter->name.GetString();
		const char* val = iter->value.GetString();
		_user_datas[key] = val;
	}
}

void CtaStraBaseCtx::load_data(uint32_t flag /* = 0xFFFFFFFF */)
{
	std::string filename = WtHelper::getStraDataDir();
	filename += _name;
	filename += ".json";
	
	if (!StdFile::exists(filename.c_str()))
	{
		return;
	}

	std::string content;
	StdFile::read_file_content(filename.c_str(), content);
	if (content.empty())
		return;

	rj::Document root;
	root.Parse(content.c_str());

	if (root.HasParseError())
		return;

	if(root.HasMember("fund"))
	{
		// get fund value
		const rj::Value& jFund = root["fund"];
		if(!jFund.IsNull() && jFund.IsObject())
		{
			_fund_info._total_profit = jFund["total_profit"].GetDouble();
			_fund_info._total_dynprofit = jFund["total_dynprofit"].GetDouble();
			uint32_t tdate = jFund["tdate"].GetUint();
			_fund_info._total_fees = jFund["total_fees"].GetDouble();
		}
	}

	{
		// get position value
		double total_profit = 0;
		double total_dynprofit = 0;
		const rj::Value& jPos = root["positions"];
		if (!jPos.IsNull() && jPos.IsArray())
		{
			for (const rj::Value& pItem : jPos.GetArray())
			{
				const char* stdCode = pItem["code"].GetString();
				const char* ruleTag = _engine->get_hot_mgr()->getRuleTag(stdCode);
				bool isExpired = (strlen(ruleTag) == 0 && _engine->get_contract_info(stdCode) == NULL);

				if(isExpired)
					log_info("{} not exists or expired, position ignored", stdCode);

				PosInfo& pInfo = _pos_map[stdCode];
				pInfo._closeprofit = pItem["closeprofit"].GetDouble();
				pInfo._last_entertime = pItem["lastentertime"].GetUint64();
				pInfo._last_exittime = pItem["lastexittime"].GetUint64();
				pInfo._volume = isExpired ? 0 : pItem["volume"].GetDouble();
				if (pItem.HasMember("frozen") && !isExpired)
				{
					pInfo._frozen = pItem["frozen"].GetDouble();
					pInfo._frozen_date = pItem["frozendate"].GetUint();
				}

				if (pInfo._volume == 0 || isExpired)
				{
					//By Wesley @ 2023.02.21
					// The reason for adding this line is that some option contracts often hold until the delivery date
					// So if the contract expires, then the floating profit and loss should be accumulated as the closing profit and loss
					// After processing, the next load, the floating profit and loss is 0
					pInfo._closeprofit += pInfo._dynprofit;

					pInfo._dynprofit = 0;
					pInfo._frozen = 0;
				}
				else
					pInfo._dynprofit = pItem["dynprofit"].GetDouble();

				total_profit += pInfo._closeprofit;
				total_dynprofit += pInfo._dynprofit;

				const rj::Value& details = pItem["details"];
				if (details.IsNull() || !details.IsArray() || details.Size() == 0 || isExpired)
					continue;

				for (uint32_t i = 0; i < details.Size(); i++)
				{
					const rj::Value& dItem = details[i];
					double vol = dItem["volume"].GetDouble();
					if(decimal::eq(vol, 0))
						continue;

					DetailInfo dInfo;
					dInfo._long = dItem["long"].GetBool();
					dInfo._price = dItem["price"].GetDouble();
					dInfo._volume = dItem["volume"].GetDouble();
					dInfo._opentime = dItem["opentime"].GetUint64();
					if(dItem.HasMember("opentdate"))
						dInfo._opentdate = dItem["opentdate"].GetUint();

					if (dItem.HasMember("maxprice"))
						dInfo._max_price = dItem["maxprice"].GetDouble();
					else
						dInfo._max_price = dInfo._price;

					if (dItem.HasMember("minprice"))
						dInfo._min_price = dItem["minprice"].GetDouble();
					else
						dInfo._min_price = dInfo._price;

					dInfo._profit = dItem["profit"].GetDouble();
					dInfo._max_profit = dItem["maxprofit"].GetDouble();
					dInfo._max_loss = dItem["maxloss"].GetDouble();

					strcpy(dInfo._opentag, dItem["opentag"].GetString());
					if (dItem.HasMember("openbarno"))
						dInfo._open_barno = dItem["openbarno"].GetUint();
					else
						dInfo._open_barno = 0;

					pInfo._details.emplace_back(dInfo);
				}

				if(!isExpired)
				{
					log_info("Position confirmed,{} -> {}", stdCode, pInfo._volume);
					stra_sub_ticks(stdCode);
				}
			}
		}

		_fund_info._total_profit = total_profit;
		_fund_info._total_dynprofit = total_dynprofit;
	}

	{
		// get condition contract
		uint32_t count = 0;
		const rj::Value& jCond = root["conditions"];
		if (!jCond.IsNull() && jCond.IsObject())
		{
			_last_cond_min = jCond["settime"].GetUint64();
			const rj::Value& jItems = jCond["items"];
			for (auto iter = jItems.MemberBegin(); iter != jItems.MemberEnd(); iter++)	
			{
				const char* stdCode = iter->name.GetString();
				const char* ruleTag = _engine->get_hot_mgr()->getRuleTag(stdCode);
				if (strlen(ruleTag) == 0 && _engine->get_contract_info(stdCode) == NULL)
				{
					log_info("{} not exists or expired, condition ignored", stdCode);
					continue;
				}

				const rj::Value& cListItem = iter->value;

				CondList& condList = _condtions[stdCode];

				for(auto& cItem : cListItem.GetArray())
				{
					CondEntrust condInfo;
					strcpy(condInfo._code, stdCode);
					strcpy(condInfo._usertag, cItem["usertag"].GetString());

					condInfo._field = (WTSCompareField)cItem["field"].GetUint();
					condInfo._alg = (WTSCompareType)cItem["alg"].GetUint();
					condInfo._target = cItem["target"].GetDouble();
					condInfo._qty = cItem["qty"].GetDouble();
					condInfo._action = (char)cItem["action"].GetUint();

					condList.emplace_back(condInfo);

					log_info("{} condition recovered, {} {}, condition: newprice {} {}",
						stdCode, ACTION_NAMES[condInfo._action], condInfo._qty, CMP_ALG_NAMES[condInfo._alg], condInfo._target);
					count++;
				}
			}

			log_info("{} conditions recovered, setup time: {}", count, _last_cond_min);
		}
	}

	if (root.HasMember("signals"))
	{
		// read all signals
		const rj::Value& jSignals = root["signals"];
		if (!jSignals.IsNull() && jSignals.IsObject())
		{
			for (auto iter = jSignals.MemberBegin(); iter != jSignals.MemberEnd(); iter++)	
			{
				const char* stdCode = iter->name.GetString();
				const char* ruleTag = _engine->get_hot_mgr()->getRuleTag(stdCode);
				if (strlen(ruleTag) == 0 && _engine->get_contract_info(stdCode) == NULL)
				{
					log_info("{} not exists or expired, signal ignored", stdCode);
					continue;
				}

				const rj::Value& jItem = iter->value;

				SigInfo& sInfo = _sig_map[stdCode];
				sInfo._usertag = jItem["usertag"].GetString();
				sInfo._volume = jItem["volume"].GetDouble();
				sInfo._sigprice = jItem["sigprice"].GetDouble();
				sInfo._gentime = jItem["gentime"].GetUint64();
				
				log_info("{} untouched signal recovered, target pos: {}", stdCode, sInfo._volume);
				stra_sub_ticks(stdCode);
			}
		}
	}

	if (root.HasMember("utils"))
	{
	    // Read miscellaneous data
	    const rj::Value& jUtils = root["utils"];
	    if (!jUtils.IsNull() && jUtils.IsObject())
	    {
	        _last_barno = jUtils["lastbarno"].GetUint();
	    }
	}
}

void CtaStraBaseCtx::save_data(uint32_t flag /* = 0xFFFFFFFF */)
{
	rj::Document root(rj::kObjectType);

	{
		// save position data
		rj::Value jPos(rj::kArrayType);

		rj::Document::AllocatorType &allocator = root.GetAllocator();

		for (auto it = _pos_map.begin(); it != _pos_map.end(); it++)
		{
			const char* stdCode = it->first.c_str();
			const PosInfo& pInfo = it->second;

			rj::Value pItem(rj::kObjectType);
			pItem.AddMember("code", rj::Value(stdCode, allocator), allocator);
			pItem.AddMember("volume", pInfo._volume, allocator);
			pItem.AddMember("closeprofit", pInfo._closeprofit, allocator);
			pItem.AddMember("dynprofit", pInfo._dynprofit, allocator);
			pItem.AddMember("lastentertime", pInfo._last_entertime, allocator);
			pItem.AddMember("lastexittime", pInfo._last_exittime, allocator);
			pItem.AddMember("frozen", pInfo._frozen, allocator);
			pItem.AddMember("frozendate", pInfo._frozen_date, allocator);

			rj::Value details(rj::kArrayType);
			for (auto dit = pInfo._details.begin(); dit != pInfo._details.end(); dit++)
			{
				const DetailInfo& dInfo = *dit;
				rj::Value dItem(rj::kObjectType);
				dItem.AddMember("long", dInfo._long, allocator);
				dItem.AddMember("price", dInfo._price, allocator);
				dItem.AddMember("maxprice", dInfo._max_price, allocator);
				dItem.AddMember("minprice", dInfo._min_price, allocator);
				dItem.AddMember("volume", dInfo._volume, allocator);
				dItem.AddMember("opentime", dInfo._opentime, allocator);
				dItem.AddMember("opentdate", dInfo._opentdate, allocator);

				dItem.AddMember("profit", dInfo._profit, allocator);
				dItem.AddMember("maxprofit", dInfo._max_profit, allocator);
				dItem.AddMember("maxloss", dInfo._max_loss, allocator);
				dItem.AddMember("opentag", rj::Value(dInfo._opentag, allocator), allocator);
				dItem.AddMember("openbarno", dInfo._open_barno, allocator);

				details.PushBack(dItem, allocator);
			}

			pItem.AddMember("details", details, allocator);

			jPos.PushBack(pItem, allocator);
		}

		root.AddMember("positions", jPos, allocator);
	}

	{
		// save fund data
		rj::Value jFund(rj::kObjectType);
		rj::Document::AllocatorType &allocator = root.GetAllocator();

		jFund.AddMember("total_profit", _fund_info._total_profit, allocator);
		jFund.AddMember("total_dynprofit", _fund_info._total_dynprofit, allocator);
		jFund.AddMember("total_fees", _fund_info._total_fees, allocator);
		jFund.AddMember("tdate", _engine->get_trading_date(), allocator);

		root.AddMember("fund", jFund, allocator);
	}

	{
		// save signal data
		rj::Value jSigs(rj::kObjectType);
		rj::Document::AllocatorType &allocator = root.GetAllocator();

		for (auto& m:_sig_map)
		{
			const char* stdCode = m.first.c_str();
			const SigInfo& sInfo = m.second;

			rj::Value jItem(rj::kObjectType);
			jItem.AddMember("usertag", rj::Value(sInfo._usertag.c_str(), allocator), allocator);

			jItem.AddMember("volume", sInfo._volume, allocator);
			jItem.AddMember("sigprice", sInfo._sigprice, allocator);
			jItem.AddMember("gentime", sInfo._gentime, allocator);

			jSigs.AddMember(rj::Value(stdCode, allocator), jItem, allocator);
		}

		root.AddMember("signals", jSigs, allocator);
	}

	{
		// save condition data
		rj::Value jCond(rj::kObjectType);
		rj::Value jItems(rj::kObjectType);

		rj::Document::AllocatorType &allocator = root.GetAllocator();

		for (auto it = _condtions.begin(); it != _condtions.end(); it++)
		{
			const char* code = it->first.c_str();
			const CondList& condList = it->second;

			rj::Value cArray(rj::kArrayType);
			for(auto& condInfo : condList)
			{
				rj::Value cItem(rj::kObjectType);
				cItem.AddMember("code", rj::Value(code, allocator), allocator);
				cItem.AddMember("usertag", rj::Value(condInfo._usertag, allocator), allocator);

				cItem.AddMember("field", (uint32_t)condInfo._field, allocator);
				cItem.AddMember("alg", (uint32_t)condInfo._alg, allocator);
				cItem.AddMember("target", condInfo._target, allocator);
				cItem.AddMember("qty", condInfo._qty, allocator);
				cItem.AddMember("action", (uint32_t)condInfo._action, allocator);

				cArray.PushBack(cItem, allocator);
			}

			jItems.AddMember(rj::Value(code, allocator), cArray, allocator);
		}
		jCond.AddMember("settime", _last_cond_min, allocator);
		jCond.AddMember("items", jItems, allocator);

		root.AddMember("conditions", jCond, allocator);
	}

	{
		// save miscellaneous data
		rj::Value jUtils(rj::kObjectType);

		rj::Document::AllocatorType &allocator = root.GetAllocator();

		jUtils.AddMember("lastbarno", _last_barno, allocator);

		root.AddMember("utils", jUtils, allocator);
	}

	{
		std::string filename = WtHelper::getStraDataDir();
		filename += _name;
		filename += ".json";

		BoostFile bf;
		if (bf.create_new_file(filename.c_str()))
		{
			rj::StringBuffer sb;
			rj::PrettyWriter<rj::StringBuffer> writer(sb);
			root.Accept(writer);
			bf.write_file(sb.GetString());
			bf.close_file();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// callback functions
void CtaStraBaseCtx::on_bar(const char* stdCode, const char* period, uint32_t times, WTSBarStruct* newBar)
{
	if (newBar == NULL)
		return;

	thread_local static char realPeriod[8] = { 0 };
	fmtutil::format_to(realPeriod, "{}{}", period, times);

	thread_local static char key[64] = { 0 };
	fmtutil::format_to(key, "{}#{}", stdCode, realPeriod);

	KlineTag& tag = _kline_tags[key];
	tag._closed = true;

	if(tag._notify)
		on_bar_close(stdCode, realPeriod, newBar);

	if(key == _main_key)
		log_debug("Main KBars {} closed", key);
}

void CtaStraBaseCtx::on_init()
{
	init_outputs();

	load_data();

	load_userdata();
}

void CtaStraBaseCtx::dump_chart_info()
{
	rj::Document root(rj::kObjectType);
	rj::Document::AllocatorType &allocator = root.GetAllocator();

	rj::Value klineItem(rj::kObjectType);
	if (_chart_code.empty())
	{
		// if no main kline, use main kline to dump
		klineItem.AddMember("code", rj::Value(_main_code.c_str(), allocator), allocator);
		klineItem.AddMember("period", rj::Value(_main_period.c_str(), allocator), allocator);
	}
	else
	{
		klineItem.AddMember("code", rj::Value(_chart_code.c_str(), allocator), allocator);
		klineItem.AddMember("period", rj::Value(_chart_period.c_str(), allocator), allocator);
	}

	root.AddMember("kline", klineItem, allocator);

	if (!_chart_indice.empty())
	{
		rj::Value jIndice(rj::kArrayType);
		for (const auto& v : _chart_indice)
		{
			const ChartIndex& cIndex = v.second;
			rj::Value jIndex(rj::kObjectType);
			jIndex.AddMember("name", rj::Value(cIndex._name.c_str(), allocator), allocator);
			jIndex.AddMember("index_type", cIndex._indexType, allocator);

			rj::Value jLines(rj::kArrayType);
			for (const auto& v2 : cIndex._lines)
			{
				const ChartLine& cLine = v2.second;
				rj::Value jLine(rj::kObjectType);
				jLine.AddMember("name", rj::Value(cLine._name.c_str(), allocator), allocator);
				jLine.AddMember("line_type", cLine._lineType, allocator);

				jLines.PushBack(jLine, allocator);
			}

			jIndex.AddMember("lines", jLines, allocator);

			rj::Value jBaseLines(rj::kObjectType);
			for (const auto& v3 : cIndex._base_lines)
			{
				jBaseLines.AddMember(rj::Value(v3.first.c_str(), allocator), rj::Value(v3.second), allocator);
			}

			jIndex.AddMember("baselines", jBaseLines, allocator);

			jIndice.PushBack(jIndex, allocator);
		}

		root.AddMember("index", jIndice, allocator);
	}

	std::string folder = WtHelper::getOutputDir();
	folder += _name;
	folder += "/";

	if (!StdFile::exists(folder.c_str()))
		boost::filesystem::create_directories(folder.c_str());

	std::string filename = folder;
	filename += "rtchart.json";

	rj::StringBuffer sb;
	rj::PrettyWriter<rj::StringBuffer> writer(sb);
	root.Accept(writer);
	StdFile::write_file_content(filename.c_str(), sb.GetString());
}

void CtaStraBaseCtx::update_dyn_profit(const char* stdCode, double price)
{
	auto it = _pos_map.find(stdCode);
	if (it != _pos_map.end())
	{
		PosInfo& pInfo = (PosInfo&)it->second;
		if (pInfo._volume == 0)
		{
			pInfo._dynprofit = 0;
		}
		else
		{
			WTSCommodityInfo* commInfo = _engine->get_commodity_info(stdCode);
			double dynprofit = 0;
			for (auto pit = pInfo._details.begin(); pit != pInfo._details.end(); pit++)
			{
				DetailInfo& dInfo = *pit;
				dInfo._profit = dInfo._volume*(price - dInfo._price)*commInfo->getVolScale()*(dInfo._long ? 1 : -1);
				if (dInfo._profit > 0)
					dInfo._max_profit = std::max(dInfo._profit, dInfo._max_profit);
				else if (dInfo._profit < 0)
					dInfo._max_loss = std::min(dInfo._profit, dInfo._max_loss);

				dInfo._max_price = std::max(dInfo._max_price, price);
				dInfo._min_price = std::min(dInfo._min_price, price);

				dynprofit += dInfo._profit;
			}

			pInfo._dynprofit = dynprofit;
		}
	}

	double total_dynprofit = 0;
	for(auto& v : _pos_map)
	{
		const PosInfo& pInfo = v.second;
		total_dynprofit += pInfo._dynprofit;
	}

	_fund_info._total_dynprofit = total_dynprofit;
}

void CtaStraBaseCtx::on_tick(const char* stdCode, WTSTickData* newTick, bool bEmitStrategy /* = true */)
{
	_price_map[stdCode] = newTick->price();

	// Check if a signal needs to be triggered first
	{
		auto it = _sig_map.find(stdCode);
		if(it != _sig_map.end())
		{
			WTSSessionInfo* sInfo = _engine->get_session_info(stdCode, true);
			if (sInfo->isInTradingTime(_engine->get_raw_time(), true))
			{
				const SigInfo sInfo = it->second;
				 // Only when the signal type is not 0, i.e., it is an intra-bar signal or a conditional order trigger signal, and the signal has not been triggered
				do_set_position(stdCode, sInfo._volume, sInfo._usertag.c_str(), (sInfo._sigtype != 0 && !sInfo._triggered));
				_sig_map.erase(it);

				 // If it is a conditional order trigger, call back on_condition_triggered
				if(sInfo._sigtype == 2)
					on_condition_triggered(stdCode, sInfo._volume, newTick->price(), sInfo._usertag.c_str());
			}
			
		}
	}

	// Update floating profit and loss
	update_dyn_profit(stdCode, newTick->price());

	//////////////////////////////////////////////////////////////////////////
	// Check conditional orders
	if(!_condtions.empty())
	{
		auto it = _condtions.find(stdCode);
		if (it == _condtions.end())
			return;

		const CondList& condList = it->second;
		for (const CondEntrust& entrust : condList)
		{
			double curPrice = newTick->price();

			bool isMatched = false;
			switch (entrust._alg)
			{
			case WCT_Equal:
				isMatched = decimal::eq(curPrice, entrust._target);
				break;
			case WCT_Larger:
				isMatched = decimal::gt(curPrice, entrust._target);
				break;
			case WCT_LargerOrEqual:
				isMatched = decimal::ge(curPrice, entrust._target);
				break;
			case WCT_Smaller:
				isMatched = decimal::lt(curPrice, entrust._target);
				break;
			case WCT_SmallerOrEqual:
				isMatched = decimal::le(curPrice, entrust._target);
				break;
			default:
				break;
			}

			if (isMatched)
			{
				log_info("Condition triggered[newprice {}{} targetprice {}], instrument: {}, {} {}",
					curPrice, CMP_ALG_NAMES[entrust._alg], entrust._target, stdCode, ACTION_NAMES[entrust._action], entrust._qty);

				switch (entrust._action)
				{
				case COND_ACTION_OL:
				{
					double curQty = stra_get_position(stdCode);
					double desQty = 0;
					if (decimal::lt(curQty, 0))
						desQty = entrust._qty;
					else
						desQty = curQty + entrust._qty;

					append_signal(stdCode, desQty, entrust._usertag, 2);
				}
				break;
				case COND_ACTION_CL:
				{
					double curQty = stra_get_position(stdCode);
					if (decimal::gt(curQty, 0))
					{
						double maxQty = min(curQty, entrust._qty);
						double desQty = curQty - maxQty;
						append_signal(stdCode, desQty, entrust._usertag, 2);
					}
				}
				break;
				case COND_ACTION_OS:
				{
					double curQty = stra_get_position(stdCode);
					double desQty = 0;
					if (decimal::gt(curQty, 0))
						desQty = -entrust._qty;
					else
						desQty = curQty - entrust._qty;

					append_signal(stdCode, desQty, entrust._usertag, 2);
				}
				break;
				case COND_ACTION_CS:
				{
					double curQty = stra_get_position(stdCode);
					if (decimal::lt(curQty, 0))
					{
						double maxQty = min(abs(curQty), entrust._qty);
						double desQty = curQty + maxQty;
						append_signal(stdCode, desQty, entrust._usertag, 2);
					}
				}
				break;
				case COND_ACTION_SP: 
				{
					append_signal(stdCode, entrust._qty, entrust._usertag, 2);
				}
				break;
				default: break;
				}

				// For the same bar, only one condition order for the same contract can be triggered
				// So it can be cleared directly here
				_condtions.erase(it);
				break;
			}
		}
	}

	if (bEmitStrategy)
		on_tick_updated(stdCode, newTick);

	if(_ud_modified)
	{
		save_userdata();
		_ud_modified = false;
	}
}

bool CtaStraBaseCtx::on_schedule(uint32_t curDate, uint32_t curTime)
{
	_is_in_schedule = true; // Start scheduling, modify the flag

	// Mainly used to save floating profit and loss
	save_data();

	bool isMainUdt = false;
	bool emmited = false;

	for (auto it = _kline_tags.begin(); it != _kline_tags.end(); it++)
	{
		const char* key = it->first.c_str();
		KlineTag& marker = (KlineTag&)it->second;

		auto idx = StrUtil::findFirst(key, '#');

		std::string stdCode(key, idx);

		if (key == _main_key)
		{
			if (marker._closed)
			{
				isMainUdt = true;
				marker._closed = false;
			}
			else
			{
				isMainUdt = false;
				break;
			}
		}

		WTSSessionInfo* sInfo = _engine->get_session_info(stdCode.c_str(), true);

		if (isMainUdt || _kline_tags.empty())
		{	
			TimeUtils::Ticker ticker;

			uint32_t offTime = sInfo->offsetTime(curTime, true);
			if(offTime <= sInfo->getCloseTime(true))
			{
				_condtions.clear();
				on_calculate(curDate, curTime);
				log_debug("Strategy {} scheduled @ {}", _name, curTime);
				emmited = true;

				_emit_times++;
				_total_calc_time += ticker.micro_seconds();

				if (_emit_times % 20 == 0)
				{
					log_info("Strategy has been scheduled {} times, totally taking {} us, {:.3f} us each time",
						_emit_times, _total_calc_time, _total_calc_time*1.0 / _emit_times);
				}

				if (_ud_modified)
				{
					save_userdata();
					_ud_modified = false;
				}

				if(!_condtions.empty())
				{
					_last_cond_min = (uint64_t)curDate * 10000 + curTime;
					save_data();
				}
			}
			else
			{
				log_info("{} not in trading time, schedule canceled", curTime);
			}
			break;
		}
	}

	_is_in_schedule = false;//Scheduling ends, modify the flag
	_last_barno++;	//Each calculation, barno plus 1
	return emmited;
}

void CtaStraBaseCtx::on_session_begin(uint32_t uTDate)
{
	//Each trading day begins, the frozen position should be set to zero
	for (auto& it : _pos_map)
	{
		const char* stdCode = it.first.c_str();
		PosInfo& pInfo = (PosInfo&)it.second;
		if(pInfo._frozen_date!=0 && pInfo._frozen_date < uTDate && !decimal::eq(pInfo._frozen, 0))
		{
			log_debug("{} of %s frozen on {} released on {}", pInfo._frozen, stdCode, pInfo._frozen_date, uTDate);

			pInfo._frozen = 0;
			pInfo._frozen_date = 0;
		}
	}

	if (_ud_modified)
	{
		save_userdata();
		_ud_modified = false;
	}
}

void CtaStraBaseCtx::enum_position(FuncEnumCtaPosCallBack cb, bool bForExecute /* = false */)
{
	/* By HeJ @ 2023.03.14
	 * When reading the theoretical position, a lock must be added to avoid repeated order placement and signal coverage caused by the simultaneous triggering of combined cross-product synchronization and signals
	 */
	std::unordered_map<std::string, double> desPos;
	{
		SpinLock lock(_mutex);		
		for (auto& it : _pos_map)
		{
			const char* stdCode = it.first.c_str();
			const PosInfo& pInfo = it.second;
			//cb(stdCode, pInfo._volume);
			desPos[stdCode] = pInfo._volume;
		}

		for (auto& sit : _sig_map)
		{
			const char* stdCode = sit.first.c_str();
			SigInfo& sInfo = (SigInfo&)sit.second;
			desPos[stdCode] = sInfo._volume;
			if (bForExecute)
				sInfo._triggered = true;
		}
	}	

	for(auto v:desPos)
	{
		cb(v.first.c_str(), v.second);
	}
}

void CtaStraBaseCtx::on_session_end(uint32_t uTDate)
{
	uint32_t curDate = uTDate;//_engine->get_trading_date();

	double total_profit = 0;
	double total_dynprofit = 0;

	for (auto it = _pos_map.begin(); it != _pos_map.end(); it++)
	{
		const char* stdCode = it->first.c_str();
		const PosInfo& pInfo = it->second;
		total_profit += pInfo._closeprofit;
		total_dynprofit += pInfo._dynprofit;

		if (decimal::eq(pInfo._volume, 0.0))
			continue;

		if(_pos_logs)
			_pos_logs->write_file(fmt::format("{},{},{},{:.2f},{:.2f}\n", curDate, stdCode,
				pInfo._volume, pInfo._closeprofit, pInfo._dynprofit));
	}

	//Here, the settlement data of the day should be written into the log file
	//Moreover, the writing methods of backtesting and real trading are different here, so keep it first and do it later
	if (_fund_logs)
		_fund_logs->write_file(fmt::format("{},{:.2f},{:.2f},{:.2f},{:.2f}\n", curDate, 
		_fund_info._total_profit, _fund_info._total_dynprofit, 
		_fund_info._total_profit + _fund_info._total_dynprofit - _fund_info._total_fees, _fund_info._total_fees));

	save_data();

	if (_ud_modified)
	{
		save_userdata();
		_ud_modified = false;
	}
}

CondList& CtaStraBaseCtx::get_cond_entrusts(const char* stdCode)
{
	CondList& ce = _condtions[stdCode];
	return ce;
}

//////////////////////////////////////////////////////////////////////////
// interface for strategy
void CtaStraBaseCtx::stra_enter_long(const char* stdCode, double qty, const char* userTag /* = "" */, double limitprice, double stopprice)
{
	WTSCommodityInfo* commInfo = _engine->get_commodity_info(stdCode);
	if (commInfo == NULL)
	{
		log_error("Cannot find corresponding commodity info of {}", stdCode);
		return;
	}

	_engine->sub_tick(id(), stdCode);
	
	if (decimal::eq(limitprice, 0.0) && decimal::eq(stopprice, 0.0))	//If it is not a dynamic order placement mode, it will be triggered directly
	{
		double curQty = stra_get_position(stdCode);
		if (decimal::lt(curQty, 0))
		{
			//If the current position is less than 0, the logic is to reverse to qty, so set the signal target position to qty
			append_signal(stdCode, qty, userTag, _is_in_schedule ? 0 : 1);
		}
		else
		{
			//If the current position is greater than or equal to 0, it is necessary to increase the long position qty
			append_signal(stdCode, curQty + qty, userTag, _is_in_schedule ? 0 : 1);
		}
	}
	else
	{
		CondList& condList = get_cond_entrusts(stdCode);

		CondEntrust entrust;
		wt_strcpy(entrust._code, stdCode);
		wt_strcpy(entrust._usertag, userTag);

		entrust._qty = qty;
		entrust._field = WCF_NEWPRICE;
		if(!decimal::eq(limitprice))
		{
			entrust._target = limitprice;
			entrust._alg = WCT_SmallerOrEqual;
		}
		else if (!decimal::eq(stopprice))
		{
			entrust._target = stopprice;
			entrust._alg = WCT_LargerOrEqual;
		}
		
		entrust._action = COND_ACTION_OL;

		condList.emplace_back(entrust);
	}
}

void CtaStraBaseCtx::stra_enter_short(const char* stdCode, double qty, const char* userTag /* = "" */, double limitprice, double stopprice)
{
	WTSCommodityInfo* commInfo = _engine->get_commodity_info(stdCode);
	if (commInfo == NULL)
	{
		log_error("Cannot find corresponding commodity info of {}", stdCode);
		return;
	}

	if (!commInfo->canShort())
	{
		log_error("Cannot short on {}", stdCode);
		return;
	}

	_engine->sub_tick(id(), stdCode);
	
	if (decimal::eq(limitprice, 0.0) && decimal::eq(stopprice, 0.0))	//If it is not a dynamic order placement mode, it will be triggered directly
	{
		double curQty = stra_get_position(stdCode);
		if (decimal::gt(curQty, 0))
		{
			//If the current position is greater than 0, the logic is to reverse to qty hands, so set the signal target position to -qty hands
			append_signal(stdCode, -qty, userTag, _is_in_schedule ? 0 : 1);
		}
		else
		{
			//If the current position is less than or equal to 0, it is to add short positions
			append_signal(stdCode, curQty - qty, userTag, _is_in_schedule ? 0 : 1);
		}
	}
	else
	{
		CondList& condList = get_cond_entrusts(stdCode);

		CondEntrust entrust;
		wt_strcpy(entrust._code, stdCode);
		wt_strcpy(entrust._usertag, userTag);

		entrust._qty = qty;
		entrust._field = WCF_NEWPRICE;
		if (!decimal::eq(limitprice))
		{
			entrust._target = limitprice;
			entrust._alg = WCT_LargerOrEqual;
		}
		else if (!decimal::eq(stopprice))
		{
			entrust._target = stopprice;
			entrust._alg = WCT_SmallerOrEqual;
		}

		entrust._action = COND_ACTION_OS;

		condList.emplace_back(entrust);
	}
}

void CtaStraBaseCtx::stra_exit_long(const char* stdCode, double qty, const char* userTag /* = "" */, double limitprice, double stopprice)
{
	WTSCommodityInfo* commInfo = _engine->get_commodity_info(stdCode);
	if (commInfo == NULL)
	{
		log_error("Cannot find corresponding commodity info of {}", stdCode);
		return;
	}

	WTSSessionInfo* sInfo = commInfo->getSessionInfo();
	uint32_t offTime = sInfo->offsetTime(_engine->get_min_time(), true);
	bool isLastBarOfDay = (offTime == sInfo->getCloseTime(true));

	//Read the available positions, if it is the closing bar, read all positions directly
	double curQty = stra_get_position(stdCode, !isLastBarOfDay);
	if (decimal::le(curQty, 0))
		return;
	
	if (decimal::eq(limitprice, 0.0) && decimal::eq(stopprice, 0.0))	//If it is not a dynamic order placement mode, it will be triggered directly
	{
		double maxQty = min(curQty, qty);
		double totalQty = stra_get_position(stdCode, false);
		append_signal(stdCode, totalQty - maxQty, userTag, _is_in_schedule ? 0 : 1);
	}
	else
	{
		CondList& condList = get_cond_entrusts(stdCode);

		CondEntrust entrust;
		wt_strcpy(entrust._code, stdCode);
		wt_strcpy(entrust._usertag, userTag);

		entrust._qty = qty;
		entrust._field = WCF_NEWPRICE;
		if (!decimal::eq(limitprice))
		{
			entrust._target = limitprice;
			entrust._alg = WCT_LargerOrEqual;
		}
		else if (!decimal::eq(stopprice))
		{
			entrust._target = stopprice;
			entrust._alg = WCT_SmallerOrEqual;
		}

		entrust._action = COND_ACTION_CL;

		condList.emplace_back(entrust);
	}
}

void CtaStraBaseCtx::stra_exit_short(const char* stdCode, double qty, const char* userTag /* = "" */, double limitprice, double stopprice)
{
	WTSCommodityInfo* commInfo = _engine->get_commodity_info(stdCode);
	if (commInfo == NULL)
	{
		log_error("Cannot find corresponding commodity info of {}", stdCode);
		return;
	}

	if (!commInfo->canShort())
	{
		log_error("Cannot short on {}", stdCode);
		return;
	}

	double curQty = stra_get_position(stdCode);
	//If the position is long, there is no need to execute the logic of exiting the short position
	if (decimal::ge(curQty, 0))
		return;
	
	if (decimal::eq(limitprice, 0.0) && decimal::eq(stopprice, 0.0))	//If it is not a dynamic order placement mode, it will be triggered directly
	{
		double maxQty = min(abs(curQty), qty);
		append_signal(stdCode, curQty + maxQty, userTag, _is_in_schedule ? 0 : 1);
	}
	else
	{
		CondList& condList = get_cond_entrusts(stdCode);

		CondEntrust entrust;
		wt_strcpy(entrust._code, stdCode);
		wt_strcpy(entrust._usertag, userTag);

		entrust._qty = qty;
		entrust._field = WCF_NEWPRICE;
		if (!decimal::eq(limitprice))
		{
			entrust._target = limitprice;
			entrust._alg = WCT_SmallerOrEqual;
		}
		else if (!decimal::eq(stopprice))
		{
			entrust._target = stopprice;
			entrust._alg = WCT_LargerOrEqual;
		}

		entrust._action = COND_ACTION_CS;
		
		condList.emplace_back(entrust);
	}
}

double CtaStraBaseCtx::stra_get_price(const char* stdCode)
{
	auto it = _price_map.find(stdCode);
	if (it != _price_map.end())
		return it->second;

	if (_engine)
		return _engine->get_cur_price(stdCode);
	
	return 0.0;
}

double CtaStraBaseCtx::stra_get_day_price(const char* stdCode, int flag /* = 0 */)
{
	if (_engine)
		return _engine->get_day_price(stdCode, flag);

	return 0.0;
}

void CtaStraBaseCtx::stra_set_position(const char* stdCode, double qty, const char* userTag /* = "" */, double limitprice /* = 0.0 */, double stopprice /* = 0.0 */)
{
	_engine->sub_tick(id(), stdCode);

	if (decimal::eq(limitprice, 0.0) && decimal::eq(stopprice, 0.0))	//If it is not a dynamic order placement mode, it will be triggered directly
	{
		append_signal(stdCode, qty, userTag, _is_in_schedule ? 0 : 1);
	}
	else
	{
		CondList& condList = get_cond_entrusts(stdCode);

		double curVol = stra_get_position(stdCode);
		//If the target position is the same as the current position, the conditional order will not be set
		if (decimal::eq(curVol, qty))
			return;

		//According to the target position and the current position, determine whether to buy or sell
		bool isBuy = decimal::gt(qty, curVol);

		CondEntrust entrust;
		wt_strcpy(entrust._code, stdCode);
		wt_strcpy(entrust._usertag, userTag);

		entrust._qty = qty;
		entrust._field = WCF_NEWPRICE;
		if (!decimal::eq(limitprice))
		{
			entrust._target = limitprice;
			entrust._alg = isBuy ? WCT_SmallerOrEqual : WCT_LargerOrEqual;
		}
		else if (!decimal::eq(stopprice))
		{
			entrust._target = stopprice;
			entrust._alg = isBuy ? WCT_LargerOrEqual : WCT_SmallerOrEqual;
		}

		entrust._action = COND_ACTION_SP;

		condList.emplace_back(entrust);
	}
}

void CtaStraBaseCtx::append_signal(const char* stdCode, double qty, const char* userTag /* = "" */, uint32_t sigType)
{
	double curPx = _price_map[stdCode];

	SigInfo& sInfo = _sig_map[stdCode];
	sInfo._volume = qty;
	sInfo._sigprice = curPx;
	sInfo._usertag = userTag;
	sInfo._gentime = (uint64_t)_engine->get_date() * 1000000000 + (uint64_t)_engine->get_raw_time() * 100000 + _engine->get_secs();
	sInfo._sigtype = sigType;

	log_signal(stdCode, qty, curPx, sInfo._gentime, userTag);

	save_data();
}

void CtaStraBaseCtx::do_set_position(const char* stdCode, double qty, const char* userTag /* = "" */, bool bFireAtOnce /* = false */)
{
	PosInfo& pInfo = _pos_map[stdCode];
	double curPx = _price_map[stdCode];
	uint64_t curTm = (uint64_t)_engine->get_date() * 10000 + _engine->get_min_time();
	uint32_t curTDate = _engine->get_trading_date();

	if (decimal::eq(pInfo._volume, qty))
		return;

	double diff = qty - pInfo._volume;

	WTSCommodityInfo* commInfo = _engine->get_commodity_info(stdCode);
	if (commInfo == NULL)
		return;

	// Price of the transaction
	double trdPx = curPx;
	/* By HeJ @ 2023.03.14
	 * When holding a position theoretically, a lock should be added to avoid the combination of netting synchronization and signal triggering at the same time, 
	 * resulting in repeated orders and signal coverage
	 */
	SpinLock lock(_mutex);
	bool isBuy = decimal::gt(diff, 0.0);
	if (decimal::gt(pInfo._volume*diff, 0))
	{//The current position is consistent with the direction of position change, add a detail, just increase the quantity
		pInfo._volume = qty;

		//If T+1, the frozen position should be increased
		if (commInfo->isT1())
		{
			//ASSERT(diff>0);
			pInfo._frozen += diff;
			pInfo._frozen_date = curTDate;
			log_debug("{} frozen position updated to {}", stdCode, pInfo._frozen);
		}

		if (_slippage != 0)
		{
			trdPx += _slippage * commInfo->getPriceTick()*(isBuy ? 1 : -1);
		}

		DetailInfo dInfo;
		dInfo._long = decimal::gt(qty, 0);
		dInfo._price = trdPx;
		dInfo._max_price = trdPx;
		dInfo._min_price = trdPx;
		dInfo._volume = abs(diff);
		dInfo._opentime = curTm;
		dInfo._opentdate = curTDate;
		dInfo._open_barno = _last_barno;
		wt_strcpy(dInfo._opentag, userTag);
		pInfo._details.emplace_back(dInfo);
		pInfo._last_entertime = curTm;

		//double fee = _engine->calc_fee(stdCode, trdPx, abs(diff), 0);
		double fee = commInfo->calcFee(trdPx, abs(diff), 0);
		_fund_info._total_fees += fee;
		log_trade(stdCode, dInfo._long, true, curTm, trdPx, abs(diff), userTag, fee, _last_barno);
	}
	else
	{//The direction of the position is inconsistent with the direction of the position change, and the position needs to be closed
		double left = abs(diff);

		if (_slippage != 0)
			trdPx += _slippage * commInfo->getPriceTick()*(isBuy ? 1 : -1);

		pInfo._volume = qty;
		if (decimal::eq(pInfo._volume, 0))
			pInfo._dynprofit = 0;
		uint32_t count = 0;
		for (auto it = pInfo._details.begin(); it != pInfo._details.end(); it++)
		{
			DetailInfo& dInfo = *it;
			if (decimal::eq(dInfo._volume, 0))
			{
				count++;
				continue;
			}

			double maxQty = min(dInfo._volume, left);
			if (decimal::eq(maxQty, 0))
				continue;

			dInfo._volume -= maxQty;
			left -= maxQty;

			if (decimal::eq(dInfo._volume, 0))
				count++;

			//Calculate the profit and loss of closing the position
			double profit = (trdPx - dInfo._price) * maxQty * commInfo->getVolScale();
			if (!dInfo._long)
				profit *= -1;
			pInfo._closeprofit += profit;

			//Floating profit and loss should also be scaled proportionally
			pInfo._dynprofit = pInfo._dynprofit*dInfo._volume / (dInfo._volume + maxQty);
			pInfo._last_exittime = curTm;
			_fund_info._total_profit += profit;

			//Calculate the handling fee
			//double fee = _engine->calc_fee(stdCode, trdPx, maxQty, dInfo._opentdate == curTDate ? 2 : 1);
			double fee = commInfo->calcFee(trdPx, maxQty, dInfo._opentdate == curTDate ? 2 : 1);
			_fund_info._total_fees += fee;

			//Write the closing record here
			log_close(stdCode, dInfo._long, dInfo._opentime, dInfo._price, curTm, trdPx, maxQty, profit, pInfo._closeprofit, dInfo._opentag, userTag, dInfo._open_barno, _last_barno);

			//Write the transaction record here
			log_trade(stdCode, dInfo._long, false, curTm, trdPx, maxQty, userTag, fee, _last_barno);

			if (decimal::eq(left,0))
				break;
		}

		//Need to clean up the details that have been closed
		while (count > 0)
		{
			auto it = pInfo._details.begin();
			pInfo._details.erase(it);
			count--;
		}

		//Finally, if there is any remaining, you need to reverse it
		if (decimal::gt(left, 0))
		{
			left = left * qty / abs(qty);

			//If T+1, the frozen position should be increased
			if (commInfo->isT1())
			{
				pInfo._frozen += left;
				pInfo._frozen_date = curTDate;
				log_debug("{} frozen position up to {}", stdCode, pInfo._frozen);
			}

			DetailInfo dInfo;
			dInfo._long = decimal::gt(qty, 0);
			dInfo._price = trdPx;
			dInfo._max_price = trdPx;
			dInfo._min_price = trdPx;
			dInfo._volume = abs(left);
			dInfo._opentime = curTm;
			dInfo._opentdate = curTDate;
			dInfo._open_barno = _last_barno;
			wt_strcpy(dInfo._opentag, userTag);
			pInfo._details.emplace_back(dInfo);
			pInfo._last_entertime = curTm;

			// should be a transaction record here
			double fee = commInfo->calcFee(trdPx, abs(left), 0);
			_fund_info._total_fees += fee;
			log_trade(stdCode, dInfo._long, true, curTm, trdPx, abs(left), userTag, fee, _last_barno);
		}
	}

	save_data();

	if (bFireAtOnce)	//If it is triggered by a conditional order, submit the change to the engine
	{
		_engine->handle_pos_change(_name.c_str(), stdCode, diff);
	}
}

WTSKlineSlice* CtaStraBaseCtx::stra_get_bars(const char* stdCode, const char* period, uint32_t count, bool isMain /* = false */)
{
	thread_local static char key[64] = { 0 };
	fmtutil::format_to(key, "{}#{}", stdCode, period);

	if (isMain)
	{
		if (_main_key.empty())
		{
			_main_key = key;
			log_debug("Main KBars confirmed: {}", key);
		}
		else if (_main_key != key)
		{
			log_error("Main KBars already confirmed");
			return NULL;
		}

		/*
		 *	By Wesley @ 2022.12.07
		 */
		_main_code = stdCode;
		_main_period = period;
	}

	thread_local static char basePeriod[2] = { 0 };
	basePeriod[0] = period[0];
	uint32_t times = 1;
	if (strlen(period) > 1)
		times = strtoul(period + 1, NULL, 10);

	WTSKlineSlice* kline = _engine->get_kline_slice(_context_id, stdCode, basePeriod, count, times);
	if(kline)
	{
		//If the K-line cannot be obtained, it means that there will be no closing event, so the local flag will not be updated
		bool isFirst = (_kline_tags.find(key) == _kline_tags.end());	//If the tag is not saved, it means that this K-line is pulled for the first time
		KlineTag& tag = _kline_tags[key];
		tag._closed = false;

		double lastClose = kline->at(-1)->close;
		_price_map[stdCode] = lastClose;

		if(isMain && isFirst && !_condtions.empty())
		{
			//If it is the first time to pull the main K-line, check the condition order trigger time
			bool isDay = basePeriod[0] == 'd';
			uint64_t lastBartime = isDay ? kline->at(-1)->date : kline->at(-1)->time;
			if(!isDay)
				lastBartime += 199000000000;

			//If the time of the last closed K-line is greater than the condition order setup time, it means the condition order has expired, so it needs to be cleared
			if(lastBartime > _last_cond_min)
			{
				log_info("Conditions expired, setup time: {}, time of last bar of main kbars: {}, all cleared", _last_cond_min, lastBartime);
				_condtions.clear();
			}
		}

		_engine->sub_tick(id(), stdCode);

		//If it is the main K-line, and the number of the last bar is 0
		//Then set the number of the last bar to the length of the main K-line
		if(isMain && _last_barno == 0)
		{
			_last_barno = kline->size();
		}
	}

	return kline;
}

WTSTickSlice* CtaStraBaseCtx::stra_get_ticks(const char* stdCode, uint32_t count)
{
	WTSTickSlice* ret = _engine->get_tick_slice(_context_id, stdCode, count);
	if (ret)
		_engine->sub_tick(id(), stdCode);

	return ret;
}

WTSTickData* CtaStraBaseCtx::stra_get_last_tick(const char* stdCode)
{
	return _engine->get_last_tick(_context_id, stdCode);
}

void CtaStraBaseCtx::stra_sub_ticks(const char* code)
{
	/*
	 *	By Wesley @ 2022.03.01
	 *	Record the subscription of tick data
	 *	When the tick data callback is triggered, check first
	 */
	_tick_subs.insert(code);

	_engine->sub_tick(_context_id, code);
	log_info("Market data subscribed: {}", code);
}

void CtaStraBaseCtx::stra_sub_bar_events(const char* stdCode, const char* period)
{
	thread_local static char key[64] = { 0 };
	fmtutil::format_to(key, "{}#{}", stdCode, period);

	KlineTag& tag = _kline_tags[key];
	tag._notify = true;
}

WTSCommodityInfo* CtaStraBaseCtx::stra_get_comminfo(const char* stdCode)
{
	return _engine->get_commodity_info(stdCode);
}

std::string CtaStraBaseCtx::stra_get_rawcode(const char* stdCode)
{
	return _engine->get_rawcode(stdCode);
}

uint32_t CtaStraBaseCtx::stra_get_tdate()
{
	return _engine->get_trading_date();
}

uint32_t CtaStraBaseCtx::stra_get_date()
{
	return _engine->get_date();
}

uint32_t CtaStraBaseCtx::stra_get_time()
{
	return _engine->get_min_time();
}

double CtaStraBaseCtx::stra_get_fund_data(int flag )
{
	switch (flag)
	{
	case 0:
		return _fund_info._total_profit - _fund_info._total_fees + _fund_info._total_dynprofit;
	case 1:
		return _fund_info._total_profit;
	case 2:
		return _fund_info._total_dynprofit;
	case 3:
		return _fund_info._total_fees;
	default:
		return 0.0;
	}
}

void CtaStraBaseCtx::stra_log_info(const char* message)
{
	WTSLogger::log_dyn_raw("strategy", _name.c_str(), LL_INFO, message);
}

void CtaStraBaseCtx::stra_log_debug(const char* message)
{
	WTSLogger::log_dyn_raw("strategy", _name.c_str(), LL_DEBUG, message);
}

void CtaStraBaseCtx::stra_log_warn(const char* message)
{
	WTSLogger::log_dyn_raw("strategy", _name.c_str(), LL_WARN, message);
}

void CtaStraBaseCtx::stra_log_error(const char* message)
{
	WTSLogger::log_dyn_raw("strategy", _name.c_str(), LL_ERROR, message);
}

const char* CtaStraBaseCtx::stra_load_user_data(const char* key, const char* defVal /*= ""*/)
{
	auto it = _user_datas.find(key);
	if (it != _user_datas.end())
		return it->second.c_str();

	return defVal;
}

void CtaStraBaseCtx::stra_save_user_data(const char* key, const char* val)
{
	_user_datas[key] = val;
	_ud_modified = true;
}

uint64_t CtaStraBaseCtx::stra_get_first_entertime(const char* stdCode)
{
	auto it = _pos_map.find(stdCode);
	if (it == _pos_map.end())
		return 0;

	const PosInfo& pInfo = it->second;
	if (pInfo._details.empty())
		return 0;

	return pInfo._details[0]._opentime;
}

const char* CtaStraBaseCtx::stra_get_last_entertag(const char* stdCode)
{
	auto it = _pos_map.find(stdCode);
	if (it == _pos_map.end())
		return "";

	const PosInfo& pInfo = it->second;
	if (pInfo._details.empty())
		return "";

	return pInfo._details[0]._opentag;
}


uint64_t CtaStraBaseCtx::stra_get_last_exittime(const char* stdCode)
{
	auto it = _pos_map.find(stdCode);
	if (it == _pos_map.end())
		return 0;

	const PosInfo& pInfo = it->second;
	return pInfo._last_exittime;
}

uint64_t CtaStraBaseCtx::stra_get_last_entertime(const char* stdCode)
{
	auto it = _pos_map.find(stdCode);
	if (it == _pos_map.end())
		return 0;

	const PosInfo& pInfo = it->second;
	if (pInfo._details.empty())
		return 0;

	return pInfo._details[pInfo._details.size() - 1]._opentime;
}

double CtaStraBaseCtx::stra_get_last_enterprice(const char* stdCode)
{
	auto it = _pos_map.find(stdCode);
	if (it == _pos_map.end())
		return 0;

	const PosInfo& pInfo = it->second;
	if (pInfo._details.empty())
		return 0;

	return pInfo._details[pInfo._details.size() - 1]._price;
}

double CtaStraBaseCtx::stra_get_position(const char* stdCode, bool bOnlyValid /* = false */, const char* userTag /* = "" */)
{
	double totalPos = 0;
	auto sit = _sig_map.find(stdCode);
	if(sit != _sig_map.end())
	{
		WTSLogger::warn("{} has untouched signal, [userTag] will be ignored", stdCode);
		totalPos = sit->second._volume;
	}

	auto it = _pos_map.find(stdCode);
	if (it == _pos_map.end())
		return totalPos;

	const PosInfo& pInfo = it->second;
	totalPos = pInfo._volume;
	if (strlen(userTag) == 0)
	{
		//only userTag is empty, bOnlyValid will be used
		if (bOnlyValid)
		{
			//In theory, only long positions will enter here
			//Other places have to ensure that _frozen is 0 for short positions
			return totalPos - pInfo._frozen;
		}
		else
			return totalPos;
	}
	else
	{
		for (auto it = pInfo._details.begin(); it != pInfo._details.end(); it++)
		{
			const DetailInfo& dInfo = (*it);
			if (strcmp(dInfo._opentag, userTag) != 0)
				continue;

			return dInfo._volume;
		}
	}

	return 0;
}

double CtaStraBaseCtx::stra_get_position_avgpx(const char* stdCode)
{
	auto it = _pos_map.find(stdCode);
	if (it == _pos_map.end())
		return 0;

	const PosInfo& pInfo = it->second;
	if (pInfo._volume == 0)
		return 0.0;

	double amount = 0.0;
	for (auto dit = pInfo._details.begin(); dit != pInfo._details.end(); dit++)
	{
		const DetailInfo& dInfo = *dit;
		amount += dInfo._price*dInfo._volume;
	}

	return amount / pInfo._volume;
}

double CtaStraBaseCtx::stra_get_position_profit(const char* stdCode)
{
	auto it = _pos_map.find(stdCode);
	if (it == _pos_map.end())
		return 0;

	const PosInfo& pInfo = it->second;
	return pInfo._dynprofit;
}

uint64_t CtaStraBaseCtx::stra_get_detail_entertime(const char* stdCode, const char* userTag)
{
	auto it = _pos_map.find(stdCode);
	if (it == _pos_map.end())
		return 0;

	const PosInfo& pInfo = it->second;
	for (auto it = pInfo._details.begin(); it != pInfo._details.end(); it++)
	{
		const DetailInfo& dInfo = (*it);
		if (strcmp(dInfo._opentag, userTag) != 0)
			continue;

		return dInfo._opentime;
	}

	return 0;
}

double CtaStraBaseCtx::stra_get_detail_cost(const char* stdCode, const char* userTag)
{
	auto it = _pos_map.find(stdCode);
	if (it == _pos_map.end())
		return 0;

	const PosInfo& pInfo = it->second;
	for (auto it = pInfo._details.begin(); it != pInfo._details.end(); it++)
	{
		const DetailInfo& dInfo = (*it);
		if (strcmp(dInfo._opentag, userTag) != 0)
			continue;

		return dInfo._price;
	}

	return 0.0;
}

double CtaStraBaseCtx::stra_get_detail_profit(const char* stdCode, const char* userTag, int flag /* = 0 */)
{
	auto it = _pos_map.find(stdCode);
	if (it == _pos_map.end())
		return 0;

	const PosInfo& pInfo = it->second;
	for (auto it = pInfo._details.begin(); it != pInfo._details.end(); it++)
	{
		const DetailInfo& dInfo = (*it);
		if (strcmp(dInfo._opentag, userTag) != 0)
			continue;

		switch (flag)
		{
		case 0:
			return dInfo._profit;
		case 1:
			return dInfo._max_profit;
		case -1:
			return dInfo._max_loss;
		case 2:
			return dInfo._max_price;
		case -2:
			return dInfo._min_price;
		}
	}

	return 0.0;
}

void CtaStraBaseCtx::set_chart_kline(const char* stdCode, const char* period)
{
	_chart_code = stdCode;
	_chart_period = period;
}

void CtaStraBaseCtx::add_chart_mark(double price, const char* icon, const char* tag)
{
	if (!_is_in_schedule)
	{
		WTSLogger::error("Marks can be added only during schedule");
		return;
	}

	uint64_t curTime = stra_get_date();
	curTime = curTime * 10000 + stra_get_time();

	if (_mark_logs)
	{
		std::stringstream ss;
		ss << curTime << "," << price << "," << icon << "," << tag << std::endl;;
		_mark_logs->write_file(ss.str());
	}

	_engine->notify_chart_marker(curTime, _name.c_str(), price, icon, tag);
}

void CtaStraBaseCtx::register_index(const char* idxName, uint32_t indexType)
{
	ChartIndex& cIndex = _chart_indice[idxName];
	cIndex._name = idxName;
	cIndex._indexType = indexType;
}

bool CtaStraBaseCtx::register_index_line(const char* idxName, const char* lineName, uint32_t lineType)
{
	auto it = _chart_indice.find(idxName);
	if (it == _chart_indice.end())
	{
		WTSLogger::error("Index {} not registered", idxName);
		return false;
	}

	ChartIndex& cIndex = (ChartIndex&)it->second;
	ChartLine& cLine = cIndex._lines[lineName];
	cLine._name = lineName;
	cLine._lineType = lineType;
	return true;
}

bool CtaStraBaseCtx::add_index_baseline(const char* idxName, const char* lineName, double val)
{
	auto it = _chart_indice.find(idxName);
	if (it == _chart_indice.end())
	{
		WTSLogger::error("Index {} not registered", idxName);
		return false;
	}

	ChartIndex& cIndex = (ChartIndex&)it->second;
	cIndex._base_lines[lineName] = val;
	return true;
}

bool CtaStraBaseCtx::set_index_value(const char* idxName, const char* lineName, double val)
{
	if (!_is_in_schedule)
	{
		WTSLogger::error("Marks can be added only during schedule");
		return false;
	}

	auto ait = _chart_indice.find(idxName);
	if (ait == _chart_indice.end())
	{
		WTSLogger::error("Index {} not registered", idxName);
		return false;
	}

	ChartIndex& cIndex = (ChartIndex&)ait->second;
	auto bit = cIndex._lines.find(lineName);
	if (bit == cIndex._lines.end())
	{
		WTSLogger::error("Line {} of index {} not registered", lineName, idxName);
		return false;
	}

	uint64_t curTime = stra_get_date();
	curTime = curTime * 10000 + stra_get_time();

	if (_idx_logs)
	{
		std::stringstream ss;
		ss << curTime << "," << idxName << "," << lineName << "," << val << std::endl;;
		_idx_logs->write_file(ss.str());
	}

	_engine->notify_chart_index(curTime, _name.c_str(), idxName, lineName, val);

	return true;
}

