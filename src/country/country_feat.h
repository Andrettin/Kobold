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

class country_feat final : public named_data_entry, public data_type<country_feat>
{
	Q_OBJECT

	Q_PROPERTY(const kobold::icon* icon MEMBER icon NOTIFY changed)
	Q_PROPERTY(bool unlimited MEMBER unlimited READ is_unlimited NOTIFY changed)
	Q_PROPERTY(kobold::country_feat* upgraded_feat MEMBER upgraded_feat NOTIFY changed)
	Q_PROPERTY(std::string description MEMBER description NOTIFY changed)
	Q_PROPERTY(QString modifier_string READ get_modifier_string CONSTANT)

public:
	static constexpr const char class_identifier[] = "country_feat";
	static constexpr const char property_class_identifier[] = "kobold::country_feat*";
	static constexpr const char database_folder[] = "country_feats";

	explicit country_feat(const std::string &identifier);
	~country_feat();

	virtual void process_gsml_property(const gsml_property &property) override;
	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void initialize() override;
	virtual void check() const override;

	const std::vector<const country_feat_type *> &get_types() const
	{
		return this->types;
	}

	bool has_type(const country_feat_type *type) const;

	const kobold::icon *get_icon() const
	{
		return this->icon;
	}

	void set_icon(const kobold::icon *icon)
	{
		this->icon = icon;
	}

	bool is_unlimited() const
	{
		return this->unlimited;
	}

	const kobold::country_feat *get_upgraded_feat() const
	{
		return this->upgraded_feat;
	}

	const std::string &get_description() const
	{
		return this->description;
	}

	const and_condition<country> *get_conditions() const
	{
		return this->conditions.get();
	}

	const kobold::modifier<const country> *get_modifier() const
	{
		return this->modifier.get();
	}

	const effect_list<const country> *get_effects() const
	{
		return this->effects.get();
	}

	QString get_modifier_string() const;

	const factor<country> *get_weight_factor() const
	{
		return this->weight_factor.get();
	}

signals:
	void changed();

private:
	const kobold::icon *icon = nullptr;
	bool unlimited = false;
	kobold::country_feat *upgraded_feat = nullptr;
	std::string description;
	std::vector<const country_feat_type *> types;
	std::unique_ptr<and_condition<country>> conditions;
	std::unique_ptr<const kobold::modifier<const country>> modifier;
	std::unique_ptr<const effect_list<const country>> effects;
	std::unique_ptr<factor<country>> weight_factor;
};

}
