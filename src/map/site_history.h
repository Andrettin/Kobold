#pragma once

#include "database/data_entry_history.h"
#include "infrastructure/building_slot_type_container.h"

Q_MOC_INCLUDE("country/culture.h")
Q_MOC_INCLUDE("infrastructure/settlement_type.h")
Q_MOC_INCLUDE("religion/religion.h")

namespace kobold {

class building_type;
class flag;
class improvement;
class settlement_type;
class site;
class wonder;
enum class improvement_slot;

class site_history final : public data_entry_history
{
	Q_OBJECT

	Q_PROPERTY(bool resource_discovered MEMBER resource_discovered READ is_resource_discovered)
	Q_PROPERTY(bool developed MEMBER developed)
	Q_PROPERTY(kobold::settlement_type* settlement_type MEMBER settlement_type)
	Q_PROPERTY(kobold::culture* culture MEMBER culture)
	Q_PROPERTY(kobold::religion* religion MEMBER religion)
	Q_PROPERTY(std::vector<const kobold::flag *> flags READ get_flags)

public:
	explicit site_history(const kobold::site *site) : site(site)
	{
	}

	virtual void process_gsml_property(const gsml_property &property) override;
	virtual void process_gsml_scope(const gsml_data &scope) override;

	bool is_resource_discovered() const
	{
		return this->resource_discovered;
	}

	bool is_developed() const
	{
		return this->developed || this->get_settlement_type() != nullptr || !this->get_improvements().empty() || !this->get_buildings().empty() || !this->get_wonders().empty();
	}

	const kobold::settlement_type *get_settlement_type() const
	{
		return this->settlement_type;
	}

	const std::map<improvement_slot, const improvement *> &get_improvements() const
	{
		return this->improvements;
	}

	const improvement *get_improvement(const improvement_slot slot) const
	{
		const auto find_iterator = this->improvements.find(slot);
		if (find_iterator != this->improvements.end()) {
			return find_iterator->second;
		}

		return nullptr;
	}

	const building_slot_type_map<const building_type *> &get_buildings() const
	{
		return this->buildings;
	}

	const building_type *get_building(const building_slot_type *slot_type) const
	{
		const auto find_iterator = this->buildings.find(slot_type);
		if (find_iterator != this->buildings.end()) {
			return find_iterator->second;
		}

		return nullptr;
	}

	const building_slot_type_map<const wonder *> &get_wonders() const
	{
		return this->wonders;
	}

	const wonder *get_wonder(const building_slot_type *slot_type) const
	{
		const auto find_iterator = this->wonders.find(slot_type);
		if (find_iterator != this->wonders.end()) {
			return find_iterator->second;
		}

		return nullptr;
	}

	const kobold::culture *get_culture() const
	{
		return this->culture;
	}

	const kobold::religion *get_religion() const
	{
		return this->religion;
	}

	const std::vector<const flag *> &get_flags() const
	{
		return this->flags;
	}

	Q_INVOKABLE void add_flag(const flag *flag)
	{
		this->flags.push_back(flag);
	}

	Q_INVOKABLE void remove_flag(const flag *flag)
	{
		std::erase(this->flags, flag);
	}

private:
	const kobold::site *site = nullptr;
	bool resource_discovered = false;
	bool developed = false;
	kobold::settlement_type *settlement_type = nullptr;
	kobold::culture *culture = nullptr;
	kobold::religion *religion = nullptr;
	std::map<improvement_slot, const improvement *> improvements;
	building_slot_type_map<const building_type *> buildings;
	building_slot_type_map<const wonder *> wonders;
	std::vector<const flag *> flags;
};

}
