#pragma once

#include <boost/locale.hpp>
#include <utf8cpp/utf8.h>
#include <string>

namespace quanttrader {

class EncodingHelper {
public:
    static bool is_ascii(const std::string_view str) {
        return std::all_of(str.begin(), str.end(), [](unsigned char c) { return 0 <= c && c <= 127; });
    }

    static bool is_valid_utf8(const std::string_view str) {
        return utf8::is_valid(str.begin(), str.end());
    }

    static std::string locale_to_utf8(const std::string& locale_str, const std::locale& loc) {
        try {
            return boost::locale::conv::to_utf<char>(locale_str, loc);
        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to convert locale to UTF-8: " + std::string(e.what()));
        }
    }

	static bool is_likely_gbk(const unsigned char* data, std::size_t len) {
		std::size_t i = 0;
		while (i < len) {
			if (data[i] <= 0x7f) {
				// The code is less than or equal to 127, only one byte code, compatible with ASCII
				i++;
				continue;
			}
			else {
				// Greater than 127 uses double-byte encoding
				if (data[i] >= 0x81 &&
					data[i] <= 0xfe &&
					data[i + 1] >= 0x40 &&
					data[i + 1] <= 0xfe &&
					data[i + 1] != 0xf7) 
				{
					// If there is GBK encoding, consider the entire string as GBK encoded
					return true;
				}
			}
		}
		return false;
	}

    static bool is_likely_gbk(const std::string_view str) {
        return is_likely_gbk(reinterpret_cast<const unsigned char*>(str.data()), str.size());
    }
};
}