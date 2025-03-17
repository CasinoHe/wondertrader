/*!
 * \file ITrdNotifySink.h
 * \project	WonderTrader
 *
 * \author Wesley
 * \date 2020/03/30
 * 
 * \brief 
 */
#pragma once
#include <stdint.h>
#include "../Includes/WTSMarcos.h"

NS_WTP_BEGIN

class ITrdNotifySink
{
public:
	/*
	 *	鎴愪氦鍥炴姤
	 */
	virtual void on_trade(uint32_t localid, const char* stdCode, bool isBuy, double vol, double price) = 0;

	/*
	 *	璁㈠崟鍥炴姤
	 */
	virtual void on_order(uint32_t localid, const char* stdCode, bool isBuy, double totalQty, double leftQty, double price, bool isCanceled = false) = 0;

	/*
	 *	鎸佷粨鏇存柊鍥炶皟
	 */
	virtual void on_position(const char* stdCode, bool isLong, double prevol, double preavail, double newvol, double newavail, uint32_t tradingday) {}

	/*
	 *	浜ゆ槗閫氶亾灏辩华
	 */
	virtual void on_channel_ready() = 0;

	/*
	 *	浜ゆ槗閫氶亾涓㈠け
	 */
	virtual void on_channel_lost() = 0;

	/*
	 *	涓嬪崟鍥炴姤
	 */
	virtual void on_entrust(uint32_t localid, const char* stdCode, bool bSuccess, const char* message){}

	/*
	 *	璧勯噾鍥炶皟
	 */
	virtual void on_account(const char* currency, double prebalance, double balance, double dynbalance, double avaliable, double closeprofit, double dynprofit, double margin, double fee, double deposit, double withdraw){}
};

NS_WTP_END