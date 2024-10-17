#include "kobold.h"

#include "script/opinion_modifier_container.h"

#include "script/opinion_modifier.h"

namespace kobold {

bool opinion_modifier_compare::operator()(const opinion_modifier *lhs, const opinion_modifier *rhs) const
{
	return lhs->get_identifier() < rhs->get_identifier();
}

}
