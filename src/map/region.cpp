#include "kobold.h"

#include "map/region.h"

#include "map/province.h"
#include "map/site.h"
#include "util/vector_util.h"

namespace kobold {

region::region(const std::string &identifier) : named_data_entry(identifier)
{
}

region::~region()
{
}

void region::initialize()
{
	for (region *subregion : this->subregions) {
		//initialize subregions, so that the sites of their own subregions are added to them
		if (!subregion->is_initialized()) {
			subregion->initialize();
		}

		//add provinces from subregions
		for (province *province : subregion->get_provinces()) {
			if (vector::contains(this->provinces, province)) {
				continue;
			}

			this->provinces.push_back(province);
			province->add_region(this);
		}
	}

	named_data_entry::initialize();
}

void region::add_province(province *province)
{
	if (vector::contains(this->provinces, province)) {
		return;
	}

	this->provinces.push_back(province);
}

bool region::is_part_of(const region *other_region) const
{
	if (vector::contains(this->superregions, other_region)) {
		return true;
	}

	for (const region *superregion : this->superregions) {
		if (superregion->is_part_of(other_region)) {
			return true;
		}
	}

	return false;
}

}
