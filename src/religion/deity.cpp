#include "kobold.h"

#include "religion/deity.h"

#include "character/character.h"
#include "character/character_class.h"
#include "character/feat_template.h"
#include "map/province.h"
#include "religion/divine_domain.h"

namespace kobold {

const std::set<std::string> deity::database_dependencies = {
	character_class::class_identifier,
	feat_template::class_identifier,
	province::class_identifier
};

void deity::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const std::vector<std::string> &values = scope.get_values();

	if (tag == "character") {
		//add a character with the same identifier as the deity for it
		kobold::character *character = character::add(this->get_identifier(), this->get_module());
		character->set_deity(this);
		this->character = character;

		database::process_gsml_data(character, scope);
	} else if (tag == "domains") {
		for (const std::string &value : values) {
			this->domains.push_back(divine_domain::get(value));
		}
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

	if (static_cast<int>(this->get_domains().size()) < deity::base_deity_domains) {
		throw std::runtime_error(std::format("Deity \"{}\" has less domains than the base number of deity domains ({}).", this->get_identifier(), deity::base_deity_domains));
	}
}

}
