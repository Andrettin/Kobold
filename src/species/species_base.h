#pragma once

#include "database/named_data_entry.h"
#include "language/name_variant.h"

namespace archimedes {
	class gendered_name_generator;
	class name_generator;
	enum class gender;
}

namespace kobold {

class species_base : public named_data_entry
{
	Q_OBJECT

protected:
	explicit species_base(const std::string &identifier);
	~species_base();

public:
	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void initialize() override;

	virtual std::vector<species_base *> get_supertaxons() const
	{
		return std::vector<species_base *>();
	}

	const name_generator *get_specimen_name_generator(const gender gender) const;

	void add_specimen_name(const gender gender, const name_variant &name);
	void add_specimen_names_from(const species_base *other);

signals:
	void changed();

private:
	std::unique_ptr<gendered_name_generator> specimen_name_generator; //specimen names, mapped to the gender they pertain to (use gender::none for names which should be available for both genders)
};

}
