#include "kobold.h"

#include "religion/deity.h"

#include "character/character.h"
#include "character/character_class.h"
#include "character/feat_template.h"
#include "country/cultural_group.h"
#include "country/culture.h"
#include "database/defines.h"
#include "map/province.h"
#include "religion/divine_domain.h"
#include "religion/divine_subdomain.h"
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
			const divine_domain_base *domain = divine_domain::try_get(value);
			if (domain == nullptr) {
				domain = divine_subdomain::get(value);
			}
			this->domains.push_back(domain);
		}
	} else if (tag == "cultural_names") {
		scope.for_each_property([&](const gsml_property &property) {
			const culture *culture = culture::get(property.get_key());
			this->cultural_names[culture] = property.get_value();
		});
	} else if (tag == "cultural_group_names") {
		scope.for_each_property([&](const gsml_property &property) {
			const cultural_group *cultural_group = cultural_group::get(property.get_key());
			this->cultural_group_names[cultural_group] = property.get_value();
		});
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

	data_entry_set<divine_domain> main_domains;
	for (const divine_domain_base *domain : this->get_domains()) {
		main_domains.insert(domain->get_domain());
	}

	if (static_cast<int>(main_domains.size()) < divine_domain::min_count) {
		throw std::runtime_error(std::format("Deity \"{}\" has less main domains than the minimum necessary ({}).", this->get_identifier(), divine_domain::min_count));
	}
}

const std::string &deity::get_cultural_name(const culture *culture) const
{
	if (culture != nullptr) {
		const auto find_iterator = this->cultural_names.find(culture);
		if (find_iterator != this->cultural_names.end()) {
			return find_iterator->second;
		}

		if (culture->get_group() != nullptr) {
			const auto group_find_iterator = this->cultural_group_names.find(culture->get_group());
			if (group_find_iterator != this->cultural_group_names.end()) {
				return group_find_iterator->second;
			}
		}
	}

	return this->get_name();
}

}
