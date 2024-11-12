#include "kobold.h"

#include "country/country_milestone.h"

#include "country/country_feat_type.h"
#include "script/condition/and_condition.h"
#include "script/condition/feat_condition.h"
#include "script/condition/not_condition.h"
#include "script/effect/effect_list.h"
#include "script/factor.h"
#include "script/modifier.h"
#include "util/vector_util.h"

namespace kobold {

country_milestone::country_milestone(const std::string &identifier)
	: named_data_entry(identifier)
{
}

country_milestone::~country_milestone()
{
}

void country_milestone::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "conditions") {
		auto conditions = std::make_unique<and_condition<country>>();
		database::process_gsml_data(conditions, scope);
		this->conditions = std::move(conditions);
	} else if (tag == "effects") {
		auto effect_list = std::make_unique<kobold::effect_list<const country>>();
		database::process_gsml_data(effect_list, scope);
		this->effects = std::move(effect_list);
	} else {
		data_entry::process_gsml_scope(scope);
	}
}

void country_milestone::check() const
{
	if (this->get_portrait() == nullptr) {
		throw std::runtime_error(std::format("Country milestone \"{}\" has no portrait.", this->get_identifier()));
	}

	if (this->get_conditions() != nullptr) {
		this->get_conditions()->check_validity();
	}

	if (this->get_effects() != nullptr) {
		this->get_effects()->check();
	}
}

QString country_milestone::get_effects_string() const
{
	if (this->get_effects() == nullptr) {
		return QString();
	}

	const read_only_context ctx;
	return QString::fromStdString(this->get_effects()->get_effects_string(nullptr, ctx));
}

}
