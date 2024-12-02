#pragma once

#include "database/named_data_entry.h"
#include "language/name_variant.h"
#include "util/dice.h"

namespace archimedes {
	class gendered_name_generator;
	class name_generator;
	enum class gender;
}

namespace kobold {

enum class starting_age_category;

class species_base : public named_data_entry
{
	Q_OBJECT

	Q_PROPERTY(int adulthood_age MEMBER adulthood_age NOTIFY changed)
	Q_PROPERTY(int middle_age MEMBER middle_age NOTIFY changed)
	Q_PROPERTY(int old_age MEMBER old_age NOTIFY changed)
	Q_PROPERTY(int venerable_age MEMBER venerable_age NOTIFY changed)
	Q_PROPERTY(archimedes::dice maximum_age_modifier MEMBER maximum_age_modifier NOTIFY changed)

protected:
	explicit species_base(const std::string &identifier);
	~species_base();

public:
	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void initialize() override;

	virtual species_base *get_supertaxon() const = 0;;

	int get_adulthood_age() const
	{
		if (this->adulthood_age != 0) {
			return this->adulthood_age;
		}

		if (this->get_supertaxon() != nullptr) {
			return this->get_supertaxon()->get_adulthood_age();
		}

		return this->adulthood_age;
	}

	int get_middle_age() const
	{
		if (this->middle_age != 0) {
			return this->middle_age;
		}

		if (this->get_supertaxon() != nullptr) {
			return this->get_supertaxon()->get_middle_age();
		}

		return this->middle_age;
	}

	int get_old_age() const
	{
		if (this->old_age != 0) {
			return this->old_age;
		}

		if (this->get_supertaxon() != nullptr) {
			return this->get_supertaxon()->get_old_age();
		}

		return this->old_age;
	}

	int get_venerable_age() const
	{
		if (this->venerable_age != 0) {
			return this->venerable_age;
		}

		if (this->get_supertaxon() != nullptr) {
			return this->get_supertaxon()->get_venerable_age();
		}

		return this->venerable_age;
	}

	const dice &get_maximum_age_modifier() const
	{
		if (!this->maximum_age_modifier.is_null()) {
			return this->maximum_age_modifier;
		}

		if (this->get_supertaxon() != nullptr) {
			return this->get_supertaxon()->get_maximum_age_modifier();
		}

		return this->maximum_age_modifier;
	}

	const dice &get_starting_age_modifier(const starting_age_category category) const;

	const name_generator *get_specimen_name_generator(const gender gender) const;

	void add_specimen_name(const gender gender, const name_variant &name);
	void add_specimen_names_from(const species_base *other);

signals:
	void changed();

private:
	int adulthood_age = 0;
	int middle_age = 0;
	int old_age = 0;
	int venerable_age = 0;
	dice maximum_age_modifier;
	std::map<starting_age_category, dice> starting_age_modifiers;
	std::unique_ptr<gendered_name_generator> specimen_name_generator; //specimen names, mapped to the gender they pertain to (use gender::none for names which should be available for both genders)
};

}
