#pragma once

#include "util/enum_converter.h"

namespace kobold {

enum class forestation_type {
	none,
	forest
};

}

extern template class archimedes::enum_converter<kobold::forestation_type>;

Q_DECLARE_METATYPE(kobold::forestation_type)
