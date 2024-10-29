#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"

namespace kobold {

class character;
class feat;

template <typename scope_type>
class and_condition;

class feat_type final : public named_data_entry, public data_type<feat_type>
{
	Q_OBJECT

	Q_PROPERTY(int max_feats MEMBER max_feats READ get_max_feats NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "feat_type";
	static constexpr const char property_class_identifier[] = "kobold::feat_type*";
	static constexpr const char database_folder[] = "feat_types";

	explicit feat_type(const std::string &identifier);
	~feat_type();

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void check() const override;

	int get_max_feats() const
	{
		return this->max_feats;
	}

	const and_condition<character> *get_gain_conditions() const
	{
		return this->gain_conditions.get();
	}

	const std::vector<const feat *> &get_feats() const
	{
		return this->feats;
	}

	void add_feat(const feat *feat)
	{
		this->feats.push_back(feat);
	}

signals:
	void changed();

private:
	int max_feats = 0; //the maximum amount of feats of this type a character can acquire
	std::unique_ptr<const and_condition<character>> gain_conditions;
	std::vector<const feat *> feats;
};

}
