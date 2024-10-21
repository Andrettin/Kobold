#include "kobold.h"

#include "character/character_class.h"

namespace kobold {

character_class::character_class(const std::string &identifier)
	: named_data_entry(identifier)
{
}

character_class::~character_class()
{
}

void character_class::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "base_attack_bonus_per_level") {
		scope.for_each_property([&](const gsml_property &property) {
			const std::string &key = property.get_key();
			const std::string &value = property.get_value();

			this->base_attack_bonus_per_level[std::stoi(key)] = std::stoi(value);
		});
	} else {
		data_entry::process_gsml_scope(scope);
	}
}

void character_class::check() const
{
	if (this->get_hit_dice().is_null()) {
		throw std::runtime_error(std::format("Character class \"{}\" has null hit dice.", this->get_identifier()));
	}

	if (this->base_attack_bonus_per_level.empty()) {
		throw std::runtime_error(std::format("Character class \"{}\" has no base attack bonus per level.", this->get_identifier()));
	}

	if (this->get_base_skill_points_per_level() == 0) {
		throw std::runtime_error(std::format("Character class \"{}\" has zero base skill points per level.", this->get_identifier()));
	}
}

}
