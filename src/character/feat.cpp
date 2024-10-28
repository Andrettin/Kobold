#include "kobold.h"

#include "character/feat.h"

#include "character/feat_type.h"
#include "script/condition/and_condition.h"
#include "script/modifier.h"

namespace kobold {

feat::feat(const std::string &identifier)
	: named_data_entry(identifier)
{
}

feat::~feat()
{
}

void feat::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const std::vector<std::string> &values = scope.get_values();

	if (tag == "types") {
		for (const std::string &value : values) {
			feat_type *type = feat_type::get(value);
			this->types.push_back(type);
			type->add_feat(this);
		}
	} else if (tag == "conditions") {
		auto conditions = std::make_unique<and_condition<character>>();
		database::process_gsml_data(conditions, scope);
		this->conditions = std::move(conditions);
	} else if (tag == "modifier") {
		auto modifier = std::make_unique<kobold::modifier<const character>>();
		database::process_gsml_data(modifier, scope);
		this->modifier = std::move(modifier);
	} else {
		data_entry::process_gsml_scope(scope);
	}
}

void feat::check() const
{
	if (this->get_types().empty()) {
		throw std::runtime_error(std::format("Feat \"{}\" has no types.", this->get_identifier()));
	}

	if (this->get_icon() == nullptr) {
		throw std::runtime_error(std::format("Feat \"{}\" has no icon.", this->get_identifier()));
	}
}

QString feat::get_modifier_string() const
{
	if (this->get_modifier() == nullptr) {
		return QString();
	}

	return QString::fromStdString(this->get_modifier()->get_string(nullptr));
}

}
