#include "kobold.h"

#include "character/character.h"

#include "character/alignment.h"
#include "character/alignment_axis.h"
#include "character/character_class.h"
#include "character/character_class_type.h"
#include "character/character_game_data.h"
#include "character/character_history.h"
#include "character/character_reference.h"
#include "character/character_template.h"
#include "character/dynasty.h"
#include "character/feat.h"
#include "character/feat_template.h"
#include "character/starting_age_category.h"
#include "country/country.h"
#include "country/country_game_data.h"
#include "country/culture.h"
#include "database/defines.h"
#include "game/game.h"
#include "language/name_generator.h"
#include "map/province.h"
#include "map/site.h"
#include "map/site_type.h"
#include "religion/deity.h"
#include "religion/religion.h"
#include "script/condition/and_condition.h"
#include "script/effect/effect_list.h"
#include "script/modifier.h"
#include "species/species.h"
#include "species/subspecies.h"
#include "time/calendar.h"
#include "unit/civilian_unit_class.h"
#include "unit/military_unit_category.h"
#include "util/assert_util.h"
#include "util/date_util.h"
#include "util/gender.h"
#include "util/log_util.h"
#include "util/map_util.h"
#include "util/random.h"
#include "util/string_util.h"
#include "util/vector_random_util.h"
#include "util/vector_util.h"

#include <QUuid>

