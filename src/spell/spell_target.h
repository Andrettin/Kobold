#pragma once

#include "util/enum_converter.h"

namespace kobold {

enum class spell_target {
	none,
	enemy,
	ally
};

}

extern template class archimedes::enum_converter<kobold::spell_target>;

Q_DECLARE_METATYPE(kobold::spell_target)
