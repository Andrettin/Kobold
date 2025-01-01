#pragma once

namespace kobold {

class character;

class character_reference final
{
public:
	explicit character_reference(kobold::character *character);
	~character_reference();

	const kobold::character *get_character() const
	{
		return this->character;
	}

private:
	kobold::character *character = nullptr;
};

}
