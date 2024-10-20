#pragma once

#include "util/enum_converter.h"

namespace kobold {

enum class moisture_type {
	none,
	arid,
	semi_arid,
	dry,
	moist,
	wet
};

}

extern template class archimedes::enum_converter<kobold::moisture_type>;

Q_DECLARE_METATYPE(kobold::moisture_type)
