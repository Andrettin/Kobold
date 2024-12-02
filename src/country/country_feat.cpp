#include "kobold.h"

#include "country/country_feat.h"

#include "country/country_feat_type.h"
#include "script/condition/and_condition.h"
#include "script/condition/feat_condition.h"
#include "script/condition/not_condition.h"
#include "script/effect/effect_list.h"
#include "script/factor.h"
#include "script/modifier.h"
#include "util/vector_util.h"

namespace kobold {

country_feat::country_feat(const std::string &identifier)
	: named_data_entry(identifier)
{
}

country_feat::~country_feat()
{
}

void country_feat::process_gsml_property(const gsml_property &property)
{
	const std::string &key = property.get_key();
	const std::string &value = property.get_value();

	if (key == "weight_factor") {
		const int factor = std::stoi(value);
		this->weight_factor = std::make_unique<kobold::factor<country>>(factor);
	} else {
		data_entry::process_gsml_property(property);
	}
}

void country_feat::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const std::vector<std::string> &values = scope.get_values();

	if (tag == "types") {
		for (const std::string &value : values) {
			country_feat_type *type = country_feat_type::get(value);
			this->types.push_back(type);
			type->add_feat(this);
		}
	} else if (tag == "conditions") {
		auto conditions = std::make_unique<and_condition<country>>();
		database::process_gsml_data(conditions, scope);
		this->conditions = std::move(conditions);
	} else if (tag == "modifier") {
		auto modifier = std::make_unique<kobold::modifier<const country>>();
		database::process_gsml_data(modifier, scope);
		this->modifier = std::move(modifier);
	} else if (tag == "effects") {
		auto effect_list = std::make_unique<kobold::effect_list<const country>>();
		database::process_gsml_data(effect_list, scope);
		this->effects = std::move(effect_list);
	} else if (tag == "weight_factor") {
		this->weight_factor = std::make_unique<factor<country>>();
		database::process_gsml_data(this->weight_factor, scope);
	} else {
		data_entry::process_gsml_scope(scope);
	}
}

void country_feat::initialize()
{
	country_feat *upgraded_feat = this->upgraded_feat;
	while (upgraded_feat != nullptr) {
		auto feat_condition = std::make_unique<kobold::feat_condition<country>>(this);
		auto not_condition = std::make_unique<archimedes::not_condition<country, read_only_context, condition<country>>>(std::move(feat_condition));
		if (upgraded_feat->get_conditions() == nullptr) {
			upgraded_feat->conditions = std::make_unique<and_condition<country>>();
		}
		upgraded_feat->conditions->add_condition(std::move(not_condition));
		upgraded_feat = upgraded_feat->upgraded_feat;
	}

	named_data_entry::initialize();
}

void country_feat::check() const
{
	if (this->get_types().empty()) {
		throw std::runtime_error(std::format("Country feat \"{}\" has no types.", this->get_identifier()));
	}

	if (this->get_icon() == nullptr) {
		throw std::runtime_error(std::format("Country feat \"{}\" has no icon.", this->get_identifier()));
	}

	if (this->get_weight_factor() != nullptr) {
		this->get_weight_factor()->check();
	}
}

bool country_feat::has_type(const country_feat_type *type) const
{
	return vector::contains(this->get_types(), type);
}

QString country_feat::get_modifier_string() const
{
	if (this->get_modifier() == nullptr) {
		return QString();
	}

	return QString::fromStdString(this->get_modifier()->get_string(nullptr));
}

}
