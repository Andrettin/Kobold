#pragma once

namespace kobold {

class challenge_rating final
{
public:
	challenge_rating()
	{
	}

	explicit challenge_rating(const std::string &str);

	std::string to_string() const;

	void change(const int change)
	{
		this->value += change;
	}

private:
	int value = 0;
};

}
