#pragma once

#include "util/enum_converter.h"

namespace kobold {

enum class transporter_category {
	none,
	land_transporter,
	small_merchant_ship,
	large_merchant_ship
};

}

extern template class archimedes::enum_converter<kobold::transporter_category>;

Q_DECLARE_METATYPE(kobold::transporter_category)
