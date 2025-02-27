#pragma once

#include "script/condition/condition.h"
#include "unit/military_unit.h"
#include "unit/military_unit_category.h"
#include "util/string_util.h"

namespace kobold {

template <typename scope_type>
class military_unit_category_condition final : public condition<scope_type>
{
public:
	explicit military_unit_category_condition(const std::string &value, const gsml_operator condition_operator)
		: condition<scope_type>(condition_operator)
	{
		this->military_unit_category = enum_converter<kobold::military_unit_category>::to_enum(value);
	}

	virtual const std::string &get_class_identifier() const override
	{
		static const std::string class_identifier = "military_unit_category";
		return class_identifier;
	}

	virtual bool check_assignment(const scope_type *scope, const read_only_context &ctx) const override
	{
		Q_UNUSED(ctx);

		return scope->get_category() == this->military_unit_category;
	}

	virtual std::string get_assignment_string(const size_t indent) const override
	{
		Q_UNUSED(indent);

		return string::highlight(get_military_unit_category_name(this->military_unit_category));
	}

private:
	kobold::military_unit_category military_unit_category = kobold::military_unit_category::none;
};

}
