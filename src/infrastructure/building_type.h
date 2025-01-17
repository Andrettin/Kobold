#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"
#include "economy/commodity_container.h"
#include "util/fractional_int.h"

Q_MOC_INCLUDE("country/cultural_group.h")
Q_MOC_INCLUDE("country/culture.h")
Q_MOC_INCLUDE("infrastructure/building_class.h")
Q_MOC_INCLUDE("ui/icon.h")
Q_MOC_INCLUDE("ui/portrait.h")

namespace kobold {

class building_class;
class building_slot_type;
class country;
class country_skill;
class cultural_group;
class culture;
class icon;
class portrait;
class province;
class settlement_type;
class site;

template <typename scope_type>
class and_condition;

template <typename scope_type>
class effect_list;

template <typename scope_type>
class factor;

template <typename scope_type>
class modifier;

class building_type final : public named_data_entry, public data_type<building_type>
{
	Q_OBJECT

	Q_PROPERTY(kobold::building_class* building_class MEMBER building_class NOTIFY changed)
	Q_PROPERTY(kobold::culture* culture MEMBER culture NOTIFY changed)
	Q_PROPERTY(kobold::cultural_group* cultural_group MEMBER cultural_group NOTIFY changed)
	Q_PROPERTY(const kobold::portrait* portrait MEMBER portrait READ get_portrait NOTIFY changed)
	Q_PROPERTY(const kobold::icon* icon MEMBER icon READ get_icon NOTIFY changed)
	Q_PROPERTY(bool provincial MEMBER provincial READ is_provincial NOTIFY changed)
	Q_PROPERTY(bool capitol MEMBER capitol READ is_capitol NOTIFY changed)
	Q_PROPERTY(bool provincial_capitol MEMBER provincial_capitol READ is_provincial_capitol NOTIFY changed)
	Q_PROPERTY(bool warehouse MEMBER warehouse READ is_warehouse NOTIFY changed)
	Q_PROPERTY(bool free_in_capital MEMBER free_in_capital READ is_free_in_capital NOTIFY changed)
	Q_PROPERTY(bool free_on_start MEMBER free_on_start READ is_free_on_start NOTIFY changed)
	Q_PROPERTY(bool capital_only MEMBER capital_only READ is_capital_only NOTIFY changed)
	Q_PROPERTY(bool provincial_capital_only MEMBER provincial_capital_only READ is_provincial_capital_only NOTIFY changed)
	Q_PROPERTY(bool wonder_only MEMBER wonder_only READ is_wonder_only NOTIFY changed)
	Q_PROPERTY(int fort_level MEMBER fort_level READ get_fort_level NOTIFY changed)
	Q_PROPERTY(kobold::building_type* required_building MEMBER required_building NOTIFY changed)
	Q_PROPERTY(const kobold::country_skill* construction_skill MEMBER construction_skill READ get_construction_skill NOTIFY changed)
	Q_PROPERTY(int construction_difficulty_class MEMBER construction_difficulty_class READ get_construction_difficulty_class NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "building_type";
	static constexpr const char property_class_identifier[] = "kobold::building_type*";
	static constexpr const char database_folder[] = "building_types";

	static const std::set<std::string> database_dependencies;

public:
	explicit building_type(const std::string &identifier);
	~building_type();

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void initialize() override;
	virtual void check() const override;

	const building_class *get_building_class() const
	{
		return this->building_class;
	}

	const building_slot_type *get_slot_type() const;

	const kobold::culture *get_culture() const
	{
		return this->culture;
	}

	const kobold::cultural_group *get_cultural_group() const
	{
		return this->cultural_group;
	}

	const kobold::portrait *get_portrait() const
	{
		return this->portrait;
	}

	const kobold::icon *get_icon() const
	{
		return this->icon;
	}

	bool is_provincial() const
	{
		return this->provincial;
	}

	int get_level() const
	{
		return this->level;
	}

	void calculate_level();

	const std::vector<const settlement_type *> &get_settlement_types() const
	{
		return this->settlement_types;
	}

	bool is_capitol() const
	{
		return this->capitol;
	}

	bool is_provincial_capitol() const
	{
		return this->provincial_capitol;
	}

	bool is_warehouse() const
	{
		return this->warehouse;
	}

	bool is_free_in_capital() const
	{
		return this->free_in_capital;
	}

	bool is_free_on_start() const
	{
		return this->free_on_start;
	}

	bool is_capital_only() const
	{
		return this->capital_only;
	}

	bool is_provincial_capital_only() const
	{
		return this->provincial_capital_only;
	}

	bool is_wonder_only() const
	{
		return this->wonder_only;
	}

	int get_fort_level() const
	{
		return this->fort_level;
	}

	const building_type *get_required_building() const
	{
		return this->required_building;
	}

	bool is_any_required_building(const building_type *building) const
	{
		if (building == nullptr) {
			return false;
		}

		if (this->get_required_building() == nullptr) {
			return false;
		}

		if (building == this->get_required_building()) {
			return true;
		}

		return this->get_required_building()->is_any_required_building(building);
	}

	const std::vector<const building_type *> &get_requiring_buildings() const
	{
		return this->requiring_buildings;
	}

	const country_skill *get_construction_skill() const
	{
		return this->construction_skill;
	}

	int get_construction_difficulty_class() const
	{
		return this->construction_difficulty_class;
	}

	const commodity_map<int> &get_commodity_costs() const
	{
		return this->commodity_costs;
	}

	commodity_map<int> get_commodity_costs_for_country(const country *country) const;

	const factor<country> *get_cost_factor() const
	{
		return this->cost_factor.get();
	}

	const and_condition<country> *get_conditions() const
	{
		return this->conditions.get();
	}

	const and_condition<site> *get_settlement_conditions() const
	{
		return this->settlement_conditions.get();
	}

	const and_condition<province> *get_province_conditions() const
	{
		return this->province_conditions.get();
	}

	const and_condition<site> *get_build_conditions() const
	{
		return this->build_conditions.get();
	}

	const modifier<const site> *get_settlement_modifier() const
	{
		return this->settlement_modifier.get();
	}

	const modifier<const province> *get_province_modifier() const
	{
		return this->province_modifier.get();
	}

	const modifier<const country> *get_country_modifier() const
	{
		return this->country_modifier.get();
	}

	Q_INVOKABLE QString get_effects_string(kobold::site *site) const;

	const effect_list<const site> *get_effects() const
	{
		return this->effects.get();
	}

signals:
	void changed();

private:
	building_class *building_class = nullptr;
	kobold::culture *culture = nullptr;
	kobold::cultural_group *cultural_group = nullptr;
	const kobold::portrait *portrait = nullptr;
	const kobold::icon *icon = nullptr;
	bool provincial = false;
	int level = 0;
	std::vector<const settlement_type *> settlement_types;
	bool capitol = false;
	bool provincial_capitol = false;
	bool warehouse = false;
	bool free_in_capital = false;
	bool free_on_start = false;
	bool capital_only = false;
	bool provincial_capital_only = false;
	bool wonder_only = false;
	int fort_level = 0;
	building_type *required_building = nullptr;
	std::vector<const building_type *> requiring_buildings; //buildings which require this one
	const country_skill *construction_skill = nullptr;
	int construction_difficulty_class = 0;
	commodity_map<int> commodity_costs;
	std::unique_ptr<const factor<country>> cost_factor;
	std::unique_ptr<const and_condition<country>> conditions;
	std::unique_ptr<and_condition<site>> settlement_conditions;
	std::unique_ptr<const and_condition<province>> province_conditions;
	std::unique_ptr<const and_condition<site>> build_conditions;
	std::unique_ptr<modifier<const site>> settlement_modifier;
	std::unique_ptr<modifier<const province>> province_modifier;
	std::unique_ptr<modifier<const country>> country_modifier;
	std::unique_ptr<effect_list<const site>> effects;
};

}
