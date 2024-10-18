#include "kobold.h"

#include "infrastructure/pathway.h"

#include "database/defines.h"
#include "economy/commodity.h"
#include "map/terrain_type.h"
#include "map/tile.h"
#include "map/tile_image_provider.h"
#include "util/assert_util.h"
#include "util/vector_util.h"

namespace kobold {
	
void pathway::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag == "commodity_costs") {
		scope.for_each_property([&](const gsml_property &property) {
			const commodity *commodity = commodity::get(property.get_key());
			this->commodity_costs[commodity] = std::stoi(property.get_value());
		});
	} else {
		data_entry::process_gsml_scope(scope);
	}
}

void pathway::initialize()
{
	tile_image_provider::get()->load_image("pathway/" + this->get_identifier() + "/0");

	named_data_entry::initialize();
}

void pathway::check() const
{
	assert_throw(this->get_transport_level() > 0 || this == defines::get()->get_route_pathway());

	assert_log(!this->get_image_filepath().empty());
}

void pathway::set_image_filepath(const std::filesystem::path &filepath)
{
	if (filepath == this->get_image_filepath()) {
		return;
	}

	this->image_filepath = database::get()->get_graphics_path(this->get_module()) / filepath;
}

bool pathway::is_buildable_on_tile(const tile *tile, const direction direction) const
{
	const pathway *tile_pathway = tile->get_direction_pathway(direction);

	if (this->get_required_pathway() != nullptr && tile_pathway != this->get_required_pathway()) {
		return false;
	}

	if (tile_pathway != nullptr) {
		if (this == tile_pathway) {
			return false;
		}

		if (this->get_transport_level() < tile_pathway->get_transport_level()) {
			return false;
		}

		if (this->get_transport_level() == tile_pathway->get_transport_level()) {
			//the pathway must be better in some way
			return false;
		}
	}

	return true;
}

}
