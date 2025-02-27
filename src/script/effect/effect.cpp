#include "kobold.h"

#include "script/effect/effect.h"

#include "character/character.h"
#include "character/character_game_data.h"
#include "country/country.h"
#include "country/country_game_data.h"
#include "country/office.h"
#include "database/database.h"
#include "map/province.h"
#include "map/province_game_data.h"
#include "map/site.h"
#include "map/site_game_data.h"
#include "script/effect/add_building_class_effect.h"
#include "script/effect/any_known_country_effect.h"
#include "script/effect/any_neighbor_country_effect.h"
#include "script/effect/battle_effect.h"
#include "script/effect/capital_effect.h"
#include "script/effect/change_opinion_effect.h"
#include "script/effect/character_scope_effect.h"
#include "script/effect/clear_flag_effect.h"
#include "script/effect/combat_effect.h"
#include "script/effect/commodity_effect.h"
#include "script/effect/commodity_percent_effect.h"
#include "script/effect/country_effect.h"
#include "script/effect/create_character_effect.h"
#include "script/effect/create_military_unit_effect.h"
#include "script/effect/delayed_effect.h"
#include "script/effect/else_effect.h"
#include "script/effect/event_effect.h"
#include "script/effect/event_trigger_effect.h"
#include "script/effect/experience_effect.h"
#include "script/effect/gain_feat_effect.h"
#include "script/effect/gain_feat_of_type_effect.h"
#include "script/effect/gain_spell_scroll_effect.h"
#include "script/effect/hidden_effect.h"
#include "script/effect/if_effect.h"
#include "script/effect/improvements_effect.h"
#include "script/effect/location_effect.h"
#include "script/effect/office_holder_effect.h"
#include "script/effect/opinion_modifiers_effect.h"
#include "script/effect/provincial_capital_scope_effect.h"
#include "script/effect/random_effect.h"
#include "script/effect/random_known_country_effect.h"
#include "script/effect/random_list_effect.h"
#include "script/effect/random_neighbor_country_effect.h"
#include "script/effect/random_settlement_effect.h"
#include "script/effect/save_building_as_effect.h"
#include "script/effect/save_scope_as_effect.h"
#include "script/effect/save_string_as_effect.h"
#include "script/effect/saved_scope_effect.h"
#include "script/effect/scripted_effect_effect.h"
#include "script/effect/scripted_modifiers_effect.h"
#include "script/effect/set_flag_effect.h"
#include "script/effect/skill_check_effect.h"
#include "script/effect/source_site_effect.h"
#include "script/effect/tooltip_effect.h"
#include "util/assert_util.h"

