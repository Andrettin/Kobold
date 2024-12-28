#pragma once

#include "country/country_container.h"
#include "database/data_entry_history.h"
#include "economy/commodity_container.h"
#include "util/fractional_int.h"

Q_MOC_INCLUDE("character/character.h")
Q_MOC_INCLUDE("country/government_type.h")
Q_MOC_INCLUDE("religion/religion.h")

namespace kobold {

class character;
class consulate;
class country;
class flag;
class government_type;
class religion;
class subject_type;
class tradition;
enum class country_tier;
enum class diplomacy_state;

class country_history final : public data_entry_history
{
	Q_OBJECT

	Q_PROPERTY(kobold::country_tier tier MEMBER tier)
	Q_PROPERTY(kobold::religion* religion MEMBER religion)
	Q_PROPERTY(const kobold::government_type* government_type MEMBER government_type READ get_government_type)
	Q_PROPERTY(std::vector<const kobold::tradition *> traditions READ get_traditions)
	Q_PROPERTY(int wealth MEMBER wealth READ get_wealth)
	Q_PROPERTY(std::vector<const kobold::flag *> flags READ get_flags)

public:
	explicit country_history(const kobold::country *country);

	virtual void process_gsml_scope(const gsml_data &scope) override;

	country_tier get_tier() const
	{
		return this->tier;
	}

	const kobold::religion *get_religion() const
	{
		return this->religion;
	}

	const kobold::subject_type *get_subject_type() const
	{
		return this->subject_type;
	}

	const kobold::government_type *get_government_type() const
	{
		return this->government_type;
	}

	const std::vector<const tradition *> &get_traditions() const
	{
		return this->traditions;
	}

	Q_INVOKABLE void add_tradition(const tradition *tradition)
	{
		this->traditions.push_back(tradition);
	}

	Q_INVOKABLE void remove_tradition(const tradition *tradition)
	{
		std::erase(this->traditions, tradition);
	}

	int get_wealth() const
	{
		return this->wealth;
	}

	const commodity_map<int> &get_commodities() const
	{
		return this->commodities;
	}

	const country_map<diplomacy_state> &get_diplomacy_states() const
	{
		return this->diplomacy_states;
	}

	diplomacy_state get_diplomacy_state(const kobold::country *other_country) const;

	const country_map<const consulate *> &get_consulates() const
	{
		return this->consulates;
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
	const kobold::country *country = nullptr;
	country_tier tier{};
	kobold::religion *religion = nullptr;
	const kobold::subject_type *subject_type = nullptr;
	const kobold::government_type *government_type = nullptr;
	std::vector<const tradition *> traditions;
	int wealth = 0;
	commodity_map<int> commodities;
	country_map<diplomacy_state> diplomacy_states;
	country_map<const consulate *> consulates;
	std::vector<const flag *> flags;
};

}
