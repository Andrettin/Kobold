#include "kobold.h"

#include "unit/promotion_container.h"

#include "unit/promotion.h"

namespace kobold {

bool promotion_compare::operator()(const promotion *lhs, const promotion *rhs) const
{
	if ((lhs == nullptr || rhs == nullptr) && lhs != rhs) {
		return lhs == nullptr;
	}

	return lhs->get_identifier() < rhs->get_identifier();
}

}
