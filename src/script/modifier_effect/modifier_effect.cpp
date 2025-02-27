#include "kobold.h"

#include "script/modifier_effect/modifier_effect.h"

#include "character/character.h"
#include "script/modifier_effect/ai_building_desire_modifier_effect.h"
#include "script/modifier_effect/armor_class_modifier_effect.h"
#include "script/modifier_effect/artillery_cost_modifier_effect.h"
#include "script/modifier_effect/attack_bonus_modifier_effect.h"
#include "script/modifier_effect/base_attack_bonus_modifier_effect.h"
#include "script/modifier_effect/building_cost_efficiency_modifier_effect.h"
#include "script/modifier_effect/capital_commodity_bonus_modifier_effect.h"
#include "script/modifier_effect/capital_commodity_output_modifier_effect.h"
#include "script/modifier_effect/cavalry_cost_modifier_effect.h"
#include "script/modifier_effect/challenge_rating_modifier_effect.h"
#include "script/modifier_effect/character_attribute_modifier_effect.h"
#include "script/modifier_effect/commodity_bonus_modifier_effect.h"
#include "script/modifier_effect/commodity_bonus_for_tile_threshold_modifier_effect.h"
#include "script/modifier_effect/commodity_bonus_per_adjacent_terrain_modifier_effect.h"
#include "script/modifier_effect/commodity_bonus_per_building_modifier_effect.h"
#include "script/modifier_effect/commodity_bonus_per_improved_resource_modifier_effect.h"
#include "script/modifier_effect/commodity_bonus_per_improvement_modifier_effect.h"
#include "script/modifier_effect/commodity_bonus_per_settlement_modifier_effect.h"
#include "script/modifier_effect/commodity_output_modifier_effect.h"
#include "script/modifier_effect/commodity_throughput_modifier_effect.h"
#include "script/modifier_effect/consumption_modifier_effect.h"
#include "script/modifier_effect/country_attribute_modifier_effect.h"
#include "script/modifier_effect/damage_reduction_modifier_effect.h"
#include "script/modifier_effect/deployment_limit_modifier_effect.h"
#include "script/modifier_effect/diplomatic_penalty_for_expansion_modifier_effect.h"
#include "script/modifier_effect/entrenchment_bonus_modifier_effect.h"
#include "script/modifier_effect/free_artillery_promotion_modifier_effect.h"
#include "script/modifier_effect/free_building_class_modifier_effect.h"
#include "script/modifier_effect/free_cavalry_promotion_modifier_effect.h"
#include "script/modifier_effect/free_consulate_modifier_effect.h"
#include "script/modifier_effect/free_infantry_promotion_modifier_effect.h"
#include "script/modifier_effect/free_warship_promotion_modifier_effect.h"
#include "script/modifier_effect/hit_points_modifier_effect.h"
#include "script/modifier_effect/infantry_cost_modifier_effect.h"
#include "script/modifier_effect/initiative_modifier_effect.h"
#include "script/modifier_effect/law_cost_modifier_effect.h"
#include "script/modifier_effect/military_unit_category_stat_modifier_effect.h"
#include "script/modifier_effect/military_unit_domain_stat_modifier_effect.h"
#include "script/modifier_effect/military_unit_stat_modifier_effect.h"
#include "script/modifier_effect/military_unit_type_stat_modifier_effect.h"
#include "script/modifier_effect/output_modifier_effect.h"
#include "script/modifier_effect/saving_throw_modifier_effect.h"
#include "script/modifier_effect/settlement_attribute_modifier_effect.h"
#include "script/modifier_effect/ship_stat_modifier_effect.h"
#include "script/modifier_effect/skill_modifier_effect.h"
#include "script/modifier_effect/skill_per_level_modifier_effect.h"
#include "script/modifier_effect/storage_capacity_modifier_effect.h"
#include "script/modifier_effect/throughput_modifier_effect.h"
#include "script/modifier_effect/unit_upgrade_cost_modifier_effect.h"
#include "script/modifier_effect/unrest_modifier_effect.h"
#include "script/modifier_effect/warship_cost_modifier_effect.h"
#include "script/modifier_effect/weapon_attack_bonus_modifier_effect.h"
#include "script/modifier_effect/wonder_cost_efficiency_modifier_effect.h"

