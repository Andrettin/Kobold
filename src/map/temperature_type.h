#pragma once

#include "util/enum_converter.h"

namespace kobold {

enum class temperature_type {
	none,
	frozen,
	cold,
	temperate,
	tropical
};

}

extern template class archimedes::enum_converter<kobold::temperature_type>;

Q_DECLARE_METATYPE(kobold::temperature_type)
