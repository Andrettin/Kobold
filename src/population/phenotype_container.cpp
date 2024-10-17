#include "kobold.h"

#include "population/phenotype_container.h"

#include "population/phenotype.h"

namespace kobold {

bool phenotype_compare::operator()(const phenotype *lhs, const phenotype *rhs) const
{
	return lhs->get_identifier() < rhs->get_identifier();
}

}
