#include "kobold.h"

#include "character/character.h"

#include "character/character_class.h"
#include "character/character_class_type.h"
#include "character/character_game_data.h"
#include "character/character_history.h"
#include "character/character_role.h"
#include "character/dynasty.h"
#include "character/trait.h"
#include "character/trait_type.h"
#include "country/country.h"
#include "country/country_game_data.h"
#include "country/culture.h"
#include "country/religion.h"
#include "database/defines.h"
#include "map/province.h"
#include "map/site.h"
#include "map/site_type.h"
#include "script/condition/and_condition.h"
#include "script/effect/effect_list.h"
#include "script/modifier.h"
#include "time/calendar.h"
#include "unit/civilian_unit_class.h"
#include "unit/military_unit_category.h"
#include "util/assert_util.h"
#include "util/log_util.h"
#include "util/string_util.h"

namespace kobold {

const std::set<std::string> character::database_dependencies = {
	//characters must be initialized after provinces, as their initialization results in settlements being assigned to their provinces, which is necessary for getting the provinces for home sites
	province::class_identifier
};

character::character(const std::string &identifier)
	: character_base(identifier), role(character_role::none)
{
	this->reset_game_data();
}

character::~character()
{
}

void character::process_gsml_property(const gsml_property &property)
{
	const std::string &key = property.get_key();
	const std::string &value = property.get_value();

	if (key == "character_class") {
		const character_class *character_class = character_class::get(value);
		this->character_classes[character_class->get_type()] = character_class;
	} else {
		data_entry::process_gsml_property(property);
	}
}

void character::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const std::vector<std::string> &values = scope.get_values();

	if (tag == "character_classes") {
		scope.for_each_property([&](const gsml_property &property) {
			const std::string &key = property.get_key();
			const std::string &value = property.get_value();

			this->character_classes[enum_converter<character_class_type>::to_enum(key)] = character_class::get(value);
		});
	} else if (tag == "rulable_countries") {
		for (const std::string &value : values) {
			this->add_rulable_country(country::get(value));
		}
	} else if (tag == "traits") {
		for (const std::string &value : values) {
			this->traits.push_back(trait::get(value));
		}
	} else if (tag == "conditions") {
		auto conditions = std::make_unique<and_condition<country>>();
		database::process_gsml_data(conditions, scope);
		this->conditions = std::move(conditions);
	} else {
		data_entry::process_gsml_scope(scope);
	}
}

void character::initialize()
{
	if (this->get_phenotype() == nullptr && this->get_culture() != nullptr) {
		this->phenotype = this->get_culture()->get_default_phenotype();
	}

	if (this->get_surname().empty() && this->get_dynasty() != nullptr) {
		std::string surname;
		if (!this->get_dynasty()->get_prefix().empty()) {
			surname = this->get_dynasty()->get_prefix() + " ";
		}

		surname += this->get_dynasty()->get_name();
		this->set_surname(surname);
	}

	if (this->get_culture() != nullptr) {
		if (!this->get_culture()->is_initialized()) {
			this->culture->initialize();
		}

		if (this->has_name_variant()) {
			this->culture->add_personal_name(this->get_gender(), this->get_name_variant());
		}

		if (!this->get_surname().empty()) {
			this->culture->add_surname(this->get_gender(), this->get_surname());
		}
	}

	if (this->home_site != nullptr) {
		if (this->home_site->is_settlement()) {
			if (this->get_home_settlement() != nullptr) {
				throw std::runtime_error(std::format("Character \"{}\" has both a home settlement and a home site.", this->get_identifier()));
			}

			this->home_settlement = this->home_site;
		} else {
			if (this->home_site->get_province() == nullptr) {
				throw std::runtime_error(std::format("Character \"{}\" has a home site (\"{}\") which has no province.", this->get_identifier(), this->home_site->get_identifier()));
			}

			this->home_settlement = this->home_site->get_province()->get_provincial_capital();
		}
	}

	if (!this->rank.empty()) {
		assert_throw(this->get_level() == 0);
		assert_throw(this->get_character_class(character_class_type::base_class) != nullptr);
		this->level = this->get_character_class(character_class_type::base_class)->get_rank_level(this->rank);
		this->rank.clear();
	}

	character_base::initialize();
}

void character::check() const
{
	if (this->get_character_class(character_class_type::base_class) == nullptr) {
		throw std::runtime_error(std::format("Character \"{}\" has no base character class.", this->get_identifier()));
	}

	switch (this->get_role()) {
		case character_role::ruler: {
			if (this->get_rulable_countries().empty()) {
				throw std::runtime_error(std::format("Character \"{}\" is a ruler, but has no rulable countries.", this->get_identifier()));
			}

			std::vector<const trait *> ruler_traits = this->get_traits();
			std::erase_if(ruler_traits, [](const trait *trait) {
				return trait->get_type() != trait_type::ruler;
			});
			const int ruler_trait_count = static_cast<int>(ruler_traits.size());
			const int min_ruler_traits = defines::get()->get_min_traits_for_type(trait_type::ruler);
			const int max_ruler_traits = defines::get()->get_max_traits_for_type(trait_type::ruler);

			if (ruler_trait_count < min_ruler_traits) {
				log::log_error(std::format("Ruler character \"{}\" only has {} ruler {}, less than the expected minimum of {}.", this->get_identifier(), ruler_trait_count, ruler_trait_count == 1 ? "trait" : "traits", min_ruler_traits));
			} else if (ruler_trait_count > max_ruler_traits) {
				log::log_error(std::format("Ruler character \"{}\" has {} ruler {}, more than the expected maximum of {}.", this->get_identifier(), ruler_trait_count, ruler_trait_count == 1 ? "trait" : "traits", max_ruler_traits));
			}
			break;
		}
		default:
			break;
	}

	if (this->get_role() != character_role::ruler && !this->get_rulable_countries().empty()) {
		throw std::runtime_error(std::format("Character \"{}\" has rulable countries, but is not a ruler.", this->get_identifier()));
	}

	assert_throw(this->get_culture() != nullptr);

	if (this->get_religion() == nullptr) {
		throw std::runtime_error(std::format("Character \"{}\" has no religion.", this->get_identifier()));
	}

	assert_throw(this->get_phenotype() != nullptr);

	if (this->get_home_settlement() == nullptr) {
		throw std::runtime_error(std::format("Character \"{}\" has no home settlement.", this->get_identifier()));
	} else if (!this->get_home_settlement()->is_settlement()) {
		throw std::runtime_error(std::format("Character \"{}\" has \"{}\" set as their home settlement, but it is not a settlement site.", this->get_identifier(), this->get_home_settlement()->get_identifier()));
	}

	character_base::check();
}

data_entry_history *character::get_history_base()
{
	return this->history.get();
}

void character::reset_history()
{
	this->history = make_qunique<character_history>(this);
}

void character::reset_game_data()
{
	this->game_data = make_qunique<character_game_data>(this);
	emit game_data_changed();
}

void character::add_rulable_country(country *country)
{
	this->rulable_countries.push_back(country);
	country->add_ruler(this);
}

}
