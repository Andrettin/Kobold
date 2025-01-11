#pragma once

namespace kobold {

enum class site_type {
	none,
	settlement,
	terrain,
	resource,
	habitable_world,
	celestial_body
};

}

Q_DECLARE_METATYPE(kobold::site_type)
