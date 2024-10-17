#pragma once

#include "util/enum_converter.h"

namespace kobold {

enum class improvement_slot {
	none,
	main,
	resource,
	depot,
	port
};

}

extern template class archimedes::enum_converter<kobold::improvement_slot>;

Q_DECLARE_METATYPE(kobold::improvement_slot)
