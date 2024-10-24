#pragma once

namespace kobold {

class route;

class route_game_data final : public QObject
{
	Q_OBJECT

public:
	explicit route_game_data(const kobold::route *route) : route(route)
	{
	}

	const std::vector<QPoint> &get_tiles() const
	{
		return this->tiles;
	}

	void add_tile(const QPoint &tile_pos)
	{
		this->tiles.push_back(tile_pos);
	}

	bool is_on_map() const
	{
		return !this->tiles.empty();
	}

private:
	const kobold::route *route = nullptr;
	std::vector<QPoint> tiles;
};

}
