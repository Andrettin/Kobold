#pragma once

#include "database/data_entry_history.h"

Q_MOC_INCLUDE("country/country.h")
Q_MOC_INCLUDE("country/office.h")
Q_MOC_INCLUDE("map/province.h")
Q_MOC_INCLUDE("map/site.h")

namespace kobold {

class character;
class country;
class feat;
class province;
class site;

class character_history final : public data_entry_history
{
	Q_OBJECT

	Q_PROPERTY(kobold::country* country MEMBER country)
	Q_PROPERTY(int level MEMBER level READ get_level)
	Q_PROPERTY(const kobold::office* office MEMBER office READ get_office)
	Q_PROPERTY(const kobold::character* spouse READ get_spouse WRITE set_spouse)
	Q_PROPERTY(kobold::province* deployment_province MEMBER deployment_province)
	Q_PROPERTY(const kobold::site* deployment_site MEMBER deployment_site)

public:
	explicit character_history(const kobold::character *character);

	virtual void process_gsml_property(const gsml_property &property) override;

	const kobold::country *get_country() const
	{
		return this->country;
	}

	int get_level() const
	{
		return this->level;
	}

	const std::vector<const feat *> &get_feats() const
	{
		return this->feats;
	}

	const kobold::office *get_office() const
	{
		return this->office;
	}

	const kobold::character *get_spouse() const
	{
		return this->spouse;
	}

	void set_spouse(const kobold::character *spouse);

	const province *get_deployment_province() const
	{
		return this->deployment_province;
	}

	const site *get_deployment_site() const
	{
		return this->deployment_site;
	}

private:
	const kobold::character *character = nullptr;
	kobold::country *country = nullptr;
	int level = 0;
	std::vector<const feat *> feats;
	const kobold::office *office = nullptr;
	const kobold::character *spouse = nullptr;
	province *deployment_province = nullptr;
	const site *deployment_site = nullptr;
};

}
