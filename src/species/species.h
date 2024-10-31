#pragma once

#include "database/data_type.h"
#include "species/taxon_base.h"
#include "util/dice.h"

namespace kobold {

class taxon;
enum class taxonomic_rank;

template <typename scope_type>
class effect_list;

template <typename scope_type>
class modifier;

class species final : public taxon_base, public data_type<species>
{
	Q_OBJECT

	Q_PROPERTY(bool sapient MEMBER sapient READ is_sapient NOTIFY changed)
	Q_PROPERTY(archimedes::dice hit_dice MEMBER hit_dice READ get_hit_dice NOTIFY changed)
	Q_PROPERTY(int level_adjustment MEMBER level_adjustment READ get_level_adjustment NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "species";
	static constexpr const char property_class_identifier[] = "kobold::species*";
	static constexpr const char database_folder[] = "species";

	explicit species(const std::string &identifier);
	~species();

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void check() const override;

	virtual taxonomic_rank get_rank() const override;

	virtual const std::string &get_common_name() const override
	{
		return this->get_name();
	}

	bool is_sapient() const
	{
		return this->sapient;
	}

	const dice &get_hit_dice() const
	{
		return this->hit_dice;
	}

	int get_level_adjustment() const
	{
		return this->level_adjustment;
	}

	const kobold::modifier<const character> *get_modifier() const
	{
		return this->modifier.get();
	}

	const effect_list<const character> *get_effects() const
	{
		return this->effects.get();
	}

private:
	bool sapient = false;
	dice hit_dice;
	int level_adjustment = 0;
	std::unique_ptr<const kobold::modifier<const character>> modifier;
	std::unique_ptr<const effect_list<const character>> effects;
};

}
