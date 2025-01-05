#pragma once

#include "species/taxon_base.h"

namespace kobold {

class character;

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
	std::unique_ptr<const kobold::modifier<const character>> modifier;
	std::unique_ptr<const effect_list<const character>> effects;
};

}
