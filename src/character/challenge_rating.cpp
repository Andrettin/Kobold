#include "kobold.h"

#include "character/challenge_rating.h"

#include "util/assert_util.h"
#include "util/string_util.h"

namespace kobold {

challenge_rating::challenge_rating(const std::string &str)
{
	static const std::map<std::string, int> string_to_challenge_rating_values{
		{ "1/10", -5 },
		{ "1/8", -4 },
		{ "1/6", -3 },
		{ "1/4", -2 },
		{ "1/3", -1 },
		{ "1/2", 0 }
	};

	if (string::is_number(str)) {
		this->value = std::stoi(str);
	} else {
		const auto find_iterator = string_to_challenge_rating_values.find(str);
		assert_throw(find_iterator != string_to_challenge_rating_values.end());
		this->value = find_iterator->second;
	}
}

std::string challenge_rating::to_string() const
{
	switch (this->value) {
		case -5:
			return "1/10";
		case -4:
			return "1/8";
		case -3:
			return "1/6";
		case -2:
			return "1/4";
		case -1:
			return "1/3";
		case 0:
			return "1/2";
		default:
			assert_throw(this->value > 0);
			return std::to_string(this->value);
	}
}

}
