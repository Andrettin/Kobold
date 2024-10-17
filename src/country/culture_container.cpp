#include "kobold.h"

#include "country/culture_container.h"

#include "country/culture.h"

namespace kobold {

bool culture_compare::operator()(const culture *lhs, const culture *rhs) const
{
	return lhs->get_identifier() < rhs->get_identifier();
}

}
