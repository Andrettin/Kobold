#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"

Q_MOC_INCLUDE("character/character.h")
Q_MOC_INCLUDE("religion/pantheon.h")

namespace kobold {

class character;
class divine_domain;
class pantheon;
class religion;

class deity final : public named_data_entry, public data_type<deity>
{
	Q_OBJECT

	Q_PROPERTY(const kobold::pantheon* pantheon MEMBER pantheon READ get_pantheon NOTIFY changed)
	Q_PROPERTY(const kobold::character* character READ get_character CONSTANT)
	Q_PROPERTY(int divine_rank READ get_divine_rank CONSTANT)

public:
	static constexpr const char class_identifier[] = "deity";
	static constexpr const char property_class_identifier[] = "kobold::deity*";
	static constexpr const char database_folder[] = "deities";
	static constexpr int base_deity_domains = 3;

	static const std::set<std::string> database_dependencies;

	static deity *add(const std::string &identifier, const kobold::data_module *data_module);

	explicit deity(const std::string &identifier) : named_data_entry(identifier)
	{
	}

	virtual void process_gsml_property(const gsml_property &scope) override;
	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void initialize() override;
	virtual void check() const override;

	const kobold::pantheon *get_pantheon() const
	{
		return this->pantheon;
	}

	const std::vector<const religion *> &get_religions() const
	{
		return this->religions;
	}

	const kobold::character *get_character() const
	{
		return this->character;
	}

	int get_divine_rank() const
	{
		return this->divine_rank;
	}

	const std::vector<const divine_domain *> &get_domains() const
	{
		return this->domains;
	}

signals:
	void changed();

private:
	const kobold::pantheon *pantheon = nullptr;
	std::vector<const religion *> religions;
	kobold::character *character = nullptr;
	int divine_rank = 0;
	std::vector<const divine_domain *> domains;
};

}
