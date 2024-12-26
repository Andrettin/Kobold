#pragma once

namespace kobold {

class challenge_rating final
{
public:
	static challenge_rating get_group_challenge_rating(const std::vector<const character *> &characters);

	challenge_rating()
	{
	}

	explicit challenge_rating(const std::string &str);

	std::string to_string() const;

	void change(const int change)
	{
		this->value += change;
	}

	int get_value() const
	{
		return this->value;
	}

	constexpr bool operator ==(const challenge_rating &other) const
	{
		return this->value == other.value;
	}

	constexpr bool operator <(const challenge_rating &other) const
	{
		return this->value < other.value;
	}

private:
	int value = 0;
};

}
