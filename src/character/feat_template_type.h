#pragma once

namespace kobold {

enum class feat_template_type {
	none,
	skill,
	weapon_type,
	country_skill
};

}

Q_DECLARE_METATYPE(kobold::feat_template_type)
