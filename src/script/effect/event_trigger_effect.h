#pragma once

#include "game/event_trigger.h"
#include "script/effect/effect.h"
#include "util/assert_util.h"
#include "util/string_util.h"

#include <magic_enum/magic_enum.hpp>

namespace kobold {

template <typename scope_type>
class event_trigger_effect final : public effect<scope_type>
{
public:
	explicit event_trigger_effect(const std::string &value, const gsml_operator effect_operator) : effect<scope_type>(effect_operator)
	{
		this->event_trigger = magic_enum::enum_cast<kobold::event_trigger>(value).value();
	}

	virtual const std::string &get_class_identifier() const override
	{
		static const std::string class_identifier = "event_trigger";
		return class_identifier;
	}

	virtual void check() const override
	{
		if (this->event_trigger == event_trigger::none) {
			throw std::runtime_error(std::format("\"{}\" effect has no event trigger set for it.", this->get_class_identifier()));
		}
	}

	virtual void do_assignment_effect(scope_type *scope, context &ctx) const override
	{
		context event_ctx = ctx;
		event_ctx.root_scope = scope;
		event_ctx.source_scope = ctx.root_scope;

		scoped_event_base<scope_type>::check_events_for_scope(scope, this->event_trigger, ctx);
	}

	virtual std::string get_assignment_string(const scope_type *scope, const read_only_context &ctx, const size_t indent, const std::string &prefix) const override
	{
		Q_UNUSED(scope);
		Q_UNUSED(ctx);
		Q_UNUSED(indent);
		Q_UNUSED(prefix);

		const std::string_view event_trigger_name = get_event_trigger_name(this->event_trigger);

		return std::format("Trigger {} {} event", string::get_indefinite_article(std::string(event_trigger_name)), event_trigger_name);
	}

private:
	kobold::event_trigger event_trigger = event_trigger::none;
};

}
