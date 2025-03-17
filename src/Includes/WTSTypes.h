/*!
 * \file WTSTypes.h
 * \project	WonderTrader
 *
 * \author Wesley
 * \date 2020/03/30
 * 
 * \brief WonderTrader basic data type definition file
 */
#pragma once
#include "WTSMarcos.h"
#include <stdint.h>

NS_WTP_BEGIN

/*
 *	Contract classification
 */
//Copied from CTP
typedef enum tagContractCategory
{
	CC_Stock,			//Stock
	CC_Future,			//Future
	CC_FutOption,		//Future option, commodity option is this classification
	CC_Combination,		//Combination
	CC_Spot,			//Spot
	CC_EFP,				//Exchange for physical
	CC_SpotOption,		//Spot option, stock index option is this classification
	CC_ETFOption,		//Stock option, ETF option is this classification

	CC_DC_Spot	= 20,	//Digital currency spot
	CC_DC_Swap,			//Digital currency perpetual
	CC_DC_Future,		//Digital currency future
	CC_DC_Margin,		//Digital currency margin
	CC_DC_Option,		//Digital currency option

	CC_UserIndex = 90	//Custom index
} ContractCategory;

/*
 *	Option type
 */
typedef enum tagOptionType
{
	OT_None = 0,
	OT_Call = '1',		//Call option
	OT_Put	= '2'		//Put option
} OptionType;

/*
 *	Cover mode
 */
typedef enum tagCoverMode
{
	CM_OpenCover,		//Open cover
	CM_CoverToday,		//Open cover today
	CM_UNFINISHED,		//Unfinished
	CM_None			//No distinction between open and cover
} CoverMode;

/*
 *	Trading mode
 */
typedef enum tagTradingMode
{
	TM_Both,	//Both long and short supported
	TM_Long,	//Only long
	TM_LongT1,	//Long T+1
	TM_None = 9	//Cannot trade
} TradingMode;

/*
*	Price mode
*/
typedef enum tagPriceMode
{
	PM_Both,		//Both market and limit supported
	PM_Limit,		//Only limit supported
	PM_Market,		//Only market supported
	PM_None	= 9		//Not supported
} PriceMode;

/*
 *	Kline data type
 *	Open, High, Low, Close, Volume, Amount, Date, Time
 */
typedef enum tagKlineFieldType
{
	KFT_OPEN,
	KFT_HIGH,
	KFT_LOW,
	KFT_CLOSE,
	KFT_DATE,
	KFT_TIME,
	KFT_VOLUME,
	KFT_SVOLUME
} WTSKlineFieldType;

/*
 *	Kline period
 */
typedef enum tagKlinePeriod
{
	KP_Tick,
	KP_Minute1,
	KP_Minute5,
	KP_DAY,
	KP_Week,
	KP_Month
} WTSKlinePeriod;

static const char* PERIOD_NAME[] = 
{
	"tick",
	"min1",
	"min5",
	"day",
	"week",
	"month"
};

/*
 *	Log level
 */
typedef enum tagLogLevel
{
	LL_ALL	= 100,
	LL_DEBUG,
	LL_INFO,
	LL_WARN,
	LL_ERROR,
	LL_FATAL,
	LL_NONE
} WTSLogLevel;

/*
 *	Price type
 */
typedef enum tagPriceType
{
	WPT_ANYPRICE	= 0,			//Market order
	WPT_LIMITPRICE,					//Limit order
	WPT_BESTPRICE,					//Best price
	WPT_LASTPRICE,					//Last price

	//////////////////////////////////////////////////////////////////////////
	//The following are aligned with CTP price types
	WPT_CTP_LASTPLUSONETICKS = 20,	//Last price +1 ticks
	WPT_CTP_LASTPLUSTWOTICKS,		//Last price +2 ticks
	WPT_CTP_LASTPLUSTHREETICKS,		//Last price +3 ticks
	WPT_CTP_ASK1,					//Ask price 1
	WPT_CTP_ASK1PLUSONETICKS,		//Ask price 1 +1 ticks
	WPT_CTP_ASK1PLUSTWOTICKS,		//Ask price 1 +2 ticks
	WPT_CTP_ASK1PLUSTHREETICKS,		//Ask price 1 +3 ticks
	WPT_CTP_BID1,					//Bid price 1
	WPT_CTP_BID1PLUSONETICKS,		//Bid price 1 +1 ticks
	WPT_CTP_BID1PLUSTWOTICKS,		//Bid price 1 +2 ticks
	WPT_CTP_BID1PLUSTHREETICKS,		//Bid price 1 +3 ticks
	WPT_CTP_FIVELEVELPRICE,			//Five level price, CFFEX market price

	//////////////////////////////////////////////////////////////////////////
	//The following are aligned with DC price types
	WPT_DC_POSTONLY	= 100,			//Post only
	WPT_DC_FOK,						//Fill or kill
	WPT_DC_IOC,						//Immediate or cancel
	WPT_DC_OPTLIMITIOC				//Market order immediate or cancel
} WTSPriceType;

/*
 *	Time condition
 */
typedef enum tagTimeCondition
{
	WTC_IOC		= '1',	//Immediate or cancel
	WTC_GFS,			//Good for session
	WTC_GFD,			//Good for day
} WTSTimeCondition;

/*
 *	Order flag
 */
typedef enum tagOrderFlag
{
	WOF_NOR = '0',		//Normal order
	WOF_FAK,			//FAK
	WOF_FOK,			//FOK
} WTSOrderFlag;

/*
 *	Offset type
 */
