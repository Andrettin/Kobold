#pragma once

#include "infrastructure/building_class.h"
#include "infrastructure/building_type.h"
#include "map/site.h"
#include "map/site_game_data.h"
#include "script/effect/effect.h"
#include "script/target_variant.h"
#include "util/assert_util.h"
#include "util/string_util.h"

namespace kobold {

class add_building_class_effect final : public effect<const site>
{
public:
	explicit add_building_class_effect(const std::string &value, const gsml_operator effect_operator)
		: effect<const site>(effect_operator)
	{
		this->building_class_target = string_to_target_variant<const building_class>(value);
	}

	virtual const std::string &get_class_identifier() const override
	{
		static const std::string class_identifier = "add_building_class";
		return class_identifier;
	}

	virtual void do_assignment_effect(const site *scope, context &ctx) const override
	{
		const building_class *building_class = this->get_building_class(ctx);

		if (building_class == nullptr) {
			return;
		}

		const building_type *building = scope->get_game_data()->get_building_class_type(building_class);

		if (building == nullptr) {
			return;
		}

		if (!scope->get_game_data()->can_gain_building(building)) {
			return;
		}

		scope->get_game_data()->add_building(building);
	}

	virtual std::string get_assignment_string(const site *scope, const read_only_context &ctx, const size_t indent, const std::string &prefix) const override
	{
		Q_UNUSED(ctx);
		Q_UNUSED(indent);
		Q_UNUSED(prefix);

		const building_class *building_class = this->get_building_class(ctx);

		if (building_class == nullptr) {
			return std::string();
		}

		const building_type *building = scope->get_game_data()->get_building_class_type(building_class);

		if (building == nullptr) {
			return std::string();
		}

		return std::format("Gain {} building", string::highlight(building->get_name()));
	}

	const building_class *get_building_class(const read_only_context &ctx) const
	{
		if (std::holds_alternative<const building_class *>(this->building_class_target)) {
			return std::get<const building_class *>(this->building_class_target);
		} else if (std::holds_alternative<std::string>(this->building_class_target)) {
			const std::string name = std::get<std::string>(this->building_class_target);
			const building_type *building = ctx.get_saved_building(name);
			if (building != nullptr) {
				return building->get_building_class();
			}

			return nullptr;
		}

		assert_throw(false);
		return nullptr;
	}

private:
	target_variant<const building_class> building_class_target;
};

}
