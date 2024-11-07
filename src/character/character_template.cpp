#include "kobold.h"

#include "character/character_template.h"

#include "character/character_class.h"
#include "character/character_class_type.h"
#include "character/feat.h"
#include "species/species.h"

namespace kobold {

character_template::character_template(const std::string &identifier)
	: named_data_entry(identifier)
{
}

character_template::~character_template()
{
}

void character_template::process_gsml_property(const gsml_property &property)
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

void character_template::process_gsml_scope(const gsml_data &scope)
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
	} else {
		data_entry::process_gsml_scope(scope);
	}
}

void character_template::initialize()
{
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

	named_data_entry::initialize();
}

void character_template::check() const
{
	if (this->get_species() == nullptr) {
		throw std::runtime_error(std::format("Character template \"{}\" has no species.", this->get_identifier()));
	}
}

}