namespace kobold {

template <typename scope_type>
std::unique_ptr<modifier_effect<scope_type>> modifier_effect<scope_type>::from_gsml_property(const gsml_property &property)
{
	const std::string &key = property.get_key();
	const std::string &value = property.get_value();

	static const std::string bonus_suffix = "_bonus";
	static const std::string per_level_suffix = "_per_level";

	if constexpr (std::is_same_v<scope_type, const character>) {
		if (key == "armor_class") {
			return std::make_unique<armor_class_modifier_effect>(value);
		} else if (key == "attack_bonus") {
			return std::make_unique<attack_bonus_modifier_effect>(value);
		} else if (key == "base_attack_bonus") {
			return std::make_unique<base_attack_bonus_modifier_effect>(value);
		} else if (key == "challenge_rating") {
			return std::make_unique<challenge_rating_modifier_effect>(value);
		} else if (key == "hit_points") {
			return std::make_unique<hit_points_modifier_effect>(value);
		} else if (key == "initiative") {
			return std::make_unique<initiative_modifier_effect>(value);
		} else if (character_attribute::try_get(key) != nullptr) {
			return std::make_unique<character_attribute_modifier_effect>(character_attribute::get(key), value);
		} else if (saving_throw_type::try_get(key) != nullptr) {
			return std::make_unique<saving_throw_modifier_effect>(saving_throw_type::get(key), value);
		} else if (damage_reduction_type::try_get(key) != nullptr) {
			return std::make_unique<damage_reduction_modifier_effect>(damage_reduction_type::get(key), value);
		}
	} else if constexpr (std::is_same_v<scope_type, const country>) {
		static const std::string capital_commodity_bonus_prefix = "capital_";
		static const std::string commodity_per_building_infix = "_per_";
		static const std::string militancy_modifier_suffix = "_militancy_modifier";
		static const std::string military_unit_type_stat_modifier_infix = "_";
		static const std::string military_unit_type_stat_modifier_suffix = "_modifier";
		static const std::string output_modifier_suffix = "_output_modifier";
		static const std::string research_modifier_suffix = "_research_modifier";
		static const std::string ship_stat_modifier_prefix = "ship_";
		static const std::string throughput_modifier_suffix = "_throughput_modifier";

		if (key == "artillery_cost_modifier") {
			return std::make_unique<artillery_cost_modifier_effect>(value);
		} else if (key == "building_cost_efficiency") {
			return std::make_unique<building_cost_efficiency_modifier_effect>(value);
		} else if (key == "cavalry_cost_modifier") {
			return std::make_unique<cavalry_cost_modifier_effect>(value);
		} else if (key == "consumption") {
			return std::make_unique<consumption_modifier_effect>(value);
		} else if (key == "deployment_limit") {
			return std::make_unique<deployment_limit_modifier_effect>(value);
		} else if (key == "diplomatic_penalty_for_expansion_modifier") {
			return std::make_unique<diplomatic_penalty_for_expansion_modifier_effect>(value);
		} else if (key == "entrenchment_bonus_modifier") {
			return std::make_unique<entrenchment_bonus_modifier_effect>(value);
		} else if (key == "free_artillery_promotion") {
			return std::make_unique<free_artillery_promotion_modifier_effect>(value);
		} else if (key == "free_building_class") {
			return std::make_unique<free_building_class_modifier_effect>(value);
		} else if (key == "free_cavalry_promotion") {
			return std::make_unique<free_cavalry_promotion_modifier_effect>(value);
		} else if (key == "free_consulate") {
			return std::make_unique<free_consulate_modifier_effect>(value);
		} else if (key == "free_infantry_promotion") {
			return std::make_unique<free_infantry_promotion_modifier_effect>(value);
		} else if (key == "free_warship_promotion") {
			return std::make_unique<free_warship_promotion_modifier_effect>(value);
		} else if (key == "infantry_cost_modifier") {
			return std::make_unique<infantry_cost_modifier_effect>(value);
		} else if (key == "law_cost_modifier") {
			return std::make_unique<law_cost_modifier_effect>(value);
		} else if (key == "storage_capacity") {
			return std::make_unique<storage_capacity_modifier_effect>(value);
		} else if (key == "throughput_modifier") {
			return std::make_unique<throughput_modifier_effect<scope_type>>(value);
		} else if (key == "unit_upgrade_cost_modifier") {
			return std::make_unique<unit_upgrade_cost_modifier_effect>(value);
		} else if (key == "unrest") {
			return std::make_unique<unrest_modifier_effect>(value);
		} else if (key == "warship_cost_modifier") {
			return std::make_unique<warship_cost_modifier_effect>(value);
		} else if (key == "wonder_cost_efficiency") {
			return std::make_unique<wonder_cost_efficiency_modifier_effect>(value);
		} else if (country_attribute::try_get(key) != nullptr) {
			return std::make_unique<country_attribute_modifier_effect>(country_attribute::get(key), value);
		} else if (key.starts_with(capital_commodity_bonus_prefix) && key.ends_with(bonus_suffix) && commodity::try_get(key.substr(capital_commodity_bonus_prefix.size(), key.size() - capital_commodity_bonus_prefix.size() - bonus_suffix.size())) != nullptr) {
			const commodity *commodity = commodity::get(key.substr(capital_commodity_bonus_prefix.size(), key.size() - capital_commodity_bonus_prefix.size() - bonus_suffix.size()));
			return std::make_unique<capital_commodity_bonus_modifier_effect>(commodity, value);
		} else if (key.starts_with(capital_commodity_bonus_prefix) && key.ends_with(output_modifier_suffix) && commodity::try_get(key.substr(capital_commodity_bonus_prefix.size(), key.size() - capital_commodity_bonus_prefix.size() - output_modifier_suffix.size())) != nullptr) {
			const commodity *commodity = commodity::get(key.substr(capital_commodity_bonus_prefix.size(), key.size() - capital_commodity_bonus_prefix.size() - output_modifier_suffix.size()));
			return std::make_unique<capital_commodity_output_modifier_effect>(commodity, value);
		} else if (key.ends_with(throughput_modifier_suffix) && commodity::try_get(key.substr(0, key.size() - throughput_modifier_suffix.size())) != nullptr) {
			const commodity *commodity = commodity::get(key.substr(0, key.size() - throughput_modifier_suffix.size()));
			return std::make_unique<commodity_throughput_modifier_effect<scope_type>>(commodity, value);
		}
		
		size_t infix_pos = key.find(commodity_per_building_infix);
		if (infix_pos != std::string::npos && !key.starts_with(commodity_per_building_infix) && !key.ends_with(commodity_per_building_infix) && building_type::try_get(key.substr(infix_pos + commodity_per_building_infix.size(), key.size() - commodity_per_building_infix.size() - infix_pos)) != nullptr) {
			const commodity *commodity = commodity::get(key.substr(0, infix_pos));

			const size_t building_identifier_pos = infix_pos + commodity_per_building_infix.size();
			const building_type *building = building_type::get(key.substr(building_identifier_pos, key.size() - building_identifier_pos));

			return std::make_unique<commodity_bonus_per_building_modifier_effect<scope_type>>(commodity, building, value);
		}
		
		const size_t suffix_pos = key.rfind(military_unit_type_stat_modifier_suffix);
		if (suffix_pos != std::string::npos && key.ends_with(military_unit_type_stat_modifier_suffix)) {
			if (key.starts_with(ship_stat_modifier_prefix)) {
				const std::string stat_name = key.substr(ship_stat_modifier_prefix.size(), key.size() - ship_stat_modifier_prefix.size() - military_unit_type_stat_modifier_suffix.size());

				return std::make_unique<ship_stat_modifier_effect>(stat_name, value);
			}

			infix_pos = key.rfind(military_unit_type_stat_modifier_infix, suffix_pos - 1);
			if (infix_pos != std::string::npos) {
				if (
					enum_converter<military_unit_stat>::has_value(key.substr(infix_pos + military_unit_type_stat_modifier_infix.size(), suffix_pos - infix_pos - 1))
					&& military_unit_type::try_get(key.substr(0, infix_pos)) != nullptr
				) {
					const military_unit_stat stat = enum_converter<military_unit_stat>::to_enum(key.substr(infix_pos + military_unit_type_stat_modifier_infix.size(), suffix_pos - infix_pos - 1));

					const military_unit_type *military_unit_type = military_unit_type::get(key.substr(0, infix_pos));

					return std::make_unique<military_unit_type_stat_modifier_effect>(military_unit_type, stat, value);
				}

				infix_pos = key.rfind(military_unit_type_stat_modifier_infix, infix_pos - 1);
				if (
					infix_pos != std::string::npos
					&& enum_converter<military_unit_stat>::has_value(key.substr(infix_pos + military_unit_type_stat_modifier_infix.size(), suffix_pos - infix_pos - 1))
					&& military_unit_type::try_get(key.substr(0, infix_pos)) != nullptr
				) {
					const military_unit_stat stat = enum_converter<military_unit_stat>::to_enum(key.substr(infix_pos + military_unit_type_stat_modifier_infix.size(), suffix_pos - infix_pos - 1));

					const military_unit_type *military_unit_type = military_unit_type::get(key.substr(0, infix_pos));

					return std::make_unique<military_unit_type_stat_modifier_effect>(military_unit_type, stat, value);
				}
			}
		}
	} else if constexpr (std::is_same_v<scope_type, const site>) {
		if (key.ends_with(bonus_suffix)) {
			const size_t commodity_identifier_size = key.size() - bonus_suffix.size();
			const commodity *commodity = commodity::get(key.substr(0, commodity_identifier_size));

			return std::make_unique<commodity_bonus_modifier_effect>(commodity, value);
		}
	}

	if constexpr (std::is_same_v<scope_type, const character> || std::is_same_v<scope_type, const country>) {
		if (skill_modifier_effect<scope_type>::skill_type::try_get(key) != nullptr) {
			return std::make_unique<skill_modifier_effect<scope_type>>(skill_modifier_effect<scope_type>::skill_type::get(key), value);
		}

		if (key.ends_with(per_level_suffix)) {
			const size_t skill_identifier_size = key.size() - per_level_suffix.size();
			const skill_per_level_modifier_effect<scope_type>::skill_type *skill = skill_per_level_modifier_effect<scope_type>::skill_type::get(key.substr(0, skill_identifier_size));

			return std::make_unique<skill_per_level_modifier_effect<scope_type>>(skill, value);
		}
	}

	if constexpr (std::is_same_v<scope_type, const character> || std::is_same_v<scope_type, military_unit>) {
		if (enum_converter<military_unit_stat>::has_value(key)) {
			return std::make_unique<military_unit_stat_modifier_effect<scope_type>>(enum_converter<military_unit_stat>::to_enum(key), value);
		}
	}

	if constexpr (std::is_same_v<scope_type, const country> || std::is_same_v<scope_type, const province>) {
		static const std::string commodity_bonus_for_tile_threshold_suffix = "_bonus_for_tile_threshold";
		static const std::string commodity_per_improved_resource_infix = "_per_improved_";

		if (key.ends_with(commodity_bonus_for_tile_threshold_suffix)) {
			const size_t commodity_identifier_size = key.size() - commodity_bonus_for_tile_threshold_suffix.size();
			const commodity *commodity = commodity::get(key.substr(0, commodity_identifier_size));

			return std::make_unique<commodity_bonus_for_tile_threshold_modifier_effect<scope_type>>(commodity, value);
		} else if (key.find(commodity_per_improved_resource_infix) != std::string::npos && !key.starts_with(commodity_per_improved_resource_infix) && !key.ends_with(commodity_per_improved_resource_infix)) {
			const size_t infix_pos = key.find(commodity_per_improved_resource_infix);
			const commodity *commodity = commodity::get(key.substr(0, infix_pos));

			const size_t resource_identifier_pos = infix_pos + commodity_per_improved_resource_infix.size();
			const resource *resource = resource::get(key.substr(resource_identifier_pos, key.size() - resource_identifier_pos));

			return std::make_unique<commodity_bonus_per_improved_resource_modifier_effect<scope_type>>(commodity, resource, value);
		}
	}

	if constexpr (std::is_same_v<scope_type, const country> || std::is_same_v<scope_type, const province> || std::is_same_v<scope_type, const site>) {
		static const std::string output_modifier_suffix = "_output_modifier";

		if (key == "output_modifier") {
			return std::make_unique<output_modifier_effect<scope_type>>(value);
		} else if (key.ends_with(output_modifier_suffix) && commodity::try_get(key.substr(0, key.size() - output_modifier_suffix.size())) != nullptr) {
			const commodity *commodity = commodity::get(key.substr(0, key.size() - output_modifier_suffix.size()));
			return std::make_unique<commodity_output_modifier_effect<scope_type>>(commodity, value);
		}
	}

	if constexpr (std::is_same_v<scope_type, const country> || std::is_same_v<scope_type, const site>) {
		if (settlement_attribute::try_get(key) != nullptr) {
			return std::make_unique<settlement_attribute_modifier_effect<scope_type>>(settlement_attribute::get(key), value);
		}
	}

	throw std::runtime_error(std::format("Invalid property modifier effect: \"{}\".", key));
}


template <typename scope_type>
std::unique_ptr<modifier_effect<scope_type>> modifier_effect<scope_type>::from_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();
	std::unique_ptr<modifier_effect> modifier_effect;

