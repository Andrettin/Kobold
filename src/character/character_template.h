#pragma once

#include "database/data_entry_container.h"
#include "database/data_type.h"
#include "database/named_data_entry.h"

Q_MOC_INCLUDE("species/species.h")

namespace kobold {

class species;

class character_template final : public named_data_entry, public data_type<character_template>
{
	Q_OBJECT

	Q_PROPERTY(kobold::species* species MEMBER species NOTIFY changed)
	Q_PROPERTY(int level MEMBER level READ get_level NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "character_template";
	static constexpr const char property_class_identifier[] = "kobold::character_template*";
	static constexpr const char database_folder[] = "character_templates";

	explicit character_template(const std::string &identifier);
	~character_template();

	virtual void process_gsml_property(const gsml_property &property) override;
	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void initialize() override;
	virtual void check() const override;

	const kobold::species *get_species() const
	{
		return this->species;
	}

	const std::map<character_class_type, const character_class *> &get_character_classes() const
	{
		return this->character_classes;
	}

	const character_class *get_character_class(const character_class_type type) const
	{
		const auto find_iterator = this->character_classes.find(type);

		if (find_iterator != this->character_classes.end()) {
			return find_iterator->second;
		}

		return nullptr;
	}

	int get_level() const
	{
		return this->level;
	}

	const std::vector<const feat *> &get_feats() const
	{
		return this->feats;
	}

signals:
	void changed();

private:
	kobold::species *species = nullptr;
	std::map<character_class_type, const character_class *> character_classes;
	int level = 0;
	std::vector<const feat *> feats;
};

}
