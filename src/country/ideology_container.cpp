#include "kobold.h"

#include "country/ideology_container.h"

#include "country/ideology.h"

namespace kobold {

bool ideology_compare::operator()(const ideology *lhs, const ideology *rhs) const
{
	return lhs->get_identifier() < rhs->get_identifier();
}

}
