#pragma once

#include "util/enum_converter.h"

namespace kobold {

enum class special_target_type {
	root,
	source,
	previous
};

}

extern template class archimedes::enum_converter<kobold::special_target_type>;

Q_DECLARE_METATYPE(kobold::special_target_type)
