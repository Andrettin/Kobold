#pragma once

#include "util/enum_converter.h"

namespace kobold {

enum class site_type {
	none,
	settlement,
	terrain,
	resource
};

}

extern template class archimedes::enum_converter<kobold::site_type>;

Q_DECLARE_METATYPE(kobold::site_type)
