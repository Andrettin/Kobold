#pragma once

#include "util/enum_converter.h"

namespace kobold {

enum class country_tier {
	none,
	barony,
	county,
	duchy,
	kingdom,
	empire
};

}

extern template class archimedes::enum_converter<kobold::country_tier>;

Q_DECLARE_METATYPE(kobold::country_tier)
