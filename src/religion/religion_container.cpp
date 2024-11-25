#include "kobold.h"

#include "religion/religion_container.h"

#include "religion/religion.h"

namespace kobold {

bool religion_compare::operator()(const religion *lhs, const religion *rhs) const
{
	return lhs->get_identifier() < rhs->get_identifier();
}

}
