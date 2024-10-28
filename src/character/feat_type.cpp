#include "kobold.h"

#include "character/feat_type.h"

namespace kobold {

void feat_type::check() const
{
	if (this->get_feats().empty()) {
		throw std::runtime_error(std::format("Feat type \"{}\" has no feats that belong to it.", this->get_identifier()));
	}
}

}