namespace kobold {

const std::set<std::string> character::database_dependencies = {
	character_class::class_identifier,
	alignment::class_identifier,
	feat_template::class_identifier,
	//characters must be initialized after provinces, as their initialization results in settlements being assigned to their provinces, which is necessary for getting the provinces for home sites
	province::class_identifier
};

void character::initialize_all()
{
	data_type::initialize_all();

	std::vector<character *> dateless_characters = character::get_all();
	std::erase_if(dateless_characters, [](const character *character) {
		return character->has_vital_dates();
	});

	bool changed = true;
	while (changed) {
		while (changed) {
			changed = false;

			for (character *character : dateless_characters) {
				if (character->has_vital_dates()) {
					continue;
				}

				const bool success = character->initialize_dates_from_children();
				if (success) {
					character->initialize_dates();
					changed = true;
				}
			}

			std::erase_if(dateless_characters, [](const character *character) {
				return character->has_vital_dates();
			});
		}

		changed = false;
		for (character *character : dateless_characters) {
			const bool success = character->initialize_dates_from_parents();
			if (success) {
				character->initialize_dates();
				changed = true;
			}
		}

		std::erase_if(dateless_characters, [](const character *character) {
			return character->has_vital_dates();
		});
	}
}

character *character::generate(const kobold::species *species, const kobold::subspecies *subspecies, const std::map<character_class_type, character_class *> &character_classes, const int level, const kobold::culture *culture, const kobold::religion *religion, const site *home_settlement, const std::vector<const feat *> &feats, const bool temporary)
{
	auto generated_character = make_qunique<character>(QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString());
	generated_character->moveToThread(QApplication::instance()->thread());

	generated_character->temporary = temporary;

	generated_character->species = const_cast<kobold::species *>(species);
	generated_character->subspecies = const_cast<kobold::subspecies *>(subspecies);
	if (generated_character->subspecies == nullptr) {
		generated_character->subspecies = const_cast<kobold::subspecies *>(species->get_default_subspecies());
	}
	generated_character->character_classes = character_classes;
	generated_character->level = level;
	if (culture != nullptr) {
		generated_character->culture = const_cast<kobold::culture *>(culture);
	} else if (!generated_character->get_species_base()->get_cultures().empty()) {
		generated_character->culture = const_cast<kobold::culture *>(vector::get_random(generated_character->get_species_base()->get_cultures()));
	}
	generated_character->religion = religion;
	generated_character->generate_patron_deity();
	if (generated_character->get_culture() != nullptr) {
		generated_character->phenotype = generated_character->get_culture()->get_default_phenotype();
	}
	generated_character->generate_alignments();
	generated_character->home_settlement = home_settlement;
	generated_character->set_start_date(game::get()->get_date());

	const archimedes::gender gender = random::get()->generate(2) == 0 ? gender::male : gender::female;
	generated_character->set_gender(gender);
	if (generated_character->get_culture() != nullptr) {
		generated_character->set_name(generated_character->get_culture()->get_personal_name_generator(gender)->generate_name());
		const archimedes::name_generator *surname_generator = generated_character->get_culture()->get_surname_generator(gender);
		if (surname_generator != nullptr) {
			generated_character->set_surname(surname_generator->generate_name());
		}
	} else {
		generated_character->set_name(generated_character->get_species_base()->get_personal_name_generator(gender)->generate_name());
	}

	generated_character->initialize_dates();
	generated_character->check();
	generated_character->get_game_data()->set_target_feats(feats);
	generated_character->get_game_data()->apply_species_and_class(level);
	generated_character->get_game_data()->on_setup_finished();

	game::get()->add_generated_character(std::move(generated_character));
	return game::get()->get_generated_characters().back().get();
}

character *character::generate(const character_template *character_template, const kobold::culture *culture, const kobold::religion *religion, const site *home_settlement, const bool temporary)
{
	return character::generate(character_template->get_species(), character_template->get_subspecies(), character_template->get_character_classes(), character_template->get_level(), culture, religion, home_settlement, character_template->get_feats(), temporary);
}

std::shared_ptr<character_reference> character::generate_temporary(const character_template *character_template, const kobold::culture *culture, const kobold::religion *religion, const site *home_settlement)
{
	kobold::character *character = character::generate(character_template, culture, religion, home_settlement, true);
	return std::make_shared<character_reference>(character);
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
		scope.for_each_property([this](const gsml_property &property) {
			const std::string &key = property.get_key();
			const std::string &value = property.get_value();

			this->character_classes[magic_enum::enum_cast<character_class_type>(key).value()] = character_class::get(value);
		});
	} else if (tag == "alignments") {
		for (const std::string &value : values) {
			const alignment *alignment = alignment::get(value);
			this->alignments[alignment->get_axis()] = alignment;
		}

		scope.for_each_property([this](const gsml_property &property) {
			const std::string &key = property.get_key();
			const std::string &value = property.get_value();

			const alignment_axis *axis = alignment_axis::get(key);
			const alignment *alignment = alignment::get(value);
			assert_throw(alignment == nullptr || alignment->get_axis() == axis);

			this->alignments[axis] = alignment;
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
	if (this->get_deity() != nullptr) {
		if (this->get_name().empty()) {
			this->set_name(this->get_deity()->get_name());
		}

		if (this->get_name_word() == nullptr) {
			this->set_name_word(this->get_deity()->get_name_word());
		}
	}

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
	} else if (this->get_species_base() != nullptr && !this->get_species()->is_sapient()) {
		if (!this->get_species_base()->is_initialized()) {
			this->get_species_base()->initialize();
		}

		if (this->has_name_variant()) {
			this->get_species_base()->add_personal_name(this->get_gender(), this->get_name_variant());
		}
	}

	if (this->subspecies == nullptr && this->get_species() != nullptr) {
		this->subspecies = const_cast<kobold::subspecies *>(this->get_species()->get_default_subspecies());
	}

	if (this->get_gender() == gender::none && this->get_species() != nullptr && !this->get_species()->is_sapient()) {
		this->set_gender(static_cast<archimedes::gender>(random::get()->generate_in_range(static_cast<int>(gender::none) + 1, static_cast<int>(gender::count) - 1)));
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

	if (this->alignments.size() != alignment_axis::get_all().size()) {
		this->generate_alignments();
	}

	for (size_t i = 0; i < this->get_feats().size(); ++i) {
		const feat *feat = this->get_feats().at(i);
		if (feat->get_upgraded_feat() != nullptr) {
			this->feats.push_back(feat->get_upgraded_feat());
		}
	}

	if (this->get_patron_deity() == nullptr) {
		this->generate_patron_deity();
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

	if (this->get_character_class(character_class_type::base_class) == nullptr && species->get_hit_dice_count() == 0) {
		throw std::runtime_error(std::format("Character \"{}\" has no base character class, and its species has a hit dice count of zero.", this->get_identifier()));
	}

	if (this->get_species()->is_sapient()) {
		if (this->get_culture() == nullptr) {
			throw std::runtime_error(std::format("Sapient character \"{}\" has no culture.", this->get_identifier()));
		}

		if (!vector::contains(this->get_culture()->get_species(), this->get_species_base())) {
			throw std::runtime_error(std::format("Sapient character \"{}\" has a culture (\"{}\") which does not include its {}species (\"{}\").", this->get_identifier(), this->get_culture()->get_identifier(), this->get_species_base() == this->get_subspecies() ? "sub" : "", this->get_species_base()->get_identifier()));
		}

		if (this->get_religion() == nullptr && !this->is_temporary()) {
			throw std::runtime_error(std::format("Sapient character \"{}\" has no religion.", this->get_identifier()));
		}
	}

	if (this->get_patron_deity() != nullptr && !vector::contains(this->get_religion()->get_deities(), this->get_patron_deity())) {
		throw std::runtime_error(std::format("Character \"{}\" has a patron deity which does not belong to their religion.", this->get_identifier()));
	}

	if (this->get_patron_deity() != nullptr && !this->get_patron_deity()->get_domains().empty() && !this->get_religion()->get_domains().empty()) {
		throw std::runtime_error(std::format("Character \"{}\" has both a patron deity and a religion which provide divine domains.", this->get_identifier()));
	}

	if (this->get_home_settlement() == nullptr && this->get_species()->is_sapient() && !this->is_deity() && !this->is_temporary()) {
		throw std::runtime_error(std::format("Sapient, non-deity character \"{}\" has no home settlement.", this->get_identifier()));
	} else if (this->get_home_settlement() != nullptr && !this->get_home_settlement()->is_settlement()) {
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

	assert_throw(this->get_species_base() != nullptr);

	const int adulthood_age = this->get_species_base()->get_adulthood_age();
	const int venerable_age = this->get_species_base()->get_venerable_age();
	const dice &maximum_age_modifier = this->get_species_base()->get_maximum_age_modifier();

	if (adulthood_age != 0 && venerable_age != 0 && !maximum_age_modifier.is_null()) {
		const dice &starting_age_modifier = this->get_species_base()->get_starting_age_modifier(character_class ? character_class->get_starting_age_category() : starting_age_category::intuitive);

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
				} else if (this->get_contemporary_character() != nullptr) {
					if (!this->get_contemporary_character()->is_initialized()) {
						this->get_contemporary_character()->initialize();
					}

					if (this->get_contemporary_character()->get_start_date().isValid()) {
						this->set_start_date(this->get_contemporary_character()->get_start_date());
						date_changed = true;
					}
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

			if (!this->get_death_date().isValid() && !this->is_immortal()) {
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

bool character::initialize_dates_from_children()
{
	assert_throw(!this->has_vital_dates());

	if (this->get_children().empty()) {
		return false;
	}

	const int adulthood_age = this->get_species_base()->get_adulthood_age();
	if (adulthood_age == 0) {
		return false;
	}

	const int middle_age = this->get_species_base()->get_middle_age();
	if (middle_age == 0) {
		return false;
	}

	QDate earliest_child_birth_date;

	for (character_base *child_base : this->get_children()) {
		character *child = static_cast<character *>(child_base);
		if (!child->has_vital_dates()) {
			if (child->initialize_dates_from_children()) {
				child->initialize_dates();
			}
		}

		if (!child->get_birth_date().isValid()) {
			continue;
		}

		if (!earliest_child_birth_date.isValid() || child->get_birth_date() < earliest_child_birth_date) {
			earliest_child_birth_date = child->get_birth_date();
		}
	}

	if (!earliest_child_birth_date.isValid()) {
		return false;
	}

	QDate birth_date = earliest_child_birth_date;
	birth_date = birth_date.addYears(-random::get()->generate_in_range(adulthood_age, middle_age));
	this->set_birth_date(birth_date);
	log_trace(std::format("Set birth date for character \"{}\": {}.", this->get_identifier(), date::to_string(birth_date)));

	return true;
}

bool character::initialize_dates_from_parents()
{
	assert_throw(!this->has_vital_dates());

	std::vector<const character *> parents;
	if (this->get_father() != nullptr) {
		parents.push_back(this->get_father());
	}
	if (this->get_mother() != nullptr) {
		parents.push_back(this->get_mother());
	}

	if (parents.empty()) {
		return false;
	}

	int adulthood_age = 0;
	int middle_age = 0;

	QDate latest_parent_birth_date;

	for (const character *parent : parents) {
		if (!parent->get_birth_date().isValid()) {
			continue;
		}

		const int parent_adulthood_age = parent->get_species_base()->get_adulthood_age();
		if (parent_adulthood_age == 0) {
			continue;
		}

		const int parent_middle_age = parent->get_species_base()->get_middle_age();
		if (parent_middle_age == 0) {
			continue;
		}

		if (!latest_parent_birth_date.isValid() || parent->get_birth_date() > latest_parent_birth_date) {
			latest_parent_birth_date = parent->get_birth_date();
			adulthood_age = parent_adulthood_age;
			middle_age = parent_middle_age;
		}
	}

	if (!latest_parent_birth_date.isValid()) {
		return false;
	}

	QDate birth_date = latest_parent_birth_date;
	birth_date = birth_date.addYears(random::get()->generate_in_range(adulthood_age, middle_age));
	this->set_birth_date(birth_date);
	log_trace(std::format("Set birth date for character \"{}\": {}.", this->get_identifier(), date::to_string(birth_date)));

	return true;
}

const species_base *character::get_species_base() const
{
	if (this->get_subspecies() != nullptr) {
		return this->get_subspecies();
	}

	return this->get_species();
}

species_base *character::get_species_base()
{
	if (this->subspecies != nullptr) {
		return this->subspecies;
	}

	return this->species;
}

void character::generate_patron_deity()
{
	assert_throw(this->get_patron_deity() == nullptr);

	if (this->get_religion() == nullptr) {
		return;
	}

	if (!this->get_religion()->get_domains().empty()) {
		//religions with domains provide domains directly, instead of via a patron deity
		return;
	}

	if (this->get_religion()->get_deities().empty()) {
		return;
	}

	std::vector<const kobold::deity *> potential_deities;

	for (const kobold::deity *deity : this->get_religion()->get_deities()) {
		for (int i = 0; i < deity->get_divine_rank(); ++i) {
			potential_deities.push_back(deity);
		}
	}

	assert_throw(!potential_deities.empty());
	this->patron_deity = vector::get_random(potential_deities);
}

std::vector<const alignment *> character::get_alignments() const
{
	std::vector<const alignment *> alignments = archimedes::map::get_values(this->alignments);
	std::erase(alignments, nullptr);
	return alignments;
}

QVariantList character::get_alignments_qvariant_list() const
{
	return container::to_qvariant_list(this->get_alignments());
}

bool character::has_alignment(const alignment *alignment) const
{
	const auto find_iterator = this->alignments.find(alignment->get_axis());
	return find_iterator != this->alignments.end() && find_iterator->second == alignment;
}

void character::generate_alignments()
{
	for (const alignment_axis *axis : alignment_axis::get_all()) {
		if (this->alignments.contains(axis)) {
			continue;
		}

		std::vector<const alignment *> alignments = axis->get_alignments();
		alignments.push_back(nullptr);

		this->alignments[axis] = vector::get_random(alignments);
	}
}

}
