#include "kobold.h"

#include "character/challenge_rating.h"

#include "character/character.h"
#include "character/character_game_data.h"
#include "database/defines.h"
#include "util/assert_util.h"
#include "util/string_util.h"

namespace kobold {

challenge_rating challenge_rating::get_challenge_rating_for_experience_award(const int64_t experience_award, const challenge_rating &base_challenge_rating)
{
	challenge_rating challenge_rating = base_challenge_rating;
	kobold::challenge_rating previous_challenge_rating = challenge_rating;
	while (defines::get()->get_experience_award_for_challenge_rating(challenge_rating) < experience_award) {
		previous_challenge_rating = challenge_rating;
		challenge_rating.change(1);
	}

	const int64_t prev_diff = std::abs(experience_award - defines::get()->get_experience_award_for_challenge_rating(previous_challenge_rating));
	const int64_t diff = std::abs(experience_award - defines::get()->get_experience_award_for_challenge_rating(challenge_rating));

	if (prev_diff < diff) {
		return previous_challenge_rating;
	} else {
		return challenge_rating;
	}
}

challenge_rating challenge_rating::get_group_challenge_rating(const std::vector<const character *> &characters)
{
	assert_throw(!characters.empty());

	if (characters.size() == 1) {
		return characters.at(0)->get_game_data()->get_challenge_rating();
	}

	int64_t group_experience_award = 0;

	for (const character *character : characters) {
		group_experience_award += defines::get()->get_experience_award_for_challenge_rating(character->get_game_data()->get_challenge_rating());
	}

	return challenge_rating::get_challenge_rating_for_experience_award(group_experience_award, characters.at(0)->get_game_data()->get_challenge_rating());
}

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
