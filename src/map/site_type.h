#pragma once

namespace kobold {

enum class site_type {
	none,
	settlement,
	terrain,
	resource
};

}

Q_DECLARE_METATYPE(kobold::site_type)