	if constexpr (std::is_same_v<scope_type, const character>) {
		if (tag == "weapon_attack_bonus") {
			modifier_effect = std::make_unique<weapon_attack_bonus_modifier_effect>();
		}
	} else if constexpr (std::is_same_v<scope_type, const country>) {
		if (tag == "ai_building_desire") {
			modifier_effect = std::make_unique<ai_building_desire_modifier_effect>();
		} else if (tag == "commodity_bonus_per_improvement") {
			modifier_effect = std::make_unique<commodity_bonus_per_improvement_modifier_effect<scope_type>>();
		} else if (tag == "commodity_bonus_per_settlement") {
			modifier_effect = std::make_unique<commodity_bonus_per_settlement_modifier_effect<scope_type>>();
		} else if (tag == "military_unit_domain_stat_modifier") {
			modifier_effect = std::make_unique<military_unit_domain_stat_modifier_effect>();
		}
	} else if constexpr (std::is_same_v<scope_type, const site>) {
		if (tag == "commodity_bonus_per_adjacent_terrain") {
			modifier_effect = std::make_unique<commodity_bonus_per_adjacent_terrain_modifier_effect>();
		}
	}
	
	if constexpr (std::is_same_v<scope_type, const character> || std::is_same_v<scope_type, const country>) {
		if (tag == "military_unit_category_stat_modifier") {
			modifier_effect = std::make_unique<military_unit_category_stat_modifier_effect<scope_type>>();
		}
	}

	if (modifier_effect == nullptr) {
		throw std::runtime_error(std::format("Invalid scope modifier effect: \"{}\".", tag));
	}

	database::process_gsml_data(modifier_effect, scope);

	return modifier_effect;
}

template class modifier_effect<const character>;
template class modifier_effect<const country>;
template class modifier_effect<military_unit>;
template class modifier_effect<const province>;
template class modifier_effect<const site>;

}
