#include "kobold.h"

#include "map/map_generator.h"

#include "country/country.h"
#include "country/country_container.h"
#include "country/country_game_data.h"
#include "country/culture.h"
#include "database/defines.h"
#include "economy/resource.h"
#include "map/elevation_type.h"
#include "map/forestation_type.h"
#include "map/map.h"
#include "map/map_template.h"
#include "map/moisture_type.h"
#include "map/province.h"
#include "map/province_game_data.h"
#include "map/province_history.h"
#include "map/region.h"
#include "map/site.h"
#include "map/site_game_data.h"
#include "map/site_map_data.h"
#include "map/site_type.h"
#include "map/temperature_type.h"
#include "map/terrain_type.h"
#include "map/tile.h"
#include "util/assert_util.h"
#include "util/container_util.h"
#include "util/log_util.h"
#include "util/number_util.h"
#include "util/point_util.h"
#include "util/rect_util.h"
#include "util/vector_util.h"
#include "util/vector_random_util.h"

namespace kobold {

void map_generator::adjust_values(std::vector<int> &values, const int target_max_value)
{
	int min_value = 0;
	int max_value = 0;

	bool first = true;
	for (const int value : values) {
		if (first) {
			min_value = value;
			max_value = value;
			first = false;
		} else {
			if (value < min_value) {
				min_value = value;
			}

			if (value > max_value) {
				max_value = value;
			}
		}
	}

	const int value_range_size = max_value - min_value + 1;
	assert_throw(value_range_size > 0);

	std::vector<int> value_counts;
	try {
		value_counts.resize(value_range_size, 0);
	} catch (...) {
		std::throw_with_nested(std::runtime_error(std::format("Failed to resize value counts vector to size {} when adjusting values for the map generator.", value_range_size)));
	}

	for (int &value : values) {
		value -= min_value;
		++value_counts[value];
	}

	int count = 0;
	for (int i = 0; i < value_range_size; ++i) {
		count += value_counts[i];
		value_counts[i] = count * target_max_value / static_cast<int>(values.size());
	}

	for (int &value : values) {
		value = value_counts[value];
	}
}

const QSize &map_generator::get_size() const
{
	return this->get_map_template()->get_size();
}

void map_generator::generate()
{
	const int tile_count = this->get_width() * this->get_height();
	this->tile_zones.resize(tile_count, -1);
	this->tile_moistures.resize(tile_count, -1);
	this->tile_forestations.resize(tile_count, -1);

	this->initialize_temperature_levels();

	this->generate_zones();
	this->generate_terrain();
	this->generate_countries();

	map *map = map::get();

	//assign provinces
	for (size_t i = 0; i < this->tile_zones.size(); ++i) {
		const int zone_index = this->tile_zones[i];
		assert_throw(zone_index >= 0);

		const auto find_iterator = this->provinces_by_zone_index.find(zone_index);

		if (find_iterator == this->provinces_by_zone_index.end()) {
			continue;
		}

		const province *province = find_iterator->second;
		assert_throw(province != nullptr);
		map->set_tile_province(point::from_index(static_cast<int>(i), this->get_width()), province);
	}

	this->generate_sites();
}

void map_generator::initialize_temperature_levels()
{
	const int sqrt_size = this->get_sqrt_size();

	const int separate_poles_factor = this->get_map_template()->are_poles_separate() ? 1 : 2;

	this->ice_base_level = (std::max(0, 100 * this->get_cold_level() / 3 - separate_poles_factor * map_generator::max_colatitude) + separate_poles_factor * map_generator::max_colatitude * sqrt_size) / (100 * sqrt_size);
}

void map_generator::generate_terrain()
{
	this->generate_elevation();

	//ensure edge zones are water
	const QRect map_rect(QPoint(0, 0), this->get_size());
	rect::for_each_edge_point(map_rect, [&](const QPoint &tile_pos) {
		const int tile_index = point::to_index(tile_pos, this->get_width());
		const int zone_index = this->tile_zones[tile_index];
		const QPoint &zone_seed = this->zones.at(zone_index).seed;
		const int zone_seed_index = point::to_index(zone_seed, this->get_width());
		this->tile_elevations[tile_index] = 0;
		this->tile_elevations[zone_seed_index] = 0;
		this->sea_zones.insert(zone_index);
	});

	this->generate_moisture();
	this->generate_forestation();

	//assign terrain
	map *map = map::get();

	for (int x = 0; x < map->get_width(); ++x) {
		for (int y = 0; y < map->get_height(); ++y) {
			const QPoint tile_pos(x, y);
			const int tile_index = point::to_index(tile_pos, this->get_width());

			int &elevation = this->tile_elevations[tile_index];
			const bool is_water = this->is_tile_water(tile_pos);

			const int zone_index = this->tile_zones[tile_index];
			const zone &zone = this->zones[zone_index];
			const bool zone_is_water = this->is_tile_water(zone.seed);

			if (zone_is_water && !is_water) {
				elevation = 0;
			} else if (!zone_is_water && is_water) {
				elevation = this->get_min_land_elevation();
			}

			const elevation_type elevation_type = this->get_tile_elevation_type(tile_pos);
			const temperature_type temperature_type = this->get_tile_temperature_type(tile_pos);
			const moisture_type moisture_type = this->get_tile_moisture_type(tile_pos);
			const forestation_type forestation_type = this->get_tile_forestation_type(tile_pos);

			const terrain_type *terrain = terrain_type::get_by_biome(elevation_type, temperature_type, moisture_type, forestation_type);

			map->set_tile_terrain(tile_pos, terrain);
		}
	}

	//build the zone tiles by terrain map
	for (size_t i = 0; i < this->zone_tiles.size(); ++i) {
		this->zone_tiles_by_terrain.emplace_back();
		this->zone_near_water_tiles_by_terrain.emplace_back();

		for (const QPoint &tile_pos : this->zone_tiles.at(i)) {
			const terrain_type *terrain = map->get_tile(tile_pos)->get_terrain();
			this->zone_tiles_by_terrain[i][terrain].push_back(tile_pos);

			if (map->is_tile_near_water(tile_pos)) {
				this->zone_near_water_tiles_by_terrain[i][terrain].push_back(tile_pos);
			}
		}
	}

	//designate which water zones are seas and which are lakes
	//lakes are water zones which have no connection to the seas on the edge of the map

	std::vector<int> sea_zones = container::to_vector(this->sea_zones);

	for (size_t i = 0; i < sea_zones.size(); ++i) {
		const int zone_index = sea_zones.at(i);

		for (const int border_zone_index : this->zone_border_zones[zone_index]) {
			const zone &border_zone = this->zones.at(border_zone_index);
			const QPoint &border_zone_seed = border_zone.seed;

			if (!this->is_tile_water(border_zone_seed)) {
				continue;
			}

			if (this->sea_zones.contains(border_zone_index)) {
				continue;
			}

			sea_zones.push_back(border_zone_index);
			this->sea_zones.insert(border_zone_index);
		}
	}

	for (int i = 0; i < this->zone_count; ++i) {
		const zone &zone = this->zones.at(i);
		const QPoint &zone_seed = zone.seed;

		if (!this->is_tile_water(zone_seed)) {
			continue;
		}

		if (this->sea_zones.contains(i)) {
			continue;
		}

		this->lakes.insert(i);
	}
}

void map_generator::generate_elevation()
{
	this->generate_pseudofractal_elevation(1);
}

void map_generator::generate_pseudofractal_elevation(const int additional_initial_block_count)
{
	const int initial_block_count_x = 5 + additional_initial_block_count;
	const int initial_block_count_y = 5 + additional_initial_block_count;

	const int modified_initial_block_count_x = initial_block_count_x + (this->is_x_wrap_enabled() ? 0 : 1);
	const int modified_initial_block_count_y = initial_block_count_y + (this->is_y_wrap_enabled() ? 0 : 1);

	const int max_x = this->get_width() - (this->is_x_wrap_enabled() ? 0 : 1);
	const int max_y = this->get_height() - (this->is_y_wrap_enabled() ? 0 : 1);

	const int step = this->get_width() + this->get_height();
	const int edge_avoidance_modifier = (100 - this->get_map_template()->get_land_percent()) * step / 100 + step / 3;

	this->tile_elevations.resize(this->get_tile_count(), 0);

	for (int block_x = 0; block_x < modified_initial_block_count_x; ++block_x) {
		for (int block_y = 0; block_y < modified_initial_block_count_y; ++block_y) {
			const int tile_x = (block_x * max_x / initial_block_count_x);
			const int tile_y = (block_y * max_y / initial_block_count_y);
			const QPoint tile_pos(tile_x, tile_y);
			const int tile_index = point::to_index(tile_pos, this->get_width());

			this->tile_elevations[tile_index] = random::get()->generate(2 * step) - (2 * step) / 2;

			if (this->is_tile_near_edge(tile_pos)) {
				this->tile_elevations[tile_index] -= edge_avoidance_modifier;
			}

			const int tile_colatitude = this->get_tile_colatitude(tile_pos);
			if (tile_colatitude <= (this->get_ice_base_level() / 2) && this->get_map_template()->get_pole_flattening() > 0) {
				this->tile_elevations[tile_index] -= random::get()->generate(edge_avoidance_modifier * this->get_map_template()->get_pole_flattening() / 100);
			}
		}
	}

	for (int block_x = 0; block_x < initial_block_count_x; ++block_x) {
		for (int block_y = 0; block_y < initial_block_count_y; ++block_y) {
			const QPoint top_left(block_x * max_x / initial_block_count_x, block_y * max_y / initial_block_count_y);
			const QPoint bottom_right((block_x + 1) * max_x / initial_block_count_x, (block_y + 1) * max_y / initial_block_count_y);
			const QRect tile_rect(top_left, bottom_right - QPoint(1, 1));
			this->generate_pseudofractal_tile_rect_elevation(step, tile_rect);
		}
	}

	//randomize a bit
	for (int &tile_elevation : this->tile_elevations) {
		tile_elevation = 8 * tile_elevation + random::get()->generate(4) - 2;
	}

	map_generator::adjust_values(this->tile_elevations, map_generator::max_elevation);
}

void map_generator::generate_pseudofractal_tile_rect_elevation(const int step, const QRect &tile_rect)
{
	std::array<std::array<int, 2>, 2> value_array{};

	const int top = tile_rect.top();
	const int left = tile_rect.left();
	const int bottom_end = tile_rect.bottom() + 1;
	const int right_end = tile_rect.right() + 1;

	int wrap_x = right_end;
	int wrap_y = bottom_end;

	if (tile_rect.width() <= 0 || tile_rect.height() <= 0) {
		return;
	}

	if (tile_rect.width() == 1 && tile_rect.height() == 1) {
		return;
	}

	if (right_end == this->get_width()) {
		wrap_x = 0;
	}

	if (bottom_end == this->get_height()) {
		wrap_y = 0;
	}

	value_array[0][0] = this->tile_elevations[point::to_index(tile_rect.topLeft(), this->get_width())];
	value_array[0][1] = this->tile_elevations[point::to_index(left, wrap_y, this->get_width())];
	value_array[1][0] = this->tile_elevations[point::to_index(wrap_x, top, this->get_width())];
	value_array[1][1] = this->tile_elevations[point::to_index(wrap_x, wrap_y, this->get_width())];

	this->set_pseudofractal_elevation_midpoints((left + right_end) / 2, top, (value_array[0][0] + value_array[1][0]) / 2 + random::get()->generate(step) - step / 2);
	this->set_pseudofractal_elevation_midpoints((left + right_end) / 2, wrap_y, (value_array[0][1] + value_array[1][1]) / 2 + random::get()->generate(step) - step / 2);
	this->set_pseudofractal_elevation_midpoints(left, (top + bottom_end) / 2, (value_array[0][0] + value_array[0][1]) / 2 + random::get()->generate(step) - step / 2);
	this->set_pseudofractal_elevation_midpoints(wrap_x, (top + bottom_end) / 2, (value_array[1][0] + value_array[1][1]) / 2 + random::get()->generate(step) - step / 2);

	this->set_pseudofractal_elevation_midpoints((left + right_end) / 2, (top + bottom_end) / 2, ((value_array[0][0] + value_array[0][1] + value_array[1][0] + value_array[1][1]) / 4 + random::get()->generate(step) - step / 2));

	this->generate_pseudofractal_tile_rect_elevation(2 * step / 3, QRect(QPoint(left, top), QPoint((right_end + left) / 2, (bottom_end + top) / 2) - QPoint(1, 1)));
	this->generate_pseudofractal_tile_rect_elevation(2 * step / 3, QRect(QPoint(left, (bottom_end + top) / 2), QPoint((right_end + left) / 2, bottom_end) - QPoint(1, 1)));
	this->generate_pseudofractal_tile_rect_elevation(2 * step / 3, QRect(QPoint((right_end + left) / 2, top), QPoint(right_end, (bottom_end + top) / 2) - QPoint(1, 1)));
	this->generate_pseudofractal_tile_rect_elevation(2 * step / 3, QRect(QPoint((right_end + left) / 2, (bottom_end + top) / 2), QPoint(right_end, bottom_end) - QPoint(1, 1)));
}

void map_generator::set_pseudofractal_elevation_midpoints(const int x, const int y, const int value)
{
	const QPoint tile_pos(x, y);
	const int tile_index = point::to_index(x, y, this->get_width());

	if (this->get_tile_colatitude(tile_pos) <= this->get_ice_base_level() / 2) {
		this->tile_elevations[tile_index] = value * (100 - this->get_map_template()->get_pole_flattening()) / 100;
	} else if (!this->is_tile_near_edge(tile_pos) && this->tile_elevations[tile_index] == 0) {
		this->tile_elevations[tile_index] = value;
	}
}

void map_generator::generate_moisture()
{
	const std::vector<QPoint> seeds = this->generate_tile_value_seeds(this->tile_moistures, 256);
	this->expand_tile_value_seeds(seeds, this->tile_moistures, 100);
}

void map_generator::generate_forestation()
{
	const std::vector<QPoint> seeds = this->generate_tile_value_seeds(this->tile_forestations, 256);
	this->expand_tile_value_seeds(seeds, this->tile_forestations, 50);
}

std::vector<QPoint> map_generator::generate_tile_value_seeds(std::vector<int> &tile_values, const int seed_divisor)
{
	const int map_area = this->get_width() * this->get_height();
	const int seed_count = map_area / seed_divisor;

	std::vector<QPoint> potential_positions;
	potential_positions.reserve(tile_values.size());

	for (int x = 0; x < this->get_width(); ++x) {
		for (int y = 0; y < this->get_height(); ++y) {
			potential_positions.emplace_back(x, y);
		}
	}

	std::vector<QPoint> seeds;
	seeds.reserve(seed_count);

	for (int i = 0; i < seed_count; ++i) {
		while (!potential_positions.empty()) {
			QPoint random_pos = vector::take_random(potential_positions);

			const int tile_index = point::to_index(random_pos, this->get_width());
			int &tile_value = tile_values[tile_index];
			if (tile_value != -1) {
				continue;
			}

			tile_value = map_generator::max_tile_value;

			seeds.push_back(std::move(random_pos));
			break;
		}
	}

	assert_throw(static_cast<int>(seeds.size()) == seed_count);

	return seeds;
}

void map_generator::expand_tile_value_seeds(const std::vector<QPoint> &base_seeds, std::vector<int> &tile_values, const int max_decrease)
{
	const map *map = map::get();

	std::vector<QPoint> seeds = vector::shuffled(base_seeds);

	while (!seeds.empty()) {
		std::vector<QPoint> new_seeds;

		for (const QPoint &seed_pos : seeds) {
			const int tile_value = tile_values[point::to_index(seed_pos, this->get_width())];

			point::for_each_cardinally_adjacent(seed_pos, [&](QPoint &&adjacent_pos) {
				if (!map->contains(adjacent_pos)) {
					return;
				}

				int &adjacent_tile_value = tile_values[point::to_index(adjacent_pos, this->get_width())];
				if (adjacent_tile_value != -1) {
					return;
				}

				adjacent_tile_value = std::max(0, tile_value - random::get()->generate_in_range(0, max_decrease));
				new_seeds.push_back(std::move(adjacent_pos));
			});
		}

		seeds = std::move(new_seeds);
		vector::shuffle(seeds);
	}
}

void map_generator::generate_zones()
{
	const int map_area = this->get_width() * this->get_height();

	this->zone_count = map_area / 64;

	std::vector<QPoint> zone_seeds = this->generate_zone_seeds(static_cast<size_t>(this->zone_count));
	this->expand_zone_seeds(zone_seeds);
}

std::vector<QPoint> map_generator::generate_zone_seeds(const size_t seed_count)
{
	static constexpr int min_zone_seed_distance = 6;

	std::vector<QPoint> potential_positions;
	potential_positions.reserve(this->tile_zones.size());

	for (int x = 0; x < this->get_width(); ++x) {
		for (int y = 0; y < this->get_height(); ++y) {
			potential_positions.emplace_back(x, y);
		}
	}

	std::vector<QPoint> zone_seeds;
	zone_seeds.reserve(seed_count);

	for (size_t i = 0; i < seed_count; ++i) {
		while (!potential_positions.empty()) {
			QPoint random_pos = vector::take_random(potential_positions);

			bool valid_location = true;
			for (const QPoint &seed : zone_seeds) {
				//when generating zone seeds, make them have a certain distance between each other
				const int distance = point::distance_to(random_pos, seed);

				if (distance < min_zone_seed_distance) {
					valid_location = false;
					break;
				}
			}

			if (valid_location) {
				zone_seeds.push_back(std::move(random_pos));
				break;
			}
		}
	}

	if (zone_seeds.size() != seed_count) {
		throw std::runtime_error("Could only generate " + std::to_string(zone_seeds.size()) + " zone seeds out of the required " + std::to_string(seed_count) + ".");
	}

	for (size_t i = 0; i < zone_seeds.size(); ++i) {
		const QPoint seed_pos = zone_seeds.at(i);
		const int tile_index = point::to_index(seed_pos, this->get_width());
		this->tile_zones[tile_index] = static_cast<int>(i);

		zone zone(seed_pos);
		this->zone_tiles.push_back({ seed_pos });
		this->zones.push_back(std::move(zone));
	}

	return zone_seeds;
}

void map_generator::expand_zone_seeds(const std::vector<QPoint> &base_seeds)
{
	const map *map = map::get();
	const QSize &map_size = this->get_size();

	std::vector<QPoint> seeds = vector::shuffled(base_seeds);

	while (!seeds.empty()) {
		std::vector<QPoint> new_seeds;

		for (const QPoint &seed_pos : seeds) {
			const int tile_index = point::to_index(seed_pos, map_size);
			const int zone_index = this->tile_zones[tile_index];

			std::vector<QPoint> adjacent_positions;

			point::for_each_cardinally_adjacent(seed_pos, [&](QPoint &&adjacent_pos) {
				if (!map->contains(adjacent_pos)) {
					return;
				}

				const int adjacent_tile_index = point::to_index(adjacent_pos, map_size);
				const int adjacent_zone_index = this->tile_zones[adjacent_tile_index];
				if (adjacent_zone_index != -1) {
					//the adjacent tile must not have a zone assigned yet
					this->zone_border_zones[zone_index].insert(adjacent_zone_index);
					this->zone_border_zones[adjacent_zone_index].insert(zone_index);
					return;
				}

				adjacent_positions.push_back(std::move(adjacent_pos));
			});

			if (adjacent_positions.empty()) {
				continue;
			}

			if (adjacent_positions.size() > 1) {
				//push the seed back again for another try, since it may be able to generate further in the future
				new_seeds.push_back(seed_pos);
			}

			QPoint adjacent_pos = vector::get_random(adjacent_positions);
			this->tile_zones[point::to_index(adjacent_pos, map_size)] = zone_index;
			this->zone_tiles[zone_index].push_back(adjacent_pos);

			new_seeds.push_back(std::move(adjacent_pos));
		}

		seeds = std::move(new_seeds);
		vector::shuffle(seeds);
	}

	//set tiles without zones (e.g. water tiles which are enclaves in land zones) to the most-neighbored zone
	std::vector<QPoint> remaining_positions;

	for (int x = 0; x < this->get_width(); ++x) {
		for (int y = 0; y < this->get_height(); ++y) {
			QPoint tile_pos(x, y);
			const int tile_index = point::to_index(tile_pos, this->get_width());

			if (this->tile_zones[tile_index] != -1) {
				continue;
			}

			remaining_positions.push_back(std::move(tile_pos));
		}
	}

	while (!remaining_positions.empty()) {
		for (size_t i = 0; i < remaining_positions.size();) {
			const QPoint &tile_pos = remaining_positions.at(i);
			const int tile_index = point::to_index(tile_pos, this->get_width());

			std::map<int, int> zone_neighbor_counts;

			point::for_each_cardinally_adjacent(tile_pos, [&](QPoint &&adjacent_pos) {
				if (!map->contains(adjacent_pos)) {
					return;
				}

				const int adjacent_tile_index = point::to_index(adjacent_pos, map_size);
				const int adjacent_zone_index = this->tile_zones[adjacent_tile_index];

				if (adjacent_zone_index != -1) {
					zone_neighbor_counts[adjacent_zone_index]++;
				}
			});

			if (zone_neighbor_counts.empty()) {
				//no adjacent zone available (i.e. the tile is surrounded by others which also have no zone), try again in the next loop
				++i;
				continue;
			}

			int best_zone_index = -1;
			int best_zone_neighbor_count = 0;
			for (const auto &[zone_index, count] : zone_neighbor_counts) {
				if (count > best_zone_neighbor_count) {
					best_zone_index = zone_index;
					best_zone_neighbor_count = count;
				}
			}

			assert_throw(best_zone_index != -1);

			//set the zone to the same as the most-neighbored one
			this->tile_zones[tile_index] = best_zone_index;
			this->zone_tiles[best_zone_index].push_back(tile_pos);

			remaining_positions.erase(remaining_positions.begin() + i);
		}
	}
}

void map_generator::generate_countries()
{
	std::vector<const region *> potential_oceans;

	for (const region *region : region::get_all()) {
		if (!region->is_ocean()) {
			continue;
		}

		if (region->get_provinces().empty()) {
			continue;
		}

		potential_oceans.push_back(region);
	}

	vector::shuffle(potential_oceans);

	for (const region *ocean : potential_oceans) {
		if (static_cast<int>(this->generated_provinces.size()) >= this->zone_count) {
			break;
		}

		this->generate_ocean(ocean);
	}

	std::vector<const province *> potential_seas;
	std::vector<const province *> potential_lakes;

	for (const province *province : province::get_all()) {
		if (province->is_sea() || province->is_bay()) {
			potential_seas.push_back(province);
		} else if (province->is_lake()) {
			potential_lakes.push_back(province);
		}
	}

	vector::shuffle(potential_seas);
	vector::shuffle(potential_lakes);

	for (const province *province : potential_seas) {
		if (static_cast<int>(this->generated_provinces.size()) == this->zone_count) {
			break;
		}

		std::vector<int> group_province_indexes;
		this->generate_province(province, group_province_indexes);
	}

	for (const province *province : potential_lakes) {
		if (static_cast<int>(this->generated_provinces.size()) == this->zone_count) {
			break;
		}

		std::vector<int> group_zone_indexes;
		this->generate_province(province, group_zone_indexes);
	}

	std::vector<const country *> potential_powers;
	std::vector<const country *> potential_minor_nations;

	country_map<std::vector<const province *>> provinces_by_country;

	for (const province *province : province::get_all()) {
		const province_history *province_history = province->get_history();
		const country *owner = province_history->get_owner();
		if (owner == nullptr) {
			continue;
		}

		provinces_by_country[owner].push_back(province);
	}

	for (const auto &[country, country_provinces] : provinces_by_country) {
		if (country->is_great_power()) {
			potential_powers.push_back(country);
		} else {
			potential_minor_nations.push_back(country);
		}
	}

	vector::shuffle(potential_powers);
	vector::shuffle(potential_minor_nations);

	for (const country *country : potential_powers) {
		if (static_cast<int>(this->generated_provinces.size()) >= this->zone_count) {
			break;
		}

		this->generate_country(country, provinces_by_country.find(country)->second);
	}

	for (const country *country : potential_minor_nations) {
		if (static_cast<int>(this->generated_provinces.size()) >= this->zone_count) {
			break;
		}

		this->generate_country(country, provinces_by_country.find(country)->second);
	}

	if (static_cast<int>(this->generated_provinces.size()) != this->zone_count) {
		log::log_error(std::to_string(this->generated_provinces.size()) + " provinces were generated, but " + std::to_string(this->zone_count) + " were needed.");
	}
}

bool map_generator::generate_ocean(const region *ocean)
{
	std::vector<const province *> potential_provinces;
	for (const province *province : ocean->get_provinces()) {
		potential_provinces.push_back(province);
	}

	const std::vector<const province *> provinces = this->generate_province_group(potential_provinces, nullptr);

	return !provinces.empty();
}

bool map_generator::generate_country(const country *country, const std::vector<const province *> &country_provinces)
{
	const province *default_capital_province = country->get_default_capital()->get_province();
	const std::vector<const province *> generated_provinces = this->generate_province_group(country_provinces, default_capital_province->get_history()->get_owner() == country ? default_capital_province : nullptr);

	return !generated_provinces.empty();
}

std::vector<const province *> map_generator::generate_province_group(const std::vector<const province *> &potential_provinces, const province *capital_province)
{
	std::vector<const province *> provinces;
	std::vector<int> group_zone_indexes;

	if (capital_province != nullptr) {
		const int zone_index = this->generate_province(capital_province, group_zone_indexes);
		if (zone_index == -1) {
			return {};
		}

		provinces.push_back(capital_province);
	}

	for (const province *province : vector::shuffled(potential_provinces)) {
		const int zone_index = this->generate_province(province, group_zone_indexes);
		if (zone_index == -1) {
			continue;
		}

		provinces.push_back(province);

		if (static_cast<int>(this->generated_provinces.size()) == this->zone_count) {
			break;
		}
	}

	return provinces;
}

int map_generator::generate_province(const province *province, std::vector<int> &group_zone_indexes)
{
	if (this->generated_provinces.contains(province)) {
		return -1;
	}

	int zone_index = -1;
	const size_t group_generated_province_count = group_zone_indexes.size();

	if (group_generated_province_count == 0) {
		//first province
		std::vector<int> best_zone_indexes;
		int best_distance = 0;

		//get the zones which are as far away from other powers as possible
		for (int i = 0; i < this->zone_count; ++i) {
			if (!this->can_assign_province_to_zone_index(province, i)) {
				continue;
			}

			const zone &zone = this->zones.at(i);
			const QPoint &zone_seed = zone.seed;

			int distance = std::numeric_limits<int>::max();

			if (!province->is_water_zone()) {
				//generate land provinces as distant from other already-generated land provinces as possible
				for (const auto &[other_zone_index, other_province] : this->provinces_by_zone_index) {
					if (other_province->is_water_zone()) {
						continue;
					}

					const map_generator::zone &other_zone = this->zones.at(other_zone_index);
					const QPoint &other_zone_seed = other_zone.seed;
					distance = std::min(distance, point::distance_to(zone_seed, other_zone_seed));
				}
			}

			if (distance > best_distance) {
				best_zone_indexes.clear();
				best_distance = distance;
			}

			if (distance == best_distance) {
				best_zone_indexes.push_back(i);
			}
		}

		if (!best_zone_indexes.empty()) {
			zone_index = vector::get_random(best_zone_indexes);
		}
	} else {
		//pick a zone bordering one of the country's existing provinces
		std::map<int, int> zone_index_border_counts;
		std::set<int> checked_zone_indexes;

		for (const int country_zone_index : group_zone_indexes) {
			for (const int border_zone_index : this->zone_border_zones[country_zone_index]) {
				if (checked_zone_indexes.contains(border_zone_index)) {
					if (!zone_index_border_counts.contains(border_zone_index)) {
						//already checked for suitability and deemed unsuitable
						continue;
					}
				} else {
					checked_zone_indexes.insert(border_zone_index);
					//make the suitability check if it hasn't been done yet
					if (!this->can_assign_province_to_zone_index(province, border_zone_index)) {
						continue;
					}
				}

				++zone_index_border_counts[border_zone_index];
			}
		}

		std::vector<int> best_zone_indexes;
		int best_border_count = 0;
		for (const auto &[border_zone_index, border_count] : zone_index_border_counts) {
			if (border_count > best_border_count) {
				best_zone_indexes.clear();
				best_border_count = border_count;
			}

			if (border_count == best_border_count) {
				best_zone_indexes.push_back(border_zone_index);
			}
		}

		if (!best_zone_indexes.empty()) {
			zone_index = vector::get_random(best_zone_indexes);
		}
	}

	if (zone_index == -1) {
		return zone_index;
	}

	this->provinces_by_zone_index[zone_index] = province;
	group_zone_indexes.push_back(zone_index);

	this->generated_provinces.insert(province);

	return zone_index;
}

bool map_generator::can_assign_province_to_zone_index(const province *province, const int zone_index) const
{
	if (this->provinces_by_zone_index.contains(zone_index)) {
		//already generated
		return false;
	}

	const zone &zone = this->zones.at(zone_index);
	const QPoint &zone_seed = zone.seed;

	if (this->is_tile_water(zone_seed) != province->is_water_zone()) {
		//can only generate water zones on water, and land provinces on land
		return false;
	}

	if ((province->is_sea() || province->is_bay()) && !this->sea_zones.contains(zone_index)) {
		return false;
	}

	if (province->is_lake() && !this->lakes.contains(zone_index)) {
		return false;
	}

	if (province->is_bay()) {
		//bays can only appear adjacent to land provinces
		bool has_adjacent_land = false;
		for (const int border_zone_index : this->zone_border_zones.find(zone_index)->second) {
			const map_generator::zone &border_zone = this->zones.at(border_zone_index);
			const QPoint &border_zone_seed = border_zone.seed;

			if (!this->is_tile_water(border_zone_seed)) {
				has_adjacent_land = true;
				break;
			}
		}

		if (!has_adjacent_land) {
			return false;
		}
	}

	if (!province->get_sites().empty()) {
		terrain_type_map<int> available_terrain_counts;
		for (const auto &[terrain, tiles] : this->zone_tiles_by_terrain[zone_index]) {
			available_terrain_counts[terrain] = static_cast<int>(tiles.size());
		}

		terrain_type_map<int> available_near_water_terrain_counts;
		for (const auto &[terrain, tiles] : this->zone_near_water_tiles_by_terrain[zone_index]) {
			available_near_water_terrain_counts[terrain] = static_cast<int>(tiles.size());
		}

		for (const site *site : province->get_sites()) {
			const resource *resource = site->get_map_data()->get_resource();
			if (resource == nullptr) {
				continue;
			}

			const std::vector<const terrain_type *> &site_terrains = resource->get_terrain_types();

			bool has_terrain = false;
			for (const terrain_type *terrain : site_terrains) {
				int &terrain_count = resource->is_near_water() ? available_near_water_terrain_counts[terrain] : available_terrain_counts[terrain];
				if (terrain_count > 0) {
					has_terrain = true;
					--terrain_count;
					break;
				}
			}

			if (!has_terrain) {
				return false;
			}
		}
	}

	return true;
}

void map_generator::generate_sites()
{
	map *map = map::get();

	for (const auto &[zone_index, province] : this->provinces_by_zone_index) {
		assert_throw(province != nullptr);

		const zone &zone = this->zones[zone_index];
		const QPoint &zone_seed = zone.seed;

		//place capital settlement
		if (province->get_provincial_capital() != nullptr) {
			map->set_tile_site(zone_seed, province->get_provincial_capital());

			//change non-flatlands or forested terrain to unforested flatlands for settlements
			const terrain_type *tile_terrain = map->get_tile(zone_seed)->get_terrain();
			if (tile_terrain->get_elevation_type() != elevation_type::flatlands || tile_terrain->get_forestation_type() != forestation_type::none) {
				const temperature_type temperature_type = this->get_tile_temperature_type(zone_seed);
				const moisture_type moisture_type = this->get_tile_moisture_type(zone_seed);

				const terrain_type *terrain = terrain_type::get_by_biome(elevation_type::flatlands, temperature_type, moisture_type, forestation_type::none);

				map->set_tile_terrain(zone_seed, terrain);
			}
		}

		for (const site *site : province->get_sites()) {
			if (site->get_type() != site_type::resource) {
				continue;
			}

			const resource *resource = site->get_map_data()->get_resource();
			const std::vector<const terrain_type *> &site_terrains = resource->get_terrain_types();

			const terrain_type_map<std::vector<QPoint>> &zone_tiles_by_terrain = resource->is_near_water() ? this->zone_near_water_tiles_by_terrain[zone_index] : this->zone_tiles_by_terrain[zone_index];

			std::vector<QPoint> potential_positions;

			for (const terrain_type *terrain : site_terrains) {
				const auto find_iterator = zone_tiles_by_terrain.find(terrain);
				if (find_iterator == zone_tiles_by_terrain.end()) {
					continue;
				}

				for (const QPoint &tile_pos : find_iterator->second) {
					if (map->get_tile(tile_pos)->get_site() != nullptr) {
						continue;
					}

					potential_positions.push_back(tile_pos);
				}
			}

			if (potential_positions.empty()) {
				log::log_error(std::format("Could not find position to generate site \"{}\" on.", site->get_identifier()));
				continue;
			}

			const QPoint &site_pos = vector::get_random(potential_positions);
			map->set_tile_site(site_pos, site);
		}
	}
}

elevation_type map_generator::get_tile_elevation_type(const QPoint &tile_pos) const
{
	const int elevation = this->tile_elevations[point::to_index(tile_pos, this->get_width())];

	if (elevation >= this->get_min_mountain_elevation()) {
		return elevation_type::mountains;
	} else if (elevation >= this->get_min_hill_elevation()) {
		return elevation_type::hills;
	} else if (elevation >= this->get_min_land_elevation()) {
		return elevation_type::flatlands;
	} else {
		return elevation_type::water;
	}
}

bool map_generator::is_tile_water(const QPoint &tile_pos) const
{
	return this->get_tile_elevation_type(tile_pos) == elevation_type::water;
}

int map_generator::get_tile_temperature(const QPoint &tile_pos) const
{
	//temperature is a function of latitude and elevation
	const int tile_index = point::to_index(tile_pos, this->get_width());
	const int colatitude = this->get_tile_colatitude(tile_pos);
	const int land_elevation = std::max(0, this->tile_elevations[tile_index] - this->get_min_land_elevation());
	return std::max(0, colatitude - land_elevation / 2);
}

temperature_type map_generator::get_tile_temperature_type(const QPoint &tile_pos) const
{
	const int temperature = this->get_tile_temperature(tile_pos);

	if (temperature >= map_generator::min_tropical_temperature) {
		return temperature_type::tropical;
	} else if (temperature >= map_generator::min_temperate_temperature) {
		return temperature_type::temperate;
	} else {
		return temperature_type::cold;
	}
}

moisture_type map_generator::get_tile_moisture_type(const QPoint &tile_pos) const
{
	const int moisture = this->tile_moistures[point::to_index(tile_pos, this->get_width())];

	if (moisture >= map_generator::min_wet_moisture) {
		return moisture_type::wet;
	} else if (moisture >= map_generator::min_moist_moisture) {
		return moisture_type::moist;
	} else if (moisture >= map_generator::min_dry_moisture) {
		return moisture_type::dry;
	} else if (moisture >= map_generator::min_semi_arid_moisture) {
		return moisture_type::semi_arid;
	} else {
		return moisture_type::arid;
	}
}

forestation_type map_generator::get_tile_forestation_type(const QPoint &tile_pos) const
{
	const int forestation = this->tile_forestations[point::to_index(tile_pos, this->get_width())];

	if (forestation >= map_generator::min_forest_forestation) {
		return forestation_type::forest;
	} else {
		return forestation_type::none;
	}
}

int map_generator::get_min_land_elevation() const
{
	return map_generator::max_elevation * (100 - this->get_map_template()->get_land_percent()) / 100;
}

int map_generator::get_min_hill_elevation() const
{
	const int min_land_elevation = this->get_min_land_elevation();
	return ((map_generator::max_elevation - min_land_elevation) * (100 - this->get_map_template()->get_steepness())) / 100 + min_land_elevation;
}

int map_generator::get_min_mountain_elevation() const
{
	const int min_hill_elevation = this->get_min_hill_elevation();
	return (map_generator::max_elevation + min_hill_elevation) / 2;
}

int map_generator::get_cold_level() const
{
	return std::max(0, map_generator::max_colatitude * (60 * 7 - this->get_map_template()->get_average_temperature() * 6) / 700);
}

bool map_generator::is_tile_near_edge(const QPoint &tile_pos) const
{
	static constexpr int distance = 2;

	if (!this->is_x_wrap_enabled()) {
		const int tile_x = tile_pos.x();
		if (tile_x < distance || tile_x >= (this->get_width() - distance)) {
			return true;
		}
	}

	if (!this->is_y_wrap_enabled()) {
		const int tile_y = tile_pos.y();
		if (tile_y < distance || tile_y >= (this->get_height() - distance)) {
			return true;
		}
	}

	return false;
}

int map_generator::get_sqrt_size() const
{
	int sqrt_size = number::sqrt(this->get_area() / 1000);
	return std::max(1, sqrt_size);
}

}
