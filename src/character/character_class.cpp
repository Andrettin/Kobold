#include "kobold.h"

#include "character/character_class.h"

#include "character/character_class_type.h"
#include "character/level_bonus_table.h"
#include "character/saving_throw_type.h"

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

	if (tag == "saving_throw_bonus_tables") {
		scope.for_each_property([&](const gsml_property &property) {
			const std::string &key = property.get_key();
			const std::string &value = property.get_value();

			this->saving_throw_bonus_tables[saving_throw_type::get(key)] = level_bonus_table::get(value);
		});
	} else if (tag == "rank_levels") {
		scope.for_each_property([&](const gsml_property &property) {
			const std::string &key = property.get_key();
			const std::string &value = property.get_value();

			this->rank_levels[key] = std::stoi(value);
		});
	} else {
		data_entry::process_gsml_scope(scope);
	}
}

void character_class::check() const
{
	if (this->get_type() == character_class_type::none) {
		throw std::runtime_error(std::format("Character class \"{}\" has no type.", this->get_identifier()));
	}

	if (this->get_hit_dice().is_null()) {
		throw std::runtime_error(std::format("Character class \"{}\" has null hit dice.", this->get_identifier()));
	}

	if (this->get_hit_dice().get_count() != 1) {
		throw std::runtime_error(std::format("Character class \"{}\" has hit dice with a dice count different than 1.", this->get_identifier()));
	}

	if (this->get_base_attack_bonus_table() == nullptr) {
		throw std::runtime_error(std::format("Character class \"{}\" has no base attack bonus table.", this->get_identifier()));
	}

	if (this->get_saving_throw_bonus_tables().empty()) {
		throw std::runtime_error(std::format("Character class \"{}\" has no saving throw bonus tables.", this->get_identifier()));
	}

	if (this->get_base_skill_points_per_level() == 0) {
		throw std::runtime_error(std::format("Character class \"{}\" has zero base skill points per level.", this->get_identifier()));
	}
}

}
