#pragma once

#include "util/fractional_int.h"

Q_MOC_INCLUDE("country/country.h")
Q_MOC_INCLUDE("country/culture.h")
Q_MOC_INCLUDE("country/religion.h")
Q_MOC_INCLUDE("map/province.h")
Q_MOC_INCLUDE("map/site.h")
Q_MOC_INCLUDE("population/phenotype.h")
Q_MOC_INCLUDE("population/population_type.h")
Q_MOC_INCLUDE("ui/icon.h")

namespace kobold {

class country;
class culture;
class employment_location;
class icon;
class ideology;
class phenotype;
class population_type;
class profession;
class province;
class religion;
class settlement_building_slot;
class site;

class population_unit final : public QObject
{
	Q_OBJECT

	Q_PROPERTY(const kobold::population_type* type READ get_type NOTIFY type_changed)
	Q_PROPERTY(const kobold::culture* culture READ get_culture NOTIFY culture_changed)
	Q_PROPERTY(const kobold::religion* religion READ get_religion NOTIFY religion_changed)
	Q_PROPERTY(const kobold::phenotype* phenotype READ get_phenotype NOTIFY phenotype_changed)
	Q_PROPERTY(const kobold::icon* icon READ get_icon NOTIFY icon_changed)
	Q_PROPERTY(const kobold::country* country READ get_country NOTIFY country_changed)
	Q_PROPERTY(const kobold::province* province READ get_province NOTIFY province_changed)
	Q_PROPERTY(const kobold::site* settlement READ get_settlement NOTIFY settlement_changed)

public:
	static constexpr int max_consciousness = 10;
	static constexpr int max_militancy = 10;

	explicit population_unit(const population_type *type, const kobold::culture *culture, const kobold::religion *religion, const kobold::phenotype *phenotype, const site *settlement);

	void do_turn();

	std::string get_scope_name() const;

	const population_type *get_type() const
	{
		return this->type;
	}

	void set_type(const population_type *type);

	const culture *get_culture() const
	{
		return this->culture;
	}

	void set_culture(const kobold::culture *culture);

	const religion *get_religion() const
	{
		return this->religion;
	}

	void set_religion(const kobold::religion *religion);

	const phenotype *get_phenotype() const
	{
		return this->phenotype;
	}

	void set_phenotype(const kobold::phenotype *phenotype);

	const icon *get_icon() const;
	const icon *get_small_icon() const;

	const kobold::country *get_country() const
	{
		return this->country;
	}

	void set_country(const kobold::country *country);

	const province *get_province() const;

	const site *get_settlement() const
	{
		return this->settlement;
	}

	void set_settlement(const site *settlement);

	const kobold::ideology *get_ideology() const
	{
		return this->ideology;
	}

	void set_ideology(const kobold::ideology *ideology);
	void choose_ideology();

	const centesimal_int &get_consciousness() const
	{
		return this->consciousness;
	}

	void set_consciousness(const centesimal_int &consciousness);

	void change_consciousness(const centesimal_int &change)
	{
		this->set_consciousness(this->get_consciousness() + change);
	}

	const centesimal_int &get_militancy() const
	{
		return this->militancy;
	}

	void set_militancy(const centesimal_int &militancy);

	void change_militancy(const centesimal_int &change)
	{
		this->set_militancy(this->get_militancy() + change);
	}

	kobold::employment_location *get_employment_location() const
	{
		return this->employment_location;
	}

	bool is_unemployed() const
	{
		return this->get_employment_location() == nullptr;
	}

	void set_employment_location(kobold::employment_location *employment_location);

	const profession *get_profession() const;
	bool is_food_producer() const;

	bool is_everyday_consumption_fulfilled() const
	{
		return this->everyday_consumption_fulfilled;
	}

	void set_everyday_consumption_fulfilled(const bool value)
	{
		if (value == this->is_everyday_consumption_fulfilled()) {
			return;
		}

		this->everyday_consumption_fulfilled = value;
	}

	bool is_luxury_consumption_fulfilled() const
	{
		return this->luxury_consumption_fulfilled;
	}

	void set_luxury_consumption_fulfilled(const bool value)
	{
		if (value == this->is_luxury_consumption_fulfilled()) {
			return;
		}

		this->luxury_consumption_fulfilled = value;
	}

	void migrate_to(const site *settlement);

signals:
	void type_changed();
	void culture_changed();
	void religion_changed();
	void phenotype_changed();
	void icon_changed();
	void country_changed();
	void province_changed();
	void settlement_changed();

private:
	const population_type *type = nullptr;
	const kobold::culture *culture = nullptr;
	const kobold::religion *religion = nullptr;
	const kobold::phenotype *phenotype = nullptr;
	const kobold::country *country = nullptr;
	const site *settlement = nullptr;
	const kobold::ideology *ideology = nullptr;
	centesimal_int consciousness;
	centesimal_int militancy;
	kobold::employment_location *employment_location = nullptr;
	bool everyday_consumption_fulfilled = true;
	bool luxury_consumption_fulfilled = true;
};

}
