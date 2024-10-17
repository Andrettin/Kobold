#include "kobold.h"

#include "country/tradition_container.h"

#include "country/tradition.h"

namespace kobold {

bool tradition_compare::operator()(const tradition *lhs, const tradition *rhs) const
{
	return lhs->get_identifier() < rhs->get_identifier();
}

}
