/*!
 * \file EventCaster.cpp
 * \project	WonderTrader
 *
 * \author Wesley
 * \date 2020/03/30
 * 
 * \brief 
 */
#include "EventNotifier.h"
#include "WtHelper.h"

#include "../Share/TimeUtils.hpp"
#include "../Share/DLLHelper.hpp"

#include "../Includes/WTSVariant.hpp"
#include "../WTSTools/WTSLogger.h"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
namespace rj = rapidjson;

USING_NS_WTP;

void on_mq_log(unsigned long id, const char* message, bool bServer)
{

}

EventNotifier::EventNotifier()
	: _mq_sid(0)
{
	
}


EventNotifier::~EventNotifier()
{
	if (_remover && _mq_sid != 0)
		_remover(_mq_sid);
}

bool EventNotifier::init(WTSVariant* cfg)
{
	if (!cfg->getBoolean("active"))
		return false;

	m_strURL = cfg->getCString("url");
	std::string module = DLLHelper::wrap_module("WtMsgQue", "lib");
	/*First check if there is a corresponding module in the working directory*/f the corresponding module is in the working directory
	std::string dllpath = WtHelper::getCWD() + module;
	/*If not, then look at the module directory, that is, under the same directory as dll*/e module directory, i.e., the same directory as the dll
	if (!StdFile::exists(dllpath.c_str()))
		dllpath = WtHelper::getInstDir() + module;

	DllHandle dllInst = DLLHelper::load_library(dllpath.c_str());
	if (dllInst == NULL)
	{
		WTSLogger::error("MQ module %{} loading failed", dllpath.c_str());
		return false;
	}

	_creator = (FuncCreateMQServer)DLLHelper::get_symbol(dllInst, "create_server");
	if (_creator == NULL)
	{
		DLLHelper::free_library(dllInst);
		WTSLogger::error("MQ module {} is not compatible", dllpath.c_str());
		return false;
	}

	_remover = (FuncDestroyMQServer)DLLHelper::get_symbol(dllInst, "destroy_server");
	_publisher = (FundPublishMessage)DLLHelper::get_symbol(dllInst, "publish_message");
	_register = (FuncRegCallbacks)DLLHelper::get_symbol(dllInst, "regiter_callbacks");

	/*Register callback function*/er callback function
	_register(on_mq_log);
	
	/*Create a MQServer*/Server
	_mq_sid = _creator(m_strURL.c_str(), true);

	WTSLogger::info("EventNotifier initialized with channel {}", m_strURL.c_str());

	return true;
}

void EventNotifier::notifyEvent(const char* evtType)
{
	if (_publisher)
		_publisher(_mq_sid, "BT_EVENT", evtType, (unsigned long)strlen(evtType));
}

void EventNotifier::notifyData(const char* topic, void* data , uint32_t dataLen)
{
	if (_publisher)
		_publisher(_mq_sid, topic, (const char*)data, dataLen);
}

void EventNotifier::notifyFund(const char* topic, uint32_t uDate, double total_profit, double dynprofit, double dynbalance, double total_fee)
{
	std::string output;
	{
		rj::Document root(rj::kObjectType);
		rj::Document::AllocatorType &allocator = root.GetAllocator();

		root.AddMember("date", uDate, allocator);
		root.AddMember("total_profit", total_profit, allocator);
		root.AddMember("dynprofit", dynprofit, allocator);
		root.AddMember("dynbalance", dynbalance, allocator);
		root.AddMember("total_fee", total_fee, allocator);

		rj::StringBuffer sb;
		rj::PrettyWriter<rj::StringBuffer> writer(sb);
		root.Accept(writer);

		output = sb.GetString();
	}

	if (_publisher)
		_publisher(_mq_sid, topic, (const char*)output.c_str(), (unsigned long)output.size());
}
