#pragma once

#include "util/enum_converter.h"

namespace kobold {

enum class feat_template_type {
	none,
	skill,
	country_skill
};

}

extern template class archimedes::enum_converter<kobold::feat_template_type>;

Q_DECLARE_METATYPE(kobold::feat_template_type)
