#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"
#include "economy/commodity_container.h"

Q_MOC_INCLUDE("country/law_group.h")
Q_MOC_INCLUDE("ui/icon.h")

namespace kobold {

class law_group;

template <typename scope_type>
class and_condition;

template <typename scope_type>
class modifier;

class law final : public named_data_entry, public data_type<law>
{
	Q_OBJECT

	Q_PROPERTY(kobold::law_group * group MEMBER group NOTIFY changed)
	Q_PROPERTY(const kobold::icon* icon MEMBER icon READ get_icon NOTIFY changed)
	Q_PROPERTY(QVariantList commodity_costs READ get_commodity_costs_qvariant_list NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "law";
	static constexpr const char property_class_identifier[] = "kobold::law*";
	static constexpr const char database_folder[] = "laws";

	explicit law(const std::string &identifier);
	~law();

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void initialize() override;
	virtual void check() const override;

	const law_group *get_group() const
	{
		return this->group;
	}

	const icon *get_icon() const
	{
		return this->icon;
	}

	const commodity_map<int> &get_commodity_costs() const
	{
		return this->commodity_costs;
	}

	QVariantList get_commodity_costs_qvariant_list() const;

	const and_condition<country> *get_conditions() const
	{
		return this->conditions.get();
	}

	const modifier<const country> *get_modifier() const
	{
		return this->modifier.get();
	}

	Q_INVOKABLE QString get_modifier_string(const kobold::country *country) const;

signals:
	void changed();

private:
	law_group *group = nullptr;
	const icon *icon = nullptr;
	commodity_map<int> commodity_costs;
	std::unique_ptr<const and_condition<country>> conditions;
	std::unique_ptr<const modifier<const country>> modifier;
};

}
