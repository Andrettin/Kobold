#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"
#include "economy/commodity_container.h"

Q_MOC_INCLUDE("economy/resource.h")
Q_MOC_INCLUDE("ui/icon.h")

namespace kobold {

class commodity;
class icon;
class resource;
class terrain_type;
class tile;
enum class improvement_slot;

template <typename scope_type>
class modifier;

//tile improvement
class improvement final : public named_data_entry, public data_type<improvement>
{
	Q_OBJECT

	Q_PROPERTY(kobold::improvement_slot slot MEMBER slot READ get_slot NOTIFY changed)
	Q_PROPERTY(bool visitable MEMBER visitable READ is_visitable NOTIFY changed)
	Q_PROPERTY(const kobold::icon* icon MEMBER icon READ get_icon NOTIFY changed)
	Q_PROPERTY(std::filesystem::path image_filepath MEMBER image_filepath WRITE set_image_filepath)
	Q_PROPERTY(int variation_count MEMBER variation_count READ get_variation_count)
	Q_PROPERTY(kobold::improvement* required_improvement MEMBER required_improvement NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "improvement";
	static constexpr const char property_class_identifier[] = "kobold::improvement*";
	static constexpr const char database_folder[] = "improvements";

	explicit improvement(const std::string &identifier);
	~improvement();

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void initialize() override;
	virtual void check() const override;

	improvement_slot get_slot() const
	{
		return this->slot;
	}

	int get_level() const
	{
		return this->level;
	}

	void calculate_level();

	const std::vector<resource *> &get_resources() const
	{
		return this->resources;
	}

	bool is_visitable() const
	{
		return this->visitable;
	}

	const kobold::icon *get_icon() const
	{
		return this->icon;
	}

	const std::filesystem::path &get_image_filepath() const
	{
		return this->image_filepath;
	}

	void set_image_filepath(const std::filesystem::path &filepath);

	const std::filesystem::path &get_terrain_image_filepath(const terrain_type *terrain) const
	{
		const auto find_iterator = this->terrain_image_filepaths.find(terrain);
		if (find_iterator != this->terrain_image_filepaths.end()) {
			return find_iterator->second;
		}

		return this->get_image_filepath();
	}

	bool has_terrain_image_filepath(const terrain_type *terrain) const
	{
		return this->terrain_image_filepaths.contains(terrain);
	}

	const std::vector<const terrain_type *> &get_terrain_types() const
	{
		return this->terrain_types;
	}

	int get_variation_count() const
	{
		return this->variation_count;
	}

	const improvement *get_required_improvement() const
	{
		return this->required_improvement;
	}

	const commodity_map<int> &get_commodity_costs() const
	{
		return this->commodity_costs;
	}

	const kobold::modifier<const site> *get_modifier() const
	{
		return this->modifier.get();
	}

	const kobold::modifier<const country> *get_country_modifier() const
	{
		return this->country_modifier.get();
	}

	bool is_buildable_on_site(const site *site) const;

signals:
	void changed();

private:
	improvement_slot slot{};
	int level = 0;
	std::vector<resource *> resources; //the resource for which this improvement can be built
	bool visitable = false; //if true, this improvement can be visited by an adventurer
	const kobold::icon *icon = nullptr;
	std::filesystem::path image_filepath;
	std::map<const terrain_type *, std::filesystem::path> terrain_image_filepaths;
	std::vector<const terrain_type *> terrain_types; //the terrain types where the improvement can be built
	int variation_count = 1;
	improvement *required_improvement = nullptr;
	commodity_map<int> commodity_costs;
	std::unique_ptr<kobold::modifier<const site>> modifier;
	std::unique_ptr<kobold::modifier<const country>> country_modifier;
};

}
