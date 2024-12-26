#pragma once

#include "infrastructure/improvement.h"
#include "map/site.h"
#include "map/site_game_data.h"
#include "script/effect/effect.h"
#include "util/assert_util.h"
#include "util/string_util.h"

namespace kobold {

class improvements_effect final : public effect<const site>
{
public:
	explicit improvements_effect(const std::string &value, const gsml_operator effect_operator)
		: effect<const site>(effect_operator)
	{
		this->improvement = improvement::get(value);
	}

	virtual const std::string &get_class_identifier() const override
	{
		static const std::string class_identifier = "improvements";
		return class_identifier;
	}

	virtual void do_addition_effect(const site *scope, context &ctx) const override
	{
		Q_UNUSED(ctx);

		if (!this->improvement->is_buildable_on_site(scope)) {
			return;
		}

		scope->get_game_data()->set_improvement(this->improvement->get_slot(), this->improvement);
	}

	virtual void do_subtraction_effect(const site *scope, context &ctx) const override
	{
		Q_UNUSED(ctx);

		if (!scope->get_game_data()->has_improvement(this->improvement)) {
			return;
		}

		scope->get_game_data()->set_improvement(this->improvement->get_slot(), nullptr);
	}

	virtual std::string get_addition_string(const site *scope, const read_only_context &ctx, const size_t indent) const override
	{
		Q_UNUSED(scope);
		Q_UNUSED(ctx);
		Q_UNUSED(indent);

		return std::format("Gain {} improvement", string::highlight(this->improvement->get_name()));
	}

	virtual std::string get_subtraction_string(const site *scope, const read_only_context &ctx) const override
	{
		Q_UNUSED(scope);
		Q_UNUSED(ctx);

		return std::format("Lose {} improvement", string::highlight(this->improvement->get_name()));
	}

private:
	const kobold::improvement *improvement = nullptr;
};

}
