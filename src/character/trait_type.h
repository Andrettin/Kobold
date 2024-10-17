#pragma once

#include "util/enum_converter.h"

namespace kobold {

enum class trait_type {
	none,
	ruler,
	background,
	personality,
	expertise
};

}

extern template class archimedes::enum_converter<kobold::trait_type>;

Q_DECLARE_METATYPE(kobold::trait_type)
