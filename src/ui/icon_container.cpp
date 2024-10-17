#include "kobold.h"

#include "ui/icon_container.h"

#include "ui/icon.h"

namespace kobold {

bool icon_compare::operator()(const icon *lhs, const icon *rhs) const
{
	return lhs->get_identifier() < rhs->get_identifier();
}

}
