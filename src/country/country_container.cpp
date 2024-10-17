#include "kobold.h"

#include "country/country_container.h"

#include "country/country.h"

namespace kobold {

bool country_compare::operator()(const country *country, const kobold::country *other_country) const
{
	return country->get_identifier() < other_country->get_identifier();
}

}