namespace kobold {

template <typename scope_type>
std::unique_ptr<effect<scope_type>> effect<scope_type>::from_gsml_property(const gsml_property &property)
{
	const std::string &key = property.get_key();
	const gsml_operator effect_operator = property.get_operator();
	const std::string &value = property.get_value();

	if constexpr (std::is_same_v<scope_type, const country>) {
		static const std::string percent_suffix = "_percent";

		if (key == "create_military_unit") {
			return std::make_unique<create_military_unit_effect>(value, effect_operator);
		} else if (key == "gain_spell_scroll") {
			return std::make_unique<gain_spell_scroll_effect>(value, effect_operator);
		} else if (key.ends_with(percent_suffix) && commodity::try_get(key.substr(0, key.size() - percent_suffix.size())) != nullptr) {
			const commodity *commodity = commodity::get(key.substr(0, key.size() - percent_suffix.size()));
			return std::make_unique<commodity_percent_effect>(commodity, value, effect_operator);
		}
	} else if constexpr (std::is_same_v<scope_type, const site>) {
		if (key == "add_building_class") {
			return std::make_unique<add_building_class_effect>(value, effect_operator);
		} else if (key == "improvements") {
			return std::make_unique<improvements_effect>(value, effect_operator);
		}
	}

	if constexpr (std::is_same_v<scope_type, const character> || std::is_same_v<scope_type, const country>) {
		if (key == "experience") {
			return std::make_unique<experience_effect<scope_type>>(value, effect_operator);
		} else if (key == "gain_feat") {
			return std::make_unique<gain_feat_effect<scope_type>>(value, effect_operator);
		} else if (key == "gain_feat_of_type") {
			return std::make_unique<gain_feat_of_type_effect<scope_type>>(value, effect_operator);
		} else if (commodity::try_get(key) != nullptr) {
			return std::make_unique<commodity_effect<scope_type>>(commodity::get(key), value, effect_operator);
		}
	}

	if constexpr (std::is_same_v<scope_type, const character> || std::is_same_v<scope_type, const country> || std::is_same_v<scope_type, const province> || std::is_same_v<scope_type, const site>) {
		if (key == "scripted_modifiers") {
			assert_throw(effect_operator == gsml_operator::subtraction);
			return std::make_unique<scripted_modifiers_effect<scope_type>>(value, effect_operator);
		}
	}

	if constexpr (std::is_same_v<scope_type, const country> || std::is_same_v<scope_type, const site>) {
		if (key == "capital") {
			return std::make_unique<capital_effect<scope_type>>(value, effect_operator);
		} else if (key == "clear_flag") {
			return std::make_unique<clear_flag_effect<scope_type>>(value, effect_operator);
		} else if (key == "set_flag") {
			return std::make_unique<set_flag_effect<scope_type>>(value, effect_operator);
		}
	}

	if (key == "event") {
		return std::make_unique<event_effect<scope_type>>(value, effect_operator);
	} else if (key == "event_trigger") {
		return std::make_unique<event_trigger_effect<scope_type>>(value, effect_operator);
	} else if (key == "save_scope_as") {
		return std::make_unique<save_scope_as_effect<scope_type>>(value, effect_operator);
	} else if (key == "scripted_effect") {
		return std::make_unique<scripted_effect_effect<scope_type>>(value, effect_operator);
	} else if (key == "tooltip") {
		return std::make_unique<tooltip_effect<scope_type>>(value, effect_operator);
	}

	throw std::runtime_error(std::format("Invalid property effect: \"{}\".", key));
}

template <typename scope_type>
std::unique_ptr<effect<scope_type>> effect<scope_type>::from_gsml_scope(const gsml_data &scope, const effect *previous_effect)
{
	const std::string &effect_identifier = scope.get_tag();
	const gsml_operator effect_operator = scope.get_operator();
	std::unique_ptr<effect> effect;

	if constexpr (std::is_same_v<scope_type, const character>) {
		if (effect_identifier == "combat") {
			effect = std::make_unique<combat_effect>(effect_operator);
		}
	} else if constexpr (std::is_same_v<scope_type, const country>) {
		if (effect_identifier == "any_known_country") {
			effect = std::make_unique<any_known_country_effect>(effect_operator);
		} else if (effect_identifier == "any_neighbor_country") {
			effect = std::make_unique<any_neighbor_country_effect>(effect_operator);
		} else if (effect_identifier == "battle") {
			effect = std::make_unique<battle_effect<scope_type>>(effect_operator);
		} else if (effect_identifier == "change_opinion") {
			effect = std::make_unique<change_opinion_effect<scope_type>>(effect_operator);
		} else if (effect_identifier == "create_military_unit") {
			effect = std::make_unique<create_military_unit_effect>(effect_operator);
		} else if (effect_identifier == "opinion_modifiers") {
			effect = std::make_unique<opinion_modifiers_effect<scope_type>>(effect_operator);
		} else if (effect_identifier == "random_known_country") {
			effect = std::make_unique<random_known_country_effect>(effect_operator);
		} else if (effect_identifier == "random_neighbor_country") {
			effect = std::make_unique<random_neighbor_country_effect>(effect_operator);
		} else if (effect_identifier == "random_settlement") {
			effect = std::make_unique<random_settlement_effect>(effect_operator);
		} else if (office::try_get(effect_identifier) != nullptr) {
			effect = std::make_unique<office_holder_effect>(office::get(effect_identifier), effect_operator);
		}
	} else if constexpr (std::is_same_v<scope_type, const site>) {
		if (effect_identifier == "location") {
			effect = std::make_unique<location_effect<scope_type>>(effect_operator);
		}
	}

	if constexpr (std::is_same_v<scope_type, const character> || std::is_same_v<scope_type, const country>) {
		if (effect_identifier == "delayed") {
			effect = std::make_unique<delayed_effect<scope_type>>(effect_operator);
		} else if (effect_identifier == "skill_check") {
			effect = std::make_unique<skill_check_effect<scope_type>>(effect_operator);
		} else if (commodity::try_get(effect_identifier) != nullptr) {
			effect = std::make_unique<commodity_effect<scope_type>>(commodity::get(effect_identifier), effect_operator);
		}
	}

	if constexpr (std::is_same_v<scope_type, const character> || std::is_same_v<scope_type, const country> || std::is_same_v<scope_type, const province> || std::is_same_v<scope_type, const site>) {
		if (effect_identifier == "scripted_modifiers") {
			effect = std::make_unique<scripted_modifiers_effect<scope_type>>(effect_operator);
		}
	}

	if constexpr (std::is_same_v<scope_type, const province> || std::is_same_v<scope_type, const site>) {
		if (effect_identifier == "provincial_capital") {
			effect = std::make_unique<provincial_capital_scope_effect<scope_type>>(effect_operator);
		}
	}

	if (effect_identifier == "character") {
		effect = std::make_unique<character_scope_effect<scope_type>>(effect_operator);
	} else if (effect_identifier == "create_character") {
		effect = std::make_unique<create_character_effect<scope_type>>(effect_operator);
	} else if (effect_identifier == "country") {
		effect = std::make_unique<country_effect<scope_type>>(effect_operator);
	} else if (effect_identifier == "hidden") {
		effect = std::make_unique<hidden_effect<scope_type>>(effect_operator);
	} else if (effect_identifier == "if") {
		effect = std::make_unique<if_effect<scope_type>>(effect_operator);
	} else if (effect_identifier == "else") {
		effect = std::make_unique<else_effect<scope_type>>(effect_operator, previous_effect);
	} else if (effect_identifier == "random") {
		effect = std::make_unique<random_effect<scope_type>>(effect_operator);
	} else if (effect_identifier == "random_list") {
		effect = std::make_unique<random_list_effect<scope_type>>(effect_operator);
	} else if (effect_identifier == "save_building_as") {
		effect = std::make_unique<save_building_as_effect<scope_type>>(effect_operator);
	} else if (effect_identifier == "save_string_as") {
		effect = std::make_unique<save_string_as_effect<scope_type>>(effect_operator);
	} else if (effect_identifier == "saved_character_scope") {
		effect = std::make_unique<saved_scope_effect<scope_type, const character>>(effect_operator);
	} else if (effect_identifier == "saved_country_scope") {
		effect = std::make_unique<saved_scope_effect<scope_type, const country>>(effect_operator);
	} else if (effect_identifier == "saved_province_scope") {
		effect = std::make_unique<saved_scope_effect<scope_type, const province>>(effect_operator);
	} else if (effect_identifier == "saved_site_scope") {
		effect = std::make_unique<saved_scope_effect<scope_type, const site>>(effect_operator);
	} else if (effect_identifier == "source_site") {
		effect = std::make_unique<source_site_effect<scope_type>>(effect_operator);
	} else if (effect_identifier == "tooltip") {
		effect = std::make_unique<tooltip_effect<scope_type>>(effect_operator);
	}

	if (effect == nullptr) {
		throw std::runtime_error(std::format("Invalid scope effect: \"{}\".", effect_identifier));
	}

	database::process_gsml_data(effect, scope);

	return effect;
}

template <typename scope_type>
const country *effect<scope_type>::get_scope_country(const scope_type *scope)
{
	if constexpr (std::is_same_v<scope_type, const character>) {
		return scope->get_game_data()->get_country();
	} else if constexpr (std::is_same_v<scope_type, const country>) {
		return scope;
	} else if constexpr (std::is_same_v<scope_type, const province> || std::is_same_v<scope_type, const site>) {
		return scope->get_game_data()->get_owner();
	}
}

template <typename scope_type>
const province *effect<scope_type>::get_scope_province(const scope_type *scope)
{
	if constexpr (std::is_same_v<scope_type, const province>) {
		return scope;
	} else if constexpr (std::is_same_v<scope_type, const site>) {
		return scope->get_game_data()->get_province();
	} else {
		assert_throw(false);
		return nullptr;
	}
}

template class effect<const character>;
template class effect<const country>;
template class effect<const province>;
template class effect<const site>;

}
