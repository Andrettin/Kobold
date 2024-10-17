#include "kobold.h"

#include "map/province_container.h"

#include "map/province.h"

namespace kobold {

bool province_compare::operator()(const province *province, const kobold::province *other_province) const
{
	return province->get_identifier() < other_province->get_identifier();
}

}
