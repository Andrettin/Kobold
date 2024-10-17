#include "kobold.h"

#include "country/policy_container.h"

#include "country/policy.h"

namespace kobold {

bool policy_compare::operator()(const policy *lhs, const policy *rhs) const
{
	return lhs->get_identifier() < rhs->get_identifier();
}

}
