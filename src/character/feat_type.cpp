#include "kobold.h"

#include "character/feat_type.h"

#include "script/condition/and_condition.h"
#include "script/effect/effect_list.h"

namespace kobold {

feat_type::feat_type(const std::string &identifier)
	: named_data_entry(identifier)
{
}

feat_type::~feat_type()
{
}

void feat_type::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "gain_conditions") {
		auto conditions = std::make_unique<and_condition<character>>();
		database::process_gsml_data(conditions, scope);
		this->gain_conditions = std::move(conditions);
	} else if (tag == "effects") {
		auto effect_list = std::make_unique<kobold::effect_list<const character>>();
		database::process_gsml_data(effect_list, scope);
		this->effects = std::move(effect_list);
	} else {
		data_entry::process_gsml_scope(scope);
	}
}

void feat_type::check() const
{
	if (this->get_feats().empty()) {
		throw std::runtime_error(std::format("Feat type \"{}\" has no feats that belong to it.", this->get_identifier()));
	}
}

}
