#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"
#include "util/qunique_ptr.h"

Q_MOC_INCLUDE("country/country.h")
Q_MOC_INCLUDE("country/culture.h")
Q_MOC_INCLUDE("country/religion.h")
Q_MOC_INCLUDE("population/phenotype.h")
Q_MOC_INCLUDE("population/population_type.h")
Q_MOC_INCLUDE("unit/military_unit_type.h")

namespace kobold {

class country;
class culture;
class historical_military_unit_history;
class military_unit_type;
class phenotype;
class population_type;
class religion;
class site;

class historical_military_unit final : public named_data_entry, public data_type<historical_military_unit>
{
	Q_OBJECT

	Q_PROPERTY(kobold::military_unit_type* type MEMBER type)
	Q_PROPERTY(kobold::country* country MEMBER country)
	Q_PROPERTY(kobold::site* home_settlement MEMBER home_settlement)
	Q_PROPERTY(kobold::population_type* population_type MEMBER population_type)
	Q_PROPERTY(kobold::culture* culture MEMBER culture)
	Q_PROPERTY(kobold::religion* religion MEMBER religion)
	Q_PROPERTY(kobold::phenotype* phenotype MEMBER phenotype)
	Q_PROPERTY(int quantity MEMBER quantity READ get_quantity)

public:
	static constexpr const char class_identifier[] = "historical_military_unit";
	static constexpr const char property_class_identifier[] = "kobold::historical_military_unit*";
	static constexpr const char database_folder[] = "military_units";
	static constexpr bool history_enabled = true;

	explicit historical_military_unit(const std::string &identifier);
	~historical_military_unit();

	virtual void check() const override;
	virtual data_entry_history *get_history_base() override;

	historical_military_unit_history *get_history() const
	{
		return this->history.get();
	}

	virtual void reset_history() override;

	const military_unit_type *get_type() const
	{
		return this->type;
	}

	const kobold::country *get_country() const
	{
		return this->country;
	}

	const site *get_home_settlement() const
	{
		return this->home_settlement;
	}

	const kobold::population_type *get_population_type() const
	{
		return this->population_type;
	}

	const kobold::culture *get_culture() const
	{
		return this->culture;
	}

	const kobold::religion *get_religion() const
	{
		return this->religion;
	}

	const kobold::phenotype *get_phenotype() const
	{
		return this->phenotype;
	}

	int get_quantity() const
	{
		return this->quantity;
	}

private:
	military_unit_type *type = nullptr;
	kobold::country *country = nullptr;
	site *home_settlement = nullptr;
	kobold::population_type *population_type = nullptr;
	kobold::culture *culture = nullptr;
	kobold::religion *religion = nullptr;
	kobold::phenotype *phenotype = nullptr;
	int quantity = 1;
	qunique_ptr<historical_military_unit_history> history;
};

}
