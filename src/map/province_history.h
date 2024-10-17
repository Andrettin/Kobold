#pragma once

#include "database/data_entry_history.h"
#include "util/fractional_int.h"

Q_MOC_INCLUDE("country/country.h")
Q_MOC_INCLUDE("country/religion.h")

namespace kobold {

class country;
class culture;
class province;
class religion;

class province_history final : public data_entry_history
{
	Q_OBJECT

	Q_PROPERTY(kobold::country* owner MEMBER owner)
	Q_PROPERTY(kobold::culture* culture MEMBER culture)
	Q_PROPERTY(kobold::religion* religion MEMBER religion)

public:
	explicit province_history(const province *province) : province(province)
	{
	}

	const country *get_owner() const
	{
		return this->owner;
	}

	const kobold::culture *get_culture() const
	{
		return this->culture;
	}

	const kobold::religion *get_religion() const
	{
		return this->religion;
	}

private:
	const kobold::province *province = nullptr;
	country *owner = nullptr;
	kobold::culture *culture = nullptr;
	kobold::religion *religion = nullptr;
};

}
