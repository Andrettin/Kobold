#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"
#include "util/geocoordinate.h"
#include "util/qunique_ptr.h"

Q_MOC_INCLUDE("economy/resource.h")
Q_MOC_INCLUDE("map/province.h")
Q_MOC_INCLUDE("map/site_game_data.h")
Q_MOC_INCLUDE("map/site_map_data.h")
Q_MOC_INCLUDE("map/terrain_type.h")
Q_MOC_INCLUDE("map/world.h")

namespace kobold {

class cultural_group;
class culture;
class province;
class resource;
class site_game_data;
class site_history;
class site_map_data;
class terrain_type;
class world;
enum class site_type;

class site final : public named_data_entry, public data_type<site>
{
	Q_OBJECT

	Q_PROPERTY(kobold::world* world MEMBER world)
	Q_PROPERTY(archimedes::geocoordinate geocoordinate MEMBER geocoordinate READ get_geocoordinate)
	Q_PROPERTY(QPoint pos_offset MEMBER pos_offset READ get_pos_offset)
	Q_PROPERTY(kobold::site_type type MEMBER type READ get_type)
	Q_PROPERTY(bool settlement READ is_settlement NOTIFY changed)
	Q_PROPERTY(kobold::terrain_type* terrain_type MEMBER terrain_type)
	Q_PROPERTY(kobold::resource* resource MEMBER resource NOTIFY changed)
	Q_PROPERTY(kobold::province* province MEMBER province NOTIFY changed)
	Q_PROPERTY(kobold::site_map_data* map_data READ get_map_data NOTIFY changed)
	Q_PROPERTY(kobold::site_game_data* game_data READ get_game_data NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "site";
	static constexpr const char property_class_identifier[] = "kobold::site*";
	static constexpr const char database_folder[] = "sites";
	static constexpr bool history_enabled = true;

public:
	explicit site(const std::string &identifier);
	~site();

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void initialize() override;
	virtual void check() const override;
	virtual data_entry_history *get_history_base() override;

	site_history *get_history() const
	{
		return this->history.get();
	}

	virtual void reset_history() override;

	void reset_map_data();

	site_map_data *get_map_data() const
	{
		return this->map_data.get();
	}

	void reset_game_data();

	site_game_data *get_game_data() const
	{
		return this->game_data.get();
	}

	const kobold::world *get_world() const
	{
		return this->world;
	}

	const archimedes::geocoordinate &get_geocoordinate() const
	{
		return this->geocoordinate;
	}

	const QPoint &get_pos_offset() const
	{
		return this->pos_offset;
	}

	site_type get_type() const
	{
		return this->type;
	}

	bool is_settlement() const;

	const kobold::terrain_type *get_terrain_type() const
	{
		return this->terrain_type;
	}

	const kobold::resource *get_resource() const
	{
		return this->resource;
	}

	kobold::province *get_province() const
	{
		return this->province;
	}

	void set_province(kobold::province *province)
	{
		this->province = province;
	}

	const std::vector<const kobold::terrain_type *> &get_terrain_types() const
	{
		return this->terrain_types;
	}

	virtual std::string get_scope_name() const override;
	const std::string &get_cultural_name(const culture *culture) const;

signals:
	void changed();

private:
	kobold::world *world = nullptr;
	archimedes::geocoordinate geocoordinate;
	QPoint pos_offset = QPoint(0, 0);
	site_type type;
	kobold::terrain_type *terrain_type = nullptr;
	kobold::resource *resource = nullptr;
	kobold::province *province = nullptr;
	std::vector<const kobold::terrain_type *> terrain_types;
	std::map<const culture *, std::string> cultural_names;
	std::map<const cultural_group *, std::string> cultural_group_names;
	qunique_ptr<site_history> history;
	qunique_ptr<site_map_data> map_data;
	qunique_ptr<site_game_data> game_data;
};

}
