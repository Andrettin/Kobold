#include "kobold.h"

#include "script/condition/condition.h"

#include "character/character.h"
#include "character/character_game_data.h"
#include "character/skill.h"
#include "country/country_game_data.h"
#include "database/database.h"
#include "database/gsml_operator.h"
#include "database/named_data_entry.h"
#include "map/province.h"
#include "map/province_game_data.h"
#include "script/condition/adjacent_terrain_condition.h"
#include "script/condition/age_condition.h"
#include "script/condition/anarchy_condition.h"
#include "script/condition/and_condition.h"
#include "script/condition/any_known_country_condition.h"
#include "script/condition/any_neighbor_country_condition.h"
#include "script/condition/any_settlement_condition.h"
#include "script/condition/artillery_condition.h"
#include "script/condition/attacking_commander_condition.h"
#include "script/condition/base_attack_bonus_condition.h"
#include "script/condition/can_gain_building_class_condition.h"
#include "script/condition/can_have_feat_condition.h"
#include "script/condition/capital_condition.h"
#include "script/condition/cavalry_condition.h"
#include "script/condition/character_attribute_condition.h"
#include "script/condition/character_class_condition.h"
#include "script/condition/class_skill_condition.h"
#include "script/condition/coastal_condition.h"
#include "script/condition/commodity_condition.h"
#include "script/condition/condition_base.ipp"
#include "script/condition/core_condition.h"
#include "script/condition/country_condition.h"
#include "script/condition/country_exists_condition.h"
#include "script/condition/country_scope_condition.h"
#include "script/condition/country_type_condition.h"
#include "script/condition/cultural_group_condition.h"
#include "script/condition/culture_condition.h"
#include "script/condition/discovered_province_condition.h"
#include "script/condition/discovered_region_condition.h"
#include "script/condition/event_condition.h"
#include "script/condition/feat_condition.h"
#include "script/condition/game_rule_condition.h"
#include "script/condition/government_type_condition.h"
#include "script/condition/has_building_condition.h"
#include "script/condition/has_building_class_condition.h"
#include "script/condition/has_flag_condition.h"
#include "script/condition/has_resource_condition.h"
#include "script/condition/has_route_condition.h"
#include "script/condition/has_terrain_condition.h"
#include "script/condition/hit_dice_condition.h"
#include "script/condition/improvement_condition.h"
#include "script/condition/independent_condition.h"
#include "script/condition/infantry_condition.h"
#include "script/condition/is_military_unit_category_available_condition.h"
#include "script/condition/is_ruler_condition.h"
#include "script/condition/is_ruler_spouse_condition.h"
#include "script/condition/law_condition.h"
#include "script/condition/level_condition.h"
#include "script/condition/military_unit_category_condition.h"
#include "script/condition/military_unit_domain_condition.h"
#include "script/condition/military_unit_type_condition.h"
#include "script/condition/near_water_condition.h"
#include "script/condition/owns_province_condition.h"
#include "script/condition/owns_site_condition.h"
#include "script/condition/produces_commodity_condition.h"
#include "script/condition/promotion_condition.h"
#include "script/condition/province_condition.h"
#include "script/condition/province_count_condition.h"
#include "script/condition/provincial_capital_condition.h"
#include "script/condition/religion_condition.h"
#include "script/condition/religious_group_condition.h"
#include "script/condition/resource_condition.h"
#include "script/condition/river_condition.h"
#include "script/condition/root_character_condition.h"
#include "script/condition/ruler_condition.h"
#include "script/condition/ruler_scope_condition.h"
#include "script/condition/saved_scope_condition.h"
#include "script/condition/scripted_condition_condition.h"
#include "script/condition/scripted_modifier_condition.h"
#include "script/condition/settlement_type_condition.h"
#include "script/condition/site_condition.h"
#include "script/condition/site_count_condition.h"
#include "script/condition/skill_condition.h"
#include "script/condition/source_character_condition.h"
#include "script/condition/source_site_condition.h"
#include "script/condition/source_site_scope_condition.h"
#include "script/condition/species_condition.h"
#include "script/condition/subject_type_condition.h"
#include "script/condition/terrain_condition.h"
#include "script/condition/tradition_condition.h"
#include "script/condition/war_condition.h"
#include "script/condition/year_condition.h"
#include "unit/military_unit.h"
#include "util/string_util.h"

