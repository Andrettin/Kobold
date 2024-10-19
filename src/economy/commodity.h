#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"

namespace kobold {

class icon;

class commodity final : public named_data_entry, public data_type<commodity>
{
	Q_OBJECT

	Q_PROPERTY(kobold::icon* icon MEMBER icon NOTIFY changed)
	Q_PROPERTY(bool abstract MEMBER abstract READ is_abstract NOTIFY changed)
	Q_PROPERTY(bool storable MEMBER storable READ is_storable NOTIFY changed)
	Q_PROPERTY(bool local MEMBER local READ is_local NOTIFY changed)
	Q_PROPERTY(bool negative_allowed MEMBER negative_allowed READ is_negative_allowed NOTIFY changed)
	Q_PROPERTY(bool labor MEMBER labor READ is_labor NOTIFY changed)
	Q_PROPERTY(int wealth_value MEMBER wealth_value READ get_wealth_value NOTIFY changed)
	Q_PROPERTY(int base_price MEMBER base_price READ get_base_price NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "commodity";
	static constexpr const char property_class_identifier[] = "kobold::commodity*";
	static constexpr const char database_folder[] = "commodities";

	static constexpr int abstract_commodity_value = 100; //used for e.g. score calculation

	explicit commodity(const std::string &identifier);

	virtual void check() const override;

	const kobold::icon *get_icon() const
	{
		return this->icon;
	}

	bool is_abstract() const
	{
		return this->abstract;
	}

	bool is_storable() const
	{
		return this->storable;
	}

	bool is_local() const
	{
		return this->local;
	}

	bool is_negative_allowed() const
	{
		return this->negative_allowed;
	}

	bool is_labor() const
	{
		return this->labor;
	}

	int get_wealth_value() const
	{
		return this->wealth_value;
	}

	bool is_convertible_to_wealth() const
	{
		return this->get_wealth_value() != 0;
	}

	int get_base_price() const
	{
		return this->base_price;
	}

	bool is_tradeable() const
	{
		return !this->is_abstract() && this->is_storable() && !this->is_convertible_to_wealth();
	}

signals:
	void changed();

private:
	kobold::icon *icon = nullptr;
	bool abstract = false;
	bool storable = true;
	bool local = false;
	bool negative_allowed = false;
	bool labor = false;
	int wealth_value = 0;
	int base_price = 0;
};

}
