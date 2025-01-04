#include "kobold.h"

#include "species/phenotype_container.h"

#include "species/phenotype.h"

namespace kobold {

bool phenotype_compare::operator()(const phenotype *lhs, const phenotype *rhs) const
{
	return lhs->get_identifier() < rhs->get_identifier();
}

}
