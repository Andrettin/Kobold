#include "kobold.h"

#include "character/character_container.h"

#include "character/character.h"

namespace kobold {

bool character_compare::operator()(const character *lhs, const character *rhs) const
{
	return lhs->get_identifier() < rhs->get_identifier();
}

}
