#pragma once

#include "character/character.h"
#include "character/character_game_data.h"
#include "character/feat.h"
#include "script/condition/condition.h"

namespace kobold {

class feat_condition final : public condition<character>
{
public:
	explicit feat_condition(const kobold::feat *feat, const gsml_operator condition_operator = gsml_operator::assignment)
		: condition<character>(condition_operator)
	{
		this->feat = feat;
	}

	explicit feat_condition(const std::string &value, const gsml_operator condition_operator)
		: feat_condition(feat::get(value), condition_operator)
	{
	}

	virtual const std::string &get_class_identifier() const override
	{
		static const std::string class_identifier = "feat";
		return class_identifier;
	}

	virtual bool check_assignment(const character *scope, const read_only_context &ctx) const override
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
	const kobold::feat *feat = nullptr;
};

}
