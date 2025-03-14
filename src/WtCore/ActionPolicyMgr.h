/*!
 * \file ActionPolicyMgr.h
 * \project	WonderTrader
 *
 * \author Wesley
 * \date 2020/03/30
 * 
 * \brief 
 */
#pragma once
#include <vector>
#include <stdint.h>
#include <string.h>

#include "../Includes/FasterDefs.h"


NS_WTP_BEGIN
class WTSVariant;

typedef enum tagActionType
{
	AT_Unknown = 8888,
	AT_Open = 9999,		// Open
	AT_Close,			// Close
	AT_CloseToday,		// CloseToday
	AT_CloseYestoday	// CloseYestoday
} ActionType;

typedef struct _ActionRule
{
	ActionType	_atype;		// Action Type
	uint32_t	_limit;		// Limit
	uint32_t	_limit_l;	// Long Limit
	uint32_t	_limit_s;	// Short Limit
	bool		_pure;		// Used to determine whether it is a net today or net yesterday for AT_CloseToday and AT_CloseYestoday

	_ActionRule()
	{
		memset(this, 0, sizeof(_ActionRule));
	}
} ActionRule;

typedef std::vector<ActionRule>	ActionRuleGroup;

class ActionPolicyMgr
{
public:
	ActionPolicyMgr();
	~ActionPolicyMgr();

public:
	bool init(const char* filename);

	const ActionRuleGroup& getActionRules(const char* pid);

private:
	typedef wt_hashmap<std::string, ActionRuleGroup> RulesMap;
	RulesMap	_rules;

	wt_hashmap<std::string, std::string> _comm_rule_map;	// Corresponing to the rules of the variety
};

NS_WTP_END
