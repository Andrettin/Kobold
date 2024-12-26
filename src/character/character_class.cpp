#include "kobold.h"

#include "character/character_class.h"

#include "character/character_attribute.h"
#include "character/character_class_type.h"
#include "character/feat_template.h"
#include "character/level_bonus_table.h"
#include "character/saving_throw_type.h"
#include "character/skill.h"
#include "character/skill_category.h"
#include "character/starting_age_category.h"
#include "script/condition/and_condition.h"
#include "script/effect/effect_list.h"
#include "script/modifier.h"

namespace kobold {

const std::set<std::string> character_class::database_dependencies = {
	feat_template::class_identifier
};

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
	const std::vector<std::string> &values = scope.get_values();

	if (tag == "saving_throw_bonus_tables") {
		scope.for_each_property([&](const gsml_property &property) {
			const std::string &key = property.get_key();
			const std::string &value = property.get_value();

			this->saving_throw_bonus_tables[saving_throw_type::get(key)] = level_bonus_table::get(value);
		});
	} else if (tag == "class_skills") {
		for (const std::string &value : values) {
			const skill *skill = skill::try_get(value);
			if (skill != nullptr) {
				this->class_skills.insert(skill);
			} else {
				const skill_category *skill_category = skill_category::get(value);
				for (const kobold::skill *category_skill : skill_category->get_skills()) {
					this->class_skills.insert(category_skill);
				}
			}
		}
	} else if (tag == "min_attribute_values") {
		scope.for_each_property([&](const gsml_property &property) {
			const std::string &key = property.get_key();
			const std::string &value = property.get_value();

			this->min_attribute_values[character_attribute::get(key)] = std::stoi(value);
		});
	} else if (tag == "rank_levels") {
		scope.for_each_property([&](const gsml_property &property) {
			const std::string &key = property.get_key();
			const std::string &value = property.get_value();

			this->rank_levels[key] = std::stoi(value);
		});
	} else if (tag == "conditions") {
		auto conditions = std::make_unique<and_condition<character>>();
		database::process_gsml_data(conditions, scope);
		this->conditions = std::move(conditions);
	} else if (tag == "modifier") {
		auto modifier = std::make_unique<kobold::modifier<const character>>();
		database::process_gsml_data(modifier, scope);
		this->modifier = std::move(modifier);
	} else if (tag == "level_effects") {
		scope.for_each_child([&](const gsml_data &child_scope) {
			const std::string &child_tag = child_scope.get_tag();
			const int level = std::stoi(child_tag);
			auto effect_list = std::make_unique<kobold::effect_list<const character>>();
			database::process_gsml_data(effect_list, child_scope);
			this->level_effects[level] = std::move(effect_list);
		});
	} else {
		data_entry::process_gsml_scope(scope);
	}
}

void character_class::initialize()
{
	character_class::character_classes_by_type[this->get_type()].push_back(this);

	named_data_entry::initialize();
}

void character_class::check() const
{
	if (this->get_type() == character_class_type::none) {
		throw std::runtime_error(std::format("Character class \"{}\" has no type.", this->get_identifier()));
	}

	if (this->get_icon() == nullptr) {
		throw std::runtime_error(std::format("Character class \"{}\" has no icon.", this->get_identifier()));
	}

	if (this->get_max_level() == 0) {
		throw std::runtime_error(std::format("Character class \"{}\" has no max level.", this->get_identifier()));
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

	if (this->get_class_skills().empty()) {
		throw std::runtime_error(std::format("Character class \"{}\" has no class skills.", this->get_identifier()));
	}

	if (this->get_base_skill_points_per_level() == 0) {
		throw std::runtime_error(std::format("Character class \"{}\" has zero base skill points per level.", this->get_identifier()));
	}

	if (this->get_type() == character_class_type::base_class && this->get_starting_age_category() == starting_age_category::none) {
		throw std::runtime_error(std::format("Base character class \"{}\" has no starting age category.", this->get_identifier()));
	}

	if (this->get_conditions() != nullptr) {
		this->get_conditions()->check_validity();

		if (this->get_type() == character_class_type::base_class || this->get_type() == character_class_type::racial_class) {
			throw std::runtime_error(std::format("Base or racial character class \"{}\" has conditions.", this->get_identifier()));
		}
	} else {
		if (this->get_type() == character_class_type::prestige_class || this->get_type() == character_class_type::epic_prestige_class) {
			throw std::runtime_error(std::format("Prestige character class \"{}\" has no conditions.", this->get_identifier()));
		}
	}
}

QString character_class::get_tooltip(const kobold::character *character) const
{
	std::string str = std::format("Hit Dice: {}", this->get_hit_dice().to_string());

	if (this->get_modifier() != nullptr) {
		str += "\n" + this->get_modifier()->get_string(character);
	}

	for (const auto &[level, level_effects] : this->level_effects) {
		str += std::format("\nLevel {}:\n{}", level, level_effects->get_effects_string(character, read_only_context(character), 1, {}, true));
	}

	return QString::fromStdString(str);
}

}
