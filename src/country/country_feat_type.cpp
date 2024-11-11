#include "kobold.h"

#include "country/country_feat_type.h"

#include "script/condition/and_condition.h"
#include "script/effect/effect_list.h"

namespace kobold {

country_feat_type::country_feat_type(const std::string &identifier)
	: named_data_entry(identifier)
{
}

country_feat_type::~country_feat_type()
{
}

void country_feat_type::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "gain_conditions") {
		auto conditions = std::make_unique<and_condition<country>>();
		database::process_gsml_data(conditions, scope);
		this->gain_conditions = std::move(conditions);
	} else if (tag == "effects") {
		auto effect_list = std::make_unique<kobold::effect_list<const country>>();
		database::process_gsml_data(effect_list, scope);
		this->effects = std::move(effect_list);
	} else {
		data_entry::process_gsml_scope(scope);
	}
}

void country_feat_type::check() const
{
	if (this->get_feats().empty()) {
		throw std::runtime_error(std::format("Feat type \"{}\" has no feats that belong to it.", this->get_identifier()));
	}
}

}
