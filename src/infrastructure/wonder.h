#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"

Q_MOC_INCLUDE("infrastructure/building_type.h")
Q_MOC_INCLUDE("ui/portrait.h")

namespace kobold {

class building_type;
class country;
class portrait;
class province;

template <typename scope_type>
class condition;

template <typename scope_type>
class factor;

template <typename scope_type>
class modifier;

class wonder final : public named_data_entry, public data_type<wonder>
{
	Q_OBJECT

	Q_PROPERTY(kobold::portrait* portrait MEMBER portrait NOTIFY changed)
	Q_PROPERTY(kobold::building_type* building MEMBER building NOTIFY changed)
	Q_PROPERTY(int wealth_cost MEMBER wealth_cost READ get_wealth_cost NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "wonder";
	static constexpr const char property_class_identifier[] = "kobold::wonder*";
	static constexpr const char database_folder[] = "wonders";

	explicit wonder(const std::string &identifier);
	~wonder();

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void initialize() override;
	virtual void check() const override;

	const kobold::portrait *get_portrait() const
	{
		return this->portrait;
	}

	const building_type *get_building() const
	{
		return this->building;
	}

	int get_wealth_cost() const
	{
		return this->wealth_cost;
	}

	int get_wealth_cost_for_country(const country *country) const;

	const commodity_map<int> &get_commodity_costs() const
	{
		return this->commodity_costs;
	}

	commodity_map<int> get_commodity_costs_for_country(const country *country) const;

	const factor<country> *get_cost_factor() const
	{
		return this->cost_factor.get();
	}

	const condition<country> *get_conditions() const
	{
		return this->conditions.get();
	}

	const condition<province> *get_province_conditions() const
	{
		return this->province_conditions.get();
	}

	const modifier<const province> *get_province_modifier() const
	{
		return this->province_modifier.get();
	}

	const modifier<const country> *get_country_modifier() const
	{
		return this->country_modifier.get();
	}

signals:
	void changed();

private:
	kobold::portrait *portrait = nullptr;
	building_type *building = nullptr;
	int wealth_cost = 0;
	commodity_map<int> commodity_costs;
	std::unique_ptr<const factor<country>> cost_factor;
	std::unique_ptr<const condition<country>> conditions;
	std::unique_ptr<const condition<province>> province_conditions;
	std::unique_ptr<modifier<const province>> province_modifier;
	std::unique_ptr<modifier<const country>> country_modifier;
};

}
