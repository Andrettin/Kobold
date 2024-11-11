#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"

namespace kobold {

class country;
class country_feat;

template <typename scope_type>
class and_condition;

template <typename scope_type>
class effect_list;

class country_feat_type final : public named_data_entry, public data_type<country_feat_type>
{
	Q_OBJECT

	Q_PROPERTY(int max_feats MEMBER max_feats READ get_max_feats NOTIFY changed)
	Q_PROPERTY(bool ignore_other_type_conditions MEMBER ignore_other_type_conditions READ ignores_other_type_conditions NOTIFY changed)
	Q_PROPERTY(bool prioritize_unacquired_feats MEMBER prioritize_unacquired_feats READ prioritizes_unacquired_feats NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "country_feat_type";
	static constexpr const char property_class_identifier[] = "kobold::country_feat_type*";
	static constexpr const char database_folder[] = "country_feat_types";

	explicit country_feat_type(const std::string &identifier);
	~country_feat_type();

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void check() const override;

	int get_max_feats() const
	{
		return this->max_feats;
	}

	bool ignores_other_type_conditions() const
	{
		return this->ignore_other_type_conditions;
	}

	bool prioritizes_unacquired_feats() const
	{
		return this->prioritize_unacquired_feats;
	}

	const and_condition<country> *get_gain_conditions() const
	{
		return this->gain_conditions.get();
	}

	const effect_list<const country> *get_effects() const
	{
		return this->effects.get();
	}

	const std::vector<const country_feat *> &get_feats() const
	{
		return this->feats;
	}

	void add_feat(const country_feat *feat)
	{
		this->feats.push_back(feat);
	}

signals:
	void changed();

private:
	int max_feats = 0; //the maximum amount of feats of this type a character can acquire
	bool ignore_other_type_conditions = false;
	bool prioritize_unacquired_feats = true;
	std::unique_ptr<const and_condition<country>> gain_conditions;
	std::unique_ptr<const effect_list<const country>> effects;
	std::vector<const country_feat *> feats;
};

}
