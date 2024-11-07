#include "kobold.h"

#include "character/character_history.h"

#include "character/character.h"
#include "character/character_class.h"
#include "character/character_class_type.h"

namespace kobold {

character_history::character_history(const kobold::character *character) : character(character)
{
	this->level = character->get_level();
	this->feats = character->get_feats();
}

void character_history::process_gsml_property(const gsml_property &property)
{
	const std::string &key = property.get_key();
	const std::string &value = property.get_value();

	if (key == "rank") {
		this->level = this->character->get_character_class(character_class_type::base_class)->get_rank_level(value);
	} else {
		data_entry_history::process_gsml_property(property);
	}
}

void character_history::set_spouse(const kobold::character *spouse)
{
	if (spouse == this->get_spouse()) {
		return;
	}

	const kobold::character *old_spouse = this->get_spouse();

	this->spouse = spouse;

	if (old_spouse != nullptr) {
		old_spouse->get_history()->set_spouse(nullptr);
	}

	if (spouse != nullptr) {
		spouse->get_history()->set_spouse(this->character);
	}
}

}