namespace kobold {

template <typename scope_type>
std::unique_ptr<const condition_base<scope_type, read_only_context>> condition<scope_type>::from_gsml_property(const gsml_property &property)
{
	const std::string &key = property.get_key();
	const gsml_operator condition_operator = property.get_operator();
	const std::string &value = property.get_value();

	if constexpr (std::is_same_v<scope_type, character>) {
		if (key == "age") {
			return std::make_unique<age_condition>(value, condition_operator);
		} else if (key == "base_attack_bonus") {
			return std::make_unique<base_attack_bonus_condition>(value, condition_operator);
		} else if (key == "character_class") {
			return std::make_unique<character_class_condition>(value, condition_operator);
		} else if (key == "class_skill") {
			return std::make_unique<class_skill_condition>(value, condition_operator);
		} else if (key == "hit_dice") {
			return std::make_unique<hit_dice_condition>(value, condition_operator);
		} else if (key == "is_ruler") {
			return std::make_unique<is_ruler_condition>(value, condition_operator);
		} else if (key == "is_ruler_spouse") {
			return std::make_unique<is_ruler_spouse_condition>(value, condition_operator);
		} else if (key == "species") {
			return std::make_unique<species_condition>(value, condition_operator);
		} else if (character_attribute::try_get(key) != nullptr) {
			return std::make_unique<character_attribute_condition>(character_attribute::get(key), value, condition_operator);
		} else if (skill::try_get(key) != nullptr) {
			return std::make_unique<skill_condition>(skill::get(key), value, condition_operator);
		}
	} else if constexpr (std::is_same_v<scope_type, country>) {
		if (key == "anarchy") {
			return std::make_unique<anarchy_condition>(value, condition_operator);
		} else if (key == "country_type") {
			return std::make_unique<country_type_condition>(value, condition_operator);
		} else if (key == "discovered_province") {
			return std::make_unique<discovered_province_condition>(value, condition_operator);
		} else if (key == "discovered_region") {
			return std::make_unique<discovered_region_condition>(value, condition_operator);
		} else if (key == "government_type") {
			return std::make_unique<government_type_condition>(value, condition_operator);
		} else if (key == "independent") {
			return std::make_unique<independent_condition>(value, condition_operator);
		} else if (key == "is_military_unit_category_available") {
			return std::make_unique<is_military_unit_category_available_condition>(value, condition_operator);
		} else if (key == "owns_province") {
			return std::make_unique<owns_province_condition>(value, condition_operator);
		} else if (key == "owns_site") {
			return std::make_unique<owns_site_condition>(value, condition_operator);
		} else if (key == "province_count") {
			return std::make_unique<province_count_condition>(value, condition_operator);
		} else if (key == "ruler") {
			return std::make_unique<ruler_condition>(value, condition_operator);
		} else if (key == "site_count") {
			return std::make_unique<site_count_condition>(value, condition_operator);
		} else if (key == "subject_type") {
			return std::make_unique<subject_type_condition>(value, condition_operator);
		}
	} else if constexpr (std::is_same_v<scope_type, military_unit>) {
		if (key == "artillery") {
			return std::make_unique<artillery_condition>(value, condition_operator);
		} else if (key == "cavalry") {
			return std::make_unique<cavalry_condition>(value, condition_operator);
		} else if (key == "infantry") {
			return std::make_unique<infantry_condition>(value, condition_operator);
		} else if (key == "military_unit_category") {
			return std::make_unique<military_unit_category_condition<scope_type>>(value, condition_operator);
		} else if (key == "military_unit_domain") {
			return std::make_unique<military_unit_domain_condition>(value, condition_operator);
		} else if (key == "military_unit_type") {
			return std::make_unique<military_unit_type_condition>(value, condition_operator);
		} else if (key == "promotion") {
			return std::make_unique<promotion_condition>(value, condition_operator);
		}
	} else if constexpr (std::is_same_v<scope_type, province>) {
		if (key == "core") {
			return std::make_unique<core_condition>(value, condition_operator);
		}
	} else if constexpr (std::is_same_v<scope_type, site>) {
		if (key == "adjacent_terrain") {
			return std::make_unique<adjacent_terrain_condition>(value, condition_operator);
		} else if (key == "can_gain_building_class") {
			return std::make_unique<can_gain_building_class_condition>(value, condition_operator);
		} else if (key == "has_route") {
			return std::make_unique<has_route_condition<site>>(value, condition_operator);
		} else if (key == "improvement") {
			return std::make_unique<improvement_condition>(value, condition_operator);
		} else if (key == "resource") {
			return std::make_unique<resource_condition>(value, condition_operator);
		} else if (key == "river") {
			return std::make_unique<river_condition>(value, condition_operator);
		} else if (key == "settlement_type") {
			return std::make_unique<settlement_type_condition>(value, condition_operator);
		} else if (key == "site") {
			return std::make_unique<site_condition>(value, condition_operator);
		} else if (key == "terrain") {
			return std::make_unique<terrain_condition>(value, condition_operator);
		}
	}
	
	if constexpr (std::is_same_v<scope_type, character> || std::is_same_v<scope_type, country>) {
		if (key == "can_have_feat") {
			return std::make_unique<can_have_feat_condition<scope_type>>(value, condition_operator);
		} else if (key == "feat") {
			return std::make_unique<feat_condition<scope_type>>(value, condition_operator);
		} else if (key == "level") {
			return std::make_unique<level_condition<scope_type>>(value, condition_operator);
		} else if (key == "war") {
			return std::make_unique<war_condition<scope_type>>(value, condition_operator);
		} else if (commodity::try_get(key) != nullptr && string::is_number(value, true)) {
			return std::make_unique<commodity_condition<scope_type>>(commodity::get(key), value, condition_operator);
		}
	}
	
	if constexpr (std::is_same_v<scope_type, character> || std::is_same_v<scope_type, country> || std::is_same_v<scope_type, province> || std::is_same_v<scope_type, site>) {
		if (key == "scripted_modifier") {
			return std::make_unique<scripted_modifier_condition<scope_type>>(value, condition_operator);
		}
	}
	
	if constexpr (std::is_same_v<scope_type, country> || std::is_same_v<scope_type, province>) {
		if (key == "has_resource") {
			return std::make_unique<has_resource_condition<scope_type>>(value, condition_operator);
		} else if (key == "has_terrain") {
			return std::make_unique<has_terrain_condition<scope_type>>(value, condition_operator);
		} else if (key == "produces_commodity") {
			return std::make_unique<produces_commodity_condition<scope_type>>(value, condition_operator);
		}
	}

	if constexpr (std::is_same_v<scope_type, country> || std::is_same_v<scope_type, site>) {
		if (key == "has_flag") {
			return std::make_unique<has_flag_condition<scope_type>>(value, condition_operator);
		}
	}

	if constexpr (std::is_same_v<scope_type, country> || std::is_same_v<scope_type, province> || std::is_same_v<scope_type, site>) {
		if (key == "capital") {
			return std::make_unique<capital_condition<scope_type>>(value, condition_operator);
		} else if (key == "coastal") {
			return std::make_unique<coastal_condition<scope_type>>(value, condition_operator);
		} else if (key == "has_building") {
			return std::make_unique<has_building_condition<scope_type>>(value, condition_operator);
		} else if (key == "has_building_class") {
			return std::make_unique<has_building_class_condition<scope_type>>(value, condition_operator);
		}
	}

	if constexpr (std::is_same_v<scope_type, province> || std::is_same_v<scope_type, site>) {
		if (key == "near_water") {
			return std::make_unique<near_water_condition<scope_type>>(value, condition_operator);
		} else if (key == "provincial_capital") {
			return std::make_unique<provincial_capital_condition<scope_type>>(value, condition_operator);
		}
	}

	if (key == "country") {
		return std::make_unique<country_condition<scope_type>>(value, condition_operator);
	} else if (key == "country_exists") {
		return std::make_unique<country_exists_condition<scope_type>>(value, condition_operator);
	} else if (key == "cultural_group") {
		return std::make_unique<cultural_group_condition<scope_type>>(value, condition_operator);
	} else if (key == "culture") {
		return std::make_unique<culture_condition<scope_type>>(value, condition_operator);
	} else if (key == "event") {
		return std::make_unique<event_condition<scope_type>>(value, condition_operator);
	} else if (key == "game_rule") {
		return std::make_unique<game_rule_condition<scope_type>>(value, condition_operator);
	} else if (key == "law") {
		return std::make_unique<law_condition<scope_type>>(value, condition_operator);
	} else if (key == "religion") {
		return std::make_unique<religion_condition<scope_type>>(value, condition_operator);
	} else if (key == "religious_group") {
		return std::make_unique<religious_group_condition<scope_type>>(value, condition_operator);
	} else if (key == "root_character") {
		return std::make_unique<root_character_condition<scope_type>>(value, condition_operator);
	} else if (key == "scripted_condition") {
		return std::make_unique<scripted_condition_condition<scope_type>>(value, condition_operator);
	} else if (key == "source_character") {
		return std::make_unique<source_character_condition<scope_type>>(value, condition_operator);
	} else if (key == "source_site") {
		return std::make_unique<source_site_condition<scope_type>>(value, condition_operator);
	} else if (key == "tradition") {
		return std::make_unique<tradition_condition<scope_type>>(value, condition_operator);
	} else if (key == "year") {
		return std::make_unique<year_condition<scope_type>>(value, condition_operator);
	}

	return condition_base<scope_type, read_only_context>::from_gsml_property(property);
}

template <typename scope_type>
std::unique_ptr<const condition_base<scope_type, read_only_context>> condition<scope_type>::from_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const gsml_operator condition_operator = scope.get_operator();
	std::unique_ptr<condition_base<scope_type, read_only_context>> condition;

