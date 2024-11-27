#include "kobold.h"

#include "religion/deity.h"

#include "character/character.h"
#include "character/character_class.h"
#include "character/feat_template.h"
#include "database/defines.h"
#include "map/province.h"
#include "religion/divine_domain.h"
#include "religion/religion.h"
#include "util/string_util.h"

namespace kobold {

const std::set<std::string> deity::database_dependencies = {
	character_class::class_identifier,
	feat_template::class_identifier,
	province::class_identifier
};

deity *deity::add(const std::string &identifier, const kobold::data_module *data_module)
{
	deity *deity = data_type::add(identifier, data_module);

	//add a character with the same identifier as the deity for it
	kobold::character *character = character::add(identifier, data_module);
	character->set_deity(deity);
	deity->character = character;

	return deity;
}

void deity::process_gsml_property(const gsml_property &property)
{
	const std::string &key = property.get_key();
	const std::string &value = property.get_value();

	if (key == "divine_rank") {
		if (string::is_number(value)) {
			this->divine_rank = std::stoi(value);
		} else {
			this->divine_rank = defines::get()->get_divine_rank(value);
		}
	} else {
		data_entry::process_gsml_property(property);
	}
}

void deity::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const std::vector<std::string> &values = scope.get_values();

	if (tag == "religions") {
		for (const std::string &value : values) {
			religion *religion = religion::get(value);
			religion->add_deity(this);
			this->religions.push_back(religion);
		}
	} else if (tag == "domains") {
		for (const std::string &value : values) {
			this->domains.push_back(divine_domain::get(value));
		}
	} else if (tag == "character") {
		database::process_gsml_data(this->character, scope);
	} else {
		data_entry::process_gsml_scope(scope);
	}
}

void deity::initialize()
{
	named_data_entry::initialize();
}

void deity::check() const
{
	if (this->get_pantheon() == nullptr) {
		throw std::runtime_error(std::format("Deity \"{}\" has no pantheon.", this->get_identifier()));
	}

	if (this->get_religions().empty()) {
		throw std::runtime_error(std::format("Deity \"{}\" is not worshipped by any religions.", this->get_identifier()));
	}

	if (this->get_divine_rank() == 0) {
		throw std::runtime_error(std::format("Deity \"{}\" has no divine rank.", this->get_identifier()));
	}

	if (static_cast<int>(this->get_domains().size()) < deity::base_deity_domains) {
		throw std::runtime_error(std::format("Deity \"{}\" has less domains than the base number of deity domains ({}).", this->get_identifier(), deity::base_deity_domains));
	}
}

}
