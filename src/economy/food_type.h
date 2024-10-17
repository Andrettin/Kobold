#pragma once

#include "util/enum_converter.h"

namespace kobold {

enum class food_type {
	none,
	starch,
	meat,
	fruit
};

}

extern template class archimedes::enum_converter<kobold::food_type>;

Q_DECLARE_METATYPE(kobold::food_type)
