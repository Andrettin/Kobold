#pragma once

#include "character/character.h"
#include "character/character_game_data.h"
#include "character/feat.h"
#include "script/condition/condition.h"

namespace kobold {

template <typename scope_type>
class can_have_feat_condition final : public condition<scope_type>
{
public:
	using feat_type = std::conditional_t<std::is_same_v<scope_type, country>, country_feat, feat>;

	explicit can_have_feat_condition(const std::string &value, const gsml_operator condition_operator)
		: condition<scope_type>(condition_operator)
	{
		this->feat = feat_type::get(value);
	}

	virtual const std::string &get_class_identifier() const override
	{
		static const std::string class_identifier = "can_have_feat";
		return class_identifier;
	}

	virtual bool check_assignment(const scope_type *scope, const read_only_context &ctx) const override
	{
		Q_UNUSED(ctx);

		return scope->get_game_data()->can_have_feat(this->feat);
	}

	virtual std::string get_assignment_string(const size_t indent) const override
	{
		Q_UNUSED(indent);

		return "Can have the " + this->feat->get_name() + " feat";
	}

private:
	const feat_type *feat = nullptr;
};

}
