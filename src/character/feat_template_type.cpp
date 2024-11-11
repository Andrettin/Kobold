#include "kobold.h"

#include "character/feat_template_type.h"

namespace archimedes {

template class enum_converter<kobold::feat_template_type>;

template <>
const std::string enum_converter<kobold::feat_template_type>::property_class_identifier = "kobold::feat_template_type";

template <>
const std::map<std::string, kobold::feat_template_type> enum_converter<kobold::feat_template_type>::string_to_enum_map = {
	{ "none", kobold::feat_template_type::none },
	{ "skill", kobold::feat_template_type::skill },
	{ "country_skill", kobold::feat_template_type::country_skill }
};

template <>
const bool enum_converter<kobold::feat_template_type>::initialized = enum_converter::initialize();

}
