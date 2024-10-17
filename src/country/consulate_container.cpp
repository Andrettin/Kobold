#include "kobold.h"

#include "country/consulate_container.h"

#include "country/consulate.h"

namespace kobold {

bool consulate_compare::operator()(const consulate *lhs, const consulate *rhs) const
{
	if (lhs->get_level() != rhs->get_level()) {
		return lhs->get_level() < rhs->get_level();
	}

	return lhs->get_identifier() < rhs->get_identifier();
}

}
