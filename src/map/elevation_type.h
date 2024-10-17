#pragma once

#include "util/enum_converter.h"

namespace kobold {

enum class elevation_type {
	none,
	water,
	flatlands,
	hills,
	mountains
};

}

extern template class archimedes::enum_converter<kobold::elevation_type>;

Q_DECLARE_METATYPE(kobold::elevation_type)