typedef enum tagOffsetType
{
	WOT_OPEN			= '0',	//Open
	WOT_CLOSE,					//Close, SHFE for close yesterday
	WOT_FORCECLOSE,				//Force close
	WOT_CLOSETODAY,				//Close today
	WOT_CLOSEYESTERDAY,			//Close yesterday
} WTSOffsetType;

/*
 *	Direction type
 */
typedef enum tagDirectionType
{
	WDT_LONG			= '0',	//Long
	WDT_SHORT,					//Short
	WDT_NET						//Net
} WTSDirectionType;

/*
 *	Business type
 */
typedef enum tagBusinessType
{
	BT_CASH		= '0',	//Cash trading
	BT_ETF		= '1',	//ETF subscription and redemption
	BT_EXECUTE	= '2',	//Option exercise
	BT_QUOTE	= '3',	//Option quote
	BT_FORQUOTE = '4',	//Option inquiry
	BT_FREEZE	= '5',	//Option lock
	BT_CREDIT	= '6',	//Margin trading
	BT_UNKNOWN			//Unknown business type
} WTSBusinessType;

/*
 *	Action flag
 */
typedef enum tagActionFlag
{
	WAF_CANCEL			= '0',	//Cancel
	WAF_MODIFY			= '3',	//Modify
} WTSActionFlag;

/*
 *	Order state
 */
typedef enum tagOrderState
{
	WOS_AllTraded				= '0',	//All traded
	WOS_PartTraded_Queuing,				//Partially traded, still queuing
	WOS_PartTraded_NotQueuing,			//Partially traded, not queuing
	WOS_NotTraded_Queuing,				//Not traded, queuing
	WOS_NotTraded_NotQueuing,			//Not traded, not queuing
	WOS_Canceled,						//Canceled
	WOS_Submitting				= 'a',	//Submitting
	WOS_Cancelling,						//Cancelling
	WOS_Nottouched,						//Not touched
} WTSOrderState;

/*
 *	Order type
 */
typedef enum tagOrderType
{
	WORT_Normal			= 0,		//Normal order
	WORT_Exception,					//Exception order
	WORT_System,					//System order
	WORT_Hedge						//Hedge order
} WTSOrderType;

/*
 *	Trade type
 */
typedef enum tagTradeType
{
	WTT_Common				= '0',	//Common
	WTT_OptionExecution		= '1',	//Option execution
	WTT_OTC					= '2',	//OTC trade
	WTT_EFPDerived			= '3',	//EFP derived trade
	WTT_CombinationDerived	= '4'	//Combination derived trade
} WTSTradeType;


/*
 *	Error code
 */
typedef enum tagErrorCode
{
	WEC_NONE			=	0,		//No error
	WEC_ORDERINSERT,				//Order insert error
	WEC_ORDERCANCEL,				//Order cancel error
	WEC_EXECINSERT,					//Execution insert error
	WEC_EXECCANCEL,					//Execution cancel error
	WEC_UNKNOWN			=	9999	//Unknown error
} WTSErroCode;

/*
 *	Compare field
 */
typedef enum tagCompareField
{
	WCF_NEWPRICE			=	0,	//New price
	WCF_BIDPRICE,					//Bid price
	WCF_ASKPRICE,					//Ask price
	WCF_PRICEDIFF,					//Price difference, for stop profit and stop loss
	WCF_NONE				=	9	//No comparison
} WTSCompareField;

/*
 *	Compare type
 */
typedef enum tagCompareType
{
	WCT_Equal			= 0,		//Equal
	WCT_Larger,						//Larger
	WCT_Smaller,					//Smaller
	WCT_LargerOrEqual,				//Larger or equal
	WCT_SmallerOrEqual				//Smaller or equal
}WTSCompareType;

/*
 *	Market data parser event
 */
typedef enum tagParserEvent
{
	WPE_Connect			= 0,		//Connect event
	WPE_Close,						//Close event
	WPE_Login,						//Login
	WPE_Logout						//Logout
}WTSParserEvent;

/*
 *	Trader event
 */
typedef enum tagTraderEvent
{
	WTE_Connect			= 0,		//Connect event
	WTE_Close,						//Close event
	WTE_Login,						//Login
	WTE_Logout						//Logout
}WTSTraderEvent;

/*
 *	Trade status
 */
typedef enum tagTradeStatus
{
	TS_BeforeTrading	= '0',	//Before trading
	TS_NotTrading		= '1',	//Not trading
	TS_Continous		= '2',	//Continuous auction
	TS_AuctionOrdering	= '3',	//Auction ordering
	TS_AuctionBalance	= '4',	//Auction balance
	TS_AuctionMatch		= '5',	//Auction match
	TS_Closed			= '6'	//Closed
}WTSTradeStatus;

/*
 *	Buy and sell direction type
 */
typedef uint32_t WTSBSDirectType;
#define BDT_Buy		'B'	//Buy	
#define BDT_Sell	'S'	//Sell
#define BDT_Unknown ' '	//Unknown
#define BDT_Borrow	'G'	//Borrow
#define BDT_Lend	'F'	//Lend

/*
 *	Transaction type
 */
typedef uint32_t WTSTransType;
#define TT_Unknown	'U'	//Unknown type
#define TT_Match	'M'	//Match
#define TT_Cancel	'C'	//Cancel

/*
 *	Order detail type
 */
typedef uint32_t WTSOrdDetailType;
#define ODT_Unknown		0	//Unknown type
#define ODT_BestPrice	'U'	//Best price
#define ODT_AnyPrice	'1'	//Market price
#define ODT_LimitPrice	'2'	//Limit price

NS_WTP_END