	if constexpr (std::is_same_v<scope_type, country>) {
		if (tag == "any_known_country") {
			condition = std::make_unique<any_known_country_condition>(condition_operator);
		} else if (tag == "any_neighbor_country") {
			condition = std::make_unique<any_neighbor_country_condition>(condition_operator);
		}
	}

	if constexpr (std::is_same_v<scope_type, country> || std::is_same_v<scope_type, province>) {
		if (tag == "any_settlement") {
			condition = std::make_unique<any_settlement_condition<scope_type>>(condition_operator);
		} else if (tag == "ruler") {
			condition = std::make_unique<ruler_scope_condition<scope_type>>(condition_operator);
		}
	}

	if constexpr (std::is_same_v<scope_type, site>) {
		if (tag == "province") {
			condition = std::make_unique<province_condition<scope_type>>(condition_operator);
		}
	}

	if (tag == "attacking_commander") {
		condition = std::make_unique<attacking_commander_condition<scope_type>>(condition_operator);
	} else if (tag == "country") {
		condition = std::make_unique<country_scope_condition<scope_type>>(condition_operator);
	} else if (tag == "saved_character_scope") {
		condition = std::make_unique<saved_scope_condition<scope_type, character, read_only_context, kobold::condition<character>>>(condition_operator);
	} else if (tag == "saved_country_scope") {
		condition = std::make_unique<saved_scope_condition<scope_type, country, read_only_context, kobold::condition<country>>>(condition_operator);
	} else if (tag == "saved_province_scope") {
		condition = std::make_unique<saved_scope_condition<scope_type, province, read_only_context, kobold::condition<province>>>(condition_operator);
	} else if (tag == "saved_site_scope") {
		condition = std::make_unique<saved_scope_condition<scope_type, site, read_only_context, kobold::condition<site>>>(condition_operator);
	} else if (tag == "source_site") {
		condition = std::make_unique<source_site_scope_condition<scope_type>>(condition_operator);
	}

	if (condition != nullptr) {
		database::process_gsml_data(condition, scope);
		return condition;
	}

	return condition_base<scope_type, read_only_context>::from_gsml_scope<kobold::condition<scope_type>>(scope);
}

template <typename scope_type>
const country *condition<scope_type>::get_scope_country(const scope_type *scope)
{
	if constexpr (std::is_same_v<scope_type, character>) {
		return scope->get_game_data()->get_country();
	} else if constexpr (std::is_same_v<scope_type, country>) {
		return scope;
	} else if constexpr (std::is_same_v<scope_type, military_unit>) {
		return scope->get_country();
	} else if constexpr (std::is_same_v<scope_type, province> || std::is_same_v<scope_type, site>) {
		return scope->get_game_data()->get_owner();
	}
}

template <typename scope_type>
const province *condition<scope_type>::get_scope_province(const scope_type *scope)
{
	if constexpr (std::is_same_v<scope_type, province>) {
		return scope;
	} else if constexpr (std::is_same_v<scope_type, site>) {
		return scope->get_game_data()->get_province();
	}
}

}
