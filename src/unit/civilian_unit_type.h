#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"
#include "economy/resource_container.h"
#include "infrastructure/pathway_container.h"

Q_MOC_INCLUDE("country/cultural_group.h")
Q_MOC_INCLUDE("country/culture.h")
Q_MOC_INCLUDE("ui/icon.h")
Q_MOC_INCLUDE("unit/civilian_unit_class.h")

namespace kobold {

class civilian_unit_class;
class cultural_group;
class culture;
class icon;

class civilian_unit_type final : public named_data_entry, public data_type<civilian_unit_type>
{
	Q_OBJECT

	Q_PROPERTY(kobold::civilian_unit_class* unit_class MEMBER unit_class NOTIFY changed)
	Q_PROPERTY(kobold::culture* culture MEMBER culture NOTIFY changed)
	Q_PROPERTY(kobold::cultural_group* cultural_group MEMBER cultural_group NOTIFY changed)
	Q_PROPERTY(kobold::icon* icon MEMBER icon NOTIFY changed)
	Q_PROPERTY(bool explorer MEMBER explorer READ is_explorer NOTIFY changed)
	Q_PROPERTY(bool prospector MEMBER prospector READ is_prospector NOTIFY changed)
	Q_PROPERTY(bool developer MEMBER developer READ is_developer NOTIFY changed)
	Q_PROPERTY(bool spy MEMBER spy READ is_spy NOTIFY changed)
	Q_PROPERTY(bool adventurer MEMBER adventurer READ is_adventurer NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "civilian_unit_type";
	static constexpr const char property_class_identifier[] = "kobold::civilian_unit_type*";
	static constexpr const char database_folder[] = "civilian_unit_types";

public:
	explicit civilian_unit_type(const std::string &identifier) : named_data_entry(identifier)
	{
	}

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void initialize() override;
	virtual void check() const override;

	const civilian_unit_class *get_unit_class() const
	{
		return this->unit_class;
	}

	const kobold::culture *get_culture() const
	{
		return this->culture;
	}

	const kobold::cultural_group *get_cultural_group() const
	{
		return this->cultural_group;
	}

	const kobold::icon *get_icon() const
	{
		return this->icon;
	}

	bool is_explorer() const
	{
		return this->explorer;
	}

	bool is_prospector() const
	{
		return this->prospector;
	}

	bool is_developer() const
	{
		return this->developer;
	}

	bool is_spy() const
	{
		return this->spy;
	}

	bool is_adventurer() const
	{
		return this->adventurer;
	}

	const resource_set &get_improvable_resources() const
	{
		return this->improvable_resources;
	}

	bool can_improve_resource(const resource *resource) const
	{
		return this->get_improvable_resources().contains(resource);
	}

	const pathway_set &get_buildable_pathways() const
	{
		return this->buildable_pathways;
	}

	bool can_build_pathway(const pathway *pathway) const
	{
		return this->get_buildable_pathways().contains(pathway);
	}

signals:
	void changed();

private:
	civilian_unit_class *unit_class = nullptr;
	kobold::culture *culture = nullptr;
	kobold::cultural_group *cultural_group = nullptr;
	kobold::icon *icon = nullptr;
	bool explorer = false;
	bool prospector = false;
	bool developer = false;
	bool spy = false;
	bool adventurer = false;
	resource_set improvable_resources;
	pathway_set buildable_pathways;
};

}
