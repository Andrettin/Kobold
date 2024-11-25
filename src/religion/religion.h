#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"
#include "religion/religion_base.h"

Q_MOC_INCLUDE("religion/religious_group.h")

namespace kobold {

class deity;
class office;
class religious_group;

class religion final : public religion_base, public data_type<religion>
{
	Q_OBJECT

	Q_PROPERTY(kobold::religious_group* group MEMBER group NOTIFY changed)
	Q_PROPERTY(QColor color MEMBER color READ get_color NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "religion";
	static constexpr const char property_class_identifier[] = "kobold::religion*";
	static constexpr const char database_folder[] = "religions";

	explicit religion(const std::string &identifier) : religion_base(identifier)
	{
	}

	virtual void initialize() override;
	virtual void check() const override;

	const religious_group *get_group() const
	{
		return this->group;
	}

	const QColor &get_color() const
	{
		return this->color;
	}

	const std::vector<const deity *> &get_deities() const
	{
		return this->deities;
	}

	void add_deity(const deity *deity)
	{
		this->deities.push_back(deity);
	}

	const std::string &get_title_name(const government_type *government_type, const country_tier tier) const;
	const std::string &get_office_title_name(const office *office, const government_type *government_type, const country_tier tier, const gender gender) const;

signals:
	void changed();

private:
	religious_group *group = nullptr;
	QColor color;
	std::vector<const deity *> deities;
};

}
