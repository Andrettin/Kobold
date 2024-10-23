#pragma once

#include "util/enum_converter.h"

namespace kobold {

enum class character_class_type {
	none,
	racial_class,
	base_class,
	prestige_class,
	epic_prestige_class
};

}

extern template class archimedes::enum_converter<kobold::character_class_type>;

Q_DECLARE_METATYPE(kobold::character_class_type)
