#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"

Q_MOC_INCLUDE("country/culture.h")

namespace kobold {

class culture;

class dynasty final : public named_data_entry, public data_type<dynasty>
{
	Q_OBJECT

	Q_PROPERTY(std::string prefix MEMBER prefix NOTIFY changed)
	Q_PROPERTY(kobold::culture* culture MEMBER culture NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "dynasty";
	static constexpr const char property_class_identifier[] = "kobold::dynasty*";
	static constexpr const char database_folder[] = "dynasties";

	explicit dynasty(const std::string &identifier) : named_data_entry(identifier)
	{
	}

	virtual void check() const override;

	const std::string &get_prefix() const
	{
		return this->prefix;
	}

	const culture *get_culture() const
	{
		return this->culture;
	}

signals:
	void changed();

private:
	std::string prefix;
	kobold::culture *culture = nullptr;
};

}
