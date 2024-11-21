#pragma once

#include "util/enum_converter.h"

namespace kobold {

enum class affix_type {
	none,
	prefix,
	suffix,
	stem
};

}

extern template class archimedes::enum_converter<kobold::affix_type>;

Q_DECLARE_METATYPE(kobold::affix_type)
