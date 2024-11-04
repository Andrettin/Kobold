#pragma once

#include "database/data_entry_container.h"
#include "database/data_type.h"
#include "database/named_data_entry.h"

Q_MOC_INCLUDE("ui/icon.h")

namespace kobold {

class character;
class feat_type;
class icon;

template <typename scope_type>
class and_condition;

template <typename scope_type>
class effect_list;

template <typename scope_type>
class factor;

template <typename scope_type>
class modifier;

class feat final : public named_data_entry, public data_type<feat>
{
	Q_OBJECT

	Q_PROPERTY(const kobold::icon* icon MEMBER icon NOTIFY changed)
	Q_PROPERTY(bool unlimited MEMBER unlimited READ is_unlimited NOTIFY changed)
	Q_PROPERTY(std::string description MEMBER description NOTIFY changed)
	Q_PROPERTY(QString modifier_string READ get_modifier_string CONSTANT)

public:
	static constexpr const char class_identifier[] = "feat";
	static constexpr const char property_class_identifier[] = "kobold::feat*";
	static constexpr const char database_folder[] = "feats";

	explicit feat(const std::string &identifier);
	~feat();

	virtual void process_gsml_property(const gsml_property &property) override;
	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void check() const override;

	const std::vector<const feat_type *> &get_types() const
	{
		return this->types;
	}

	bool has_type(const feat_type *type) const;

	const kobold::icon *get_icon() const
	{
		return this->icon;
	}

	bool is_unlimited() const
	{
		return this->unlimited;
	}

	const std::string &get_description() const
	{
		return this->description;
	}

	const and_condition<character> *get_conditions() const
	{
		return this->conditions.get();
	}

	const kobold::modifier<const character> *get_modifier() const
	{
		return this->modifier.get();
	}

	const effect_list<const character> *get_effects() const
	{
		return this->effects.get();
	}

	QString get_modifier_string() const;

	const factor<character> *get_weight_factor() const
	{
		return this->weight_factor.get();
	}

signals:
	void changed();

private:
	const kobold::icon *icon = nullptr;
	bool unlimited = false;
	std::string description;
	std::vector<const feat_type *> types;
	std::unique_ptr<const and_condition<character>> conditions;
	std::unique_ptr<const kobold::modifier<const character>> modifier;
	std::unique_ptr<const effect_list<const character>> effects;
	std::unique_ptr<factor<character>> weight_factor;
};

}
