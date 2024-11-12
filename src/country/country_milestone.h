#pragma once

#include "database/data_entry_container.h"
#include "database/data_type.h"
#include "database/named_data_entry.h"

Q_MOC_INCLUDE("ui/icon.h")

namespace kobold {

class country;
class country_feat_type;
class icon;

template <typename scope_type>
class and_condition;

template <typename scope_type>
class effect_list;

template <typename scope_type>
class factor;

template <typename scope_type>
class modifier;

class country_milestone final : public named_data_entry, public data_type<country_milestone>
{
	Q_OBJECT

	Q_PROPERTY(const kobold::portrait* portrait MEMBER portrait READ get_portrait NOTIFY changed)
	Q_PROPERTY(QString effects_string READ get_effects_string CONSTANT)

public:
	static constexpr const char class_identifier[] = "country_milestone";
	static constexpr const char property_class_identifier[] = "kobold::country_milestone*";
	static constexpr const char database_folder[] = "country_milestones";

	explicit country_milestone(const std::string &identifier);
	~country_milestone();

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void check() const override;

	const kobold::portrait *get_portrait() const
	{
		return this->portrait;
	}

	const and_condition<country> *get_conditions() const
	{
		return this->conditions.get();
	}

	const effect_list<const country> *get_effects() const
	{
		return this->effects.get();
	}

	QString get_effects_string() const;

signals:
	void changed();

private:
	const kobold::portrait *portrait = nullptr;
	std::unique_ptr<and_condition<country>> conditions;
	std::unique_ptr<const effect_list<const country>> effects;
};

}
