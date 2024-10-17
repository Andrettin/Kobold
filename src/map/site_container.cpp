#include "kobold.h"

#include "map/site_container.h"

#include "map/site.h"

namespace kobold {

bool site_compare::operator()(const site *site, const kobold::site *other_site) const
{
	return site->get_identifier() < other_site->get_identifier();
}

}
