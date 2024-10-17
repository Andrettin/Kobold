#pragma once

#include "util/enum_converter.h"

namespace kobold {

enum class cultural_group_rank {
	none,
	infragroup,
	subgroup,
	group,
	supergroup
};

}

extern template class archimedes::enum_converter<kobold::cultural_group_rank>;

Q_DECLARE_METATYPE(kobold::cultural_group_rank)
