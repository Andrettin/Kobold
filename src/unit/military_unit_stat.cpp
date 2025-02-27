#include "kobold.h"

#include "unit/military_unit_stat.h"

namespace archimedes {

template class enum_converter<kobold::military_unit_stat>;

template <>
const std::string enum_converter<kobold::military_unit_stat>::property_class_identifier = "kobold::military_unit_stat";

template <>
const std::map<std::string, kobold::military_unit_stat> enum_converter<kobold::military_unit_stat>::string_to_enum_map = {
	{ "firepower", kobold::military_unit_stat::firepower },
	{ "melee", kobold::military_unit_stat::melee },
	{ "shock", kobold::military_unit_stat::shock },
	{ "range", kobold::military_unit_stat::range },
	{ "defense", kobold::military_unit_stat::defense },
	{ "resistance", kobold::military_unit_stat::resistance },
	{ "discipline", kobold::military_unit_stat::discipline },
	{ "movement", kobold::military_unit_stat::movement },
	{ "damage_bonus", kobold::military_unit_stat::damage_bonus },
	{ "bonus_vs_infantry", kobold::military_unit_stat::bonus_vs_infantry },
	{ "bonus_vs_cavalry", kobold::military_unit_stat::bonus_vs_cavalry },
	{ "bonus_vs_artillery", kobold::military_unit_stat::bonus_vs_artillery },
	{ "bonus_vs_fortifications", kobold::military_unit_stat::bonus_vs_fortifications },
	{ "defense_modifier", kobold::military_unit_stat::defense_modifier },
	{ "ranged_defense_modifier", kobold::military_unit_stat::ranged_defense_modifier },
	{ "entrenchment_bonus_modifier", kobold::military_unit_stat::entrenchment_bonus_modifier },
	{ "movement_modifier", kobold::military_unit_stat::movement_modifier },
	{ "recovery_modifier", kobold::military_unit_stat::recovery_modifier },
	{ "morale_recovery_modifier", kobold::military_unit_stat::morale_recovery_modifier },
	{ "desert_attack_modifier", kobold::military_unit_stat::desert_attack_modifier },
	{ "desert_defense_modifier", kobold::military_unit_stat::desert_defense_modifier },
	{ "forest_attack_modifier", kobold::military_unit_stat::forest_attack_modifier },
	{ "forest_defense_modifier", kobold::military_unit_stat::forest_defense_modifier },
	{ "hills_attack_modifier", kobold::military_unit_stat::hills_attack_modifier },
	{ "hills_defense_modifier", kobold::military_unit_stat::hills_defense_modifier },
	{ "mountains_attack_modifier", kobold::military_unit_stat::mountains_attack_modifier },
	{ "mountains_defense_modifier", kobold::military_unit_stat::mountains_defense_modifier },
	{ "wetland_attack_modifier", kobold::military_unit_stat::wetland_attack_modifier },
	{ "wetland_defense_modifier", kobold::military_unit_stat::wetland_defense_modifier }
};

template <>
const bool enum_converter<kobold::military_unit_stat>::initialized = enum_converter::initialize();

}
