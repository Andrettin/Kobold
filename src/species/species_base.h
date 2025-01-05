#pragma once

#include "species/taxon_base.h"

namespace kobold {

class character;
class species;
class subspecies;

template <typename scope_type>
class effect_list;

template <typename scope_type>
class modifier;

class species_base : public taxon_base
{
	Q_OBJECT

protected:
	explicit species_base(const std::string &identifier);
	~species_base();

public:
	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void check() const override;

	virtual const species *get_species() const = 0;

	virtual const subspecies *get_subspecies() const
	{
		return nullptr;
	}

	virtual bool is_sapient() const = 0;

	const std::vector<const culture *> &get_cultures() const
	{
		return this->cultures;
	}

	void add_culture(const culture *culture)
	{
		this->cultures.push_back(culture);
	}

	virtual const kobold::modifier<const character> *get_modifier() const
	{
		return this->modifier.get();
	}

	virtual const effect_list<const character> *get_effects() const
	{
		return this->effects.get();
	}

signals:
	void changed();

private:
	std::vector<const culture *> cultures;
	std::unique_ptr<const kobold::modifier<const character>> modifier;
	std::unique_ptr<const effect_list<const character>> effects;
};

}
