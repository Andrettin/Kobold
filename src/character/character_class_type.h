#pragma once

namespace kobold {

enum class character_class_type {
	none,
	racial_class,
	base_class,
	prestige_class,
	epic_prestige_class
};

}

Q_DECLARE_METATYPE(kobold::character_class_type)
