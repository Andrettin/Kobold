#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"

Q_MOC_INCLUDE("ui/icon.h")

namespace kobold {

class icon;
class military_unit;

template <typename scope_type>
class and_condition;

template <typename scope_type>
class modifier;

class promotion final : public named_data_entry, public data_type<promotion>
{
	Q_OBJECT

	Q_PROPERTY(kobold::icon* icon MEMBER icon NOTIFY changed)
	Q_PROPERTY(QString modifier_string READ get_modifier_string CONSTANT)

public:
	static constexpr const char class_identifier[] = "promotion";
	static constexpr const char property_class_identifier[] = "kobold::promotion*";
	static constexpr const char database_folder[] = "promotions";

	explicit promotion(const std::string &identifier);
	~promotion();

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void check() const override;

	const kobold::icon *get_icon() const
	{
		return this->icon;
	}

	const and_condition<military_unit> *get_conditions() const
	{
		return this->conditions.get();
	}

	const kobold::modifier<military_unit> *get_modifier() const
	{
		return this->modifier.get();
	}

	QString get_modifier_string() const;

signals:
	void changed();

private:
	kobold::icon *icon = nullptr;
	std::unique_ptr<const and_condition<military_unit>> conditions;
	std::unique_ptr<const kobold::modifier<military_unit>> modifier;
};

}
