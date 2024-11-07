#include "kobold.h"

#include "character/character.h"

#include "character/character_class.h"
#include "character/character_class_type.h"
#include "character/character_game_data.h"
#include "character/character_history.h"
#include "character/character_template.h"
#include "character/dynasty.h"
#include "character/feat.h"
#include "country/country.h"
#include "country/country_game_data.h"
#include "country/culture.h"
#include "country/religion.h"
#include "database/defines.h"
#include "game/game.h"
#include "language/name_generator.h"
#include "map/province.h"
#include "map/site.h"
#include "map/site_type.h"
#include "script/condition/and_condition.h"
#include "script/effect/effect_list.h"
#include "script/modifier.h"
#include "species/species.h"
#include "time/calendar.h"
#include "unit/civilian_unit_class.h"
#include "unit/military_unit_category.h"
#include "util/assert_util.h"
#include "util/gender.h"
#include "util/log_util.h"
#include "util/random.h"
#include "util/string_util.h"

#include <QUuid>

namespace kobold {

const std::set<std::string> character::database_dependencies = {
	//characters must be initialized after provinces, as their initialization results in settlements being assigned to their provinces, which is necessary for getting the provinces for home sites
	province::class_identifier
};

const character *character::generate(const kobold::species *species, const std::map<character_class_type, character_class *> &character_classes, const int level, const kobold::culture *culture, const kobold::religion *religion, const site *home_settlement, const std::vector<const feat *> &feats)
{
	auto generated_character = make_qunique<character>(QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString());
	generated_character->moveToThread(QApplication::instance()->thread());

	generated_character->species = const_cast<kobold::species *>(species);
	generated_character->character_classes = character_classes;
	generated_character->level = level;
	generated_character->culture = const_cast<kobold::culture *>(culture);
	generated_character->religion = religion;
	generated_character->phenotype = culture->get_default_phenotype();
	generated_character->home_settlement = home_settlement;
	generated_character->set_start_date(game::get()->get_date());

	const archimedes::gender gender = random::get()->generate(2) == 0 ? gender::male : gender::female;
	generated_character->set_gender(gender);
	generated_character->set_name(culture->get_personal_name_generator(gender)->generate_name());
	generated_character->set_surname(culture->get_surname_generator(gender)->generate_name());

	generated_character->initialize_dates();
	generated_character->check();
	generated_character->get_game_data()->set_target_feats(feats);
	generated_character->get_game_data()->apply_species_and_class(level);
	generated_character->get_game_data()->on_setup_finished();

	game::get()->add_generated_character(std::move(generated_character));
	return game::get()->get_generated_characters().back().get();
}

const character *character::generate(const character_template *character_template, const kobold::culture *culture, const kobold::religion *religion, const site *home_settlement)
{
	return character::generate(character_template->get_species(), character_template->get_character_classes(), character_template->get_level(), culture, religion, home_settlement, character_template->get_feats());
}

character::character(const std::string &identifier)
	: character_base(identifier)
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
		character_class *character_class = character_class::get(value);
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
	} else if (tag == "feats") {
		for (const std::string &value : values) {
			this->feats.push_back(feat::get(value));
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

	for (size_t i = 0; i < this->get_feats().size(); ++i) {
		const feat *feat = this->get_feats().at(i);
		if (feat->get_upgraded_feat() != nullptr) {
			this->feats.push_back(feat->get_upgraded_feat());
		}
	}

	for (const auto &[character_class_type, character_class] : this->get_character_classes()) {
		this->species->add_character_class_weight(character_class, 1);
	}

	character_class *character_class = this->get_character_class();
	if (character_class != nullptr) {
		for (const feat *feat : this->get_feats()) {
			character_class->add_feat_weight(feat, 1);
		}
	} else {
		for (const feat *feat : this->get_feats()) {
			this->species->add_feat_weight(feat, 1);
		}
	}

	character_base::initialize();
}

void character::check() const
{
	if (this->get_species() == nullptr) {
		throw std::runtime_error(std::format("Character \"{}\" has no species.", this->get_identifier()));
	}

	if (this->get_character_class(character_class_type::base_class) == nullptr) {
		throw std::runtime_error(std::format("Character \"{}\" has no base character class.", this->get_identifier()));
	}

	if (this->get_culture() == nullptr) {
		throw std::runtime_error(std::format("Character \"{}\" has no culture.", this->get_identifier()));
	}

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

void character::initialize_dates()
{
	const character_class *character_class = this->get_character_class(character_class_type::base_class);

	assert_throw(this->get_species() != nullptr);
	assert_throw(character_class != nullptr);

	const int adulthood_age = this->get_species()->get_adulthood_age();
	const int venerable_age = this->get_species()->get_venerable_age();
	const dice &maximum_age_modifier = this->get_species()->get_maximum_age_modifier();

	if (adulthood_age != 0 && venerable_age != 0 && !maximum_age_modifier.is_null()) {
		const dice &starting_age_modifier = this->get_species()->get_starting_age_modifier(character_class);

		bool date_changed = true;
		while (date_changed) {
			date_changed = false;

			if (!this->get_start_date().isValid()) {
				if (this->get_birth_date().isValid()) {
					QDate start_date = this->get_birth_date();
					start_date = start_date.addYears(adulthood_age);
					start_date = start_date.addYears(random::get()->roll_dice(starting_age_modifier));
					this->set_start_date(start_date);
					date_changed = true;
				}
			}

			if (!this->get_birth_date().isValid()) {
				if (this->get_start_date().isValid()) {
					QDate birth_date = this->get_start_date();
					birth_date = birth_date.addYears(-adulthood_age);
					birth_date = birth_date.addYears(-random::get()->roll_dice(starting_age_modifier));
					this->set_birth_date(birth_date);
					date_changed = true;
				} else if (this->get_death_date().isValid()) {
					QDate birth_date = this->get_death_date();
					birth_date = birth_date.addYears(-venerable_age);
					birth_date = birth_date.addYears(-random::get()->roll_dice(maximum_age_modifier));
					this->set_birth_date(birth_date);
					date_changed = true;
				}
			}

			if (!this->get_death_date().isValid()) {
				if (this->get_birth_date().isValid()) {
					QDate death_date = this->get_birth_date();
					death_date = death_date.addYears(venerable_age);
					death_date = death_date.addYears(random::get()->roll_dice(maximum_age_modifier));
					this->set_death_date(death_date);
					date_changed = true;
				}
			}
		}
	}

	character_base::initialize_dates();
}

}
