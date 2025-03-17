#pragma once

#ifndef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY
#endif
#include <fmt/format.h>
#include "../Includes/WTSTypes.h"
#include <atomic>

// Add formatter specialization for WTSKlinePeriod enum
template<>
struct fmt::formatter<wtp::WTSKlinePeriod> : formatter<const char*> {
    auto format(wtp::WTSKlinePeriod period, format_context& ctx) const {
        const char* period_name = "";
        switch (period) {
            case wtp::KP_Tick:   period_name = "tick"; break;
            case wtp::KP_Minute1: period_name = "min1"; break;
            case wtp::KP_Minute5: period_name = "min5"; break;
            case wtp::KP_DAY:     period_name = "day"; break;
            case wtp::KP_Week:    period_name = "week"; break;
            case wtp::KP_Month:   period_name = "month"; break;
            default:             period_name = "unknown"; break;
        }
        return formatter<const char*>::format(period_name, ctx);
    }
};

// Add formatter specialization for std::atomic types
template <typename T>
struct fmt::formatter<std::atomic<T>> : formatter<T> {
    auto format(const std::atomic<T>& value, format_context& ctx) const {
        return formatter<T>::format(value.load(), ctx);
    }
};

namespace fmtutil
{
	template<typename... Args>
	inline char* format_to(char* buffer, const char* format, const Args& ...args)
	{
		char* s = fmt::format_to(buffer, fmt::runtime(format), args...);
		s[0] = '\0';
		return s;
	}

	template<int BUFSIZE=512, typename... Args>
	inline const char* format(const char* format, const Args& ...args)
	{
		thread_local static char buffer[BUFSIZE];
		char* s = fmt::format_to(buffer, fmt::runtime(format), args...);
		s[0] = '\0';
		return buffer;
	}
}
