#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"

Q_MOC_INCLUDE("ui/icon.h")

namespace kobold {

class icon;
class improvement;
class terrain_type;

class resource final : public named_data_entry, public data_type<resource>
{
	Q_OBJECT

	Q_PROPERTY(const kobold::icon* icon MEMBER icon READ get_icon NOTIFY changed)
	Q_PROPERTY(const kobold::icon* tiny_icon MEMBER tiny_icon READ get_tiny_icon NOTIFY changed)
	Q_PROPERTY(bool coastal MEMBER coastal READ is_coastal NOTIFY changed)
	Q_PROPERTY(bool near_water MEMBER near_water READ is_near_water NOTIFY changed)
	Q_PROPERTY(bool prospectable MEMBER prospectable READ is_prospectable NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "resource";
	static constexpr const char property_class_identifier[] = "kobold::resource*";
	static constexpr const char database_folder[] = "resources";

	explicit resource(const std::string &identifier) : named_data_entry(identifier)
	{
	}

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void check() const override;

	const kobold::icon *get_icon() const
	{
		return this->icon;
	}

	const kobold::icon *get_tiny_icon() const
	{
		return this->tiny_icon;
	}

	bool is_coastal() const
	{
		return this->coastal;
	}

	bool is_near_water() const
	{
		return this->near_water;
	}

	bool is_prospectable() const
	{
		return this->prospectable;
	}

	const std::vector<const terrain_type *> &get_terrain_types() const
	{
		return this->terrain_types;
	}

	const terrain_type *get_fallback_terrain(const terrain_type *terrain) const;

	const std::vector<const improvement *> &get_improvements() const
	{
		return this->improvements;
	}

	void add_improvement(const improvement *improvement)
	{
		this->improvements.push_back(improvement);
	}

signals:
	void changed();

private:
	const kobold::icon *icon = nullptr;
	const kobold::icon *tiny_icon = nullptr;
	bool coastal = false;
	bool near_water = false;
	bool prospectable = false;
	std::vector<const terrain_type *> terrain_types;
	std::vector<const improvement *> improvements;
};

}
