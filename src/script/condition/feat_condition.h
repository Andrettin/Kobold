#pragma once

#include "character/character.h"
#include "character/character_game_data.h"
#include "character/feat.h"
#include "country/country.h"
#include "country/country_feat.h"
#include "country/country_game_data.h"
#include "script/condition/condition.h"

namespace kobold {

template <typename scope_type>
class feat_condition final : public condition<scope_type>
{
public:
	using feat_type = std::conditional_t<std::is_same_v<scope_type, country>, country_feat, feat>;

	explicit feat_condition(const feat_type *feat, const gsml_operator condition_operator = gsml_operator::assignment)
		: condition<scope_type>(condition_operator)
	{
		this->feat = feat;
	}

	explicit feat_condition(const std::string &value, const gsml_operator condition_operator)
		: feat_condition(feat_type::get(value), condition_operator)
	{
	}

	virtual const std::string &get_class_identifier() const override
	{
		static const std::string class_identifier = "feat";
		return class_identifier;
	}

	virtual bool check_assignment(const scope_type *scope, const read_only_context &ctx) const override
	{
		Q_UNUSED(ctx);

		return scope->get_game_data()->has_feat(this->feat);
	}

	virtual std::string get_assignment_string(const size_t indent) const override
	{
		Q_UNUSED(indent);

		return this->feat->get_name() + " feat";
	}

private:
	const feat_type *feat = nullptr;
};

}
