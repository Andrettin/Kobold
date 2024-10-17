#include "kobold.h"

#include "unit/military_unit_category.h"

namespace archimedes {

template class enum_converter<kobold::military_unit_category>;

template <>
const std::string enum_converter<kobold::military_unit_category>::property_class_identifier = "kobold::military_unit_category";

template <>
const std::map<std::string, kobold::military_unit_category> enum_converter<kobold::military_unit_category>::string_to_enum_map = {
	{ "beasts", kobold::military_unit_category::beasts },
	{ "colossal_beasts", kobold::military_unit_category::colossal_beasts },
	{ "sea_beasts", kobold::military_unit_category::sea_beasts },
	{ "colossal_sea_beasts", kobold::military_unit_category::colossal_sea_beasts },
	{ "flying_beasts", kobold::military_unit_category::flying_beasts },
	{ "colossal_flying_beasts", kobold::military_unit_category::colossal_flying_beasts },
	{ "militia", kobold::military_unit_category::militia },
	{ "mace_infantry", kobold::military_unit_category::mace_infantry },
	{ "spear_infantry", kobold::military_unit_category::spear_infantry },
	{ "blade_infantry", kobold::military_unit_category::blade_infantry },
	{ "light_infantry", kobold::military_unit_category::light_infantry },
	{ "regular_infantry", kobold::military_unit_category::regular_infantry },
	{ "heavy_infantry", kobold::military_unit_category::heavy_infantry },
	{ "bowmen", kobold::military_unit_category::bowmen },
	{ "light_cavalry", kobold::military_unit_category::light_cavalry },
	{ "heavy_cavalry", kobold::military_unit_category::heavy_cavalry },
	{ "spear_cavalry", kobold::military_unit_category::spear_cavalry },
	{ "light_artillery", kobold::military_unit_category::light_artillery },
	{ "heavy_artillery", kobold::military_unit_category::heavy_artillery },
	{ "combat_engineers", kobold::military_unit_category::combat_engineers },
	{ "general", kobold::military_unit_category::general },
	{ "alchemist", kobold::military_unit_category::alchemist },
	{ "bard", kobold::military_unit_category::bard },
	{ "cleric", kobold::military_unit_category::cleric },
	{ "mage", kobold::military_unit_category::mage },
	{ "paladin", kobold::military_unit_category::paladin },
	{ "ranger", kobold::military_unit_category::ranger },
	{ "rogue", kobold::military_unit_category::rogue },
	{ "warrior", kobold::military_unit_category::warrior },
	{ "light_warship", kobold::military_unit_category::light_warship },
	{ "heavy_warship", kobold::military_unit_category::heavy_warship }
};

template <>
const bool enum_converter<kobold::military_unit_category>::initialized = enum_converter::initialize();

}
