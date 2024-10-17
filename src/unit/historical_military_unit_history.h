#pragma once

#include "database/data_entry_history.h"

Q_MOC_INCLUDE("map/province.h")

namespace kobold {

class promotion;
class province;

class historical_military_unit_history final : public data_entry_history
{
	Q_OBJECT

	Q_PROPERTY(kobold::province* province MEMBER province)
	Q_PROPERTY(std::vector<kobold::promotion *> promotions READ get_promotions)

public:
	const kobold::province *get_province() const
	{
		return this->province;
	}

	bool is_active() const
	{
		return this->get_province() != nullptr;
	}

	const std::vector<promotion *> &get_promotions() const
	{
		return this->promotions;
	}

	Q_INVOKABLE void add_promotion(kobold::promotion *promotion)
	{
		this->promotions.push_back(promotion);
	}

	Q_INVOKABLE void remove_promotion(kobold::promotion *promotion)
	{
		std::erase(this->promotions, promotion);
	}

private:
	kobold::province *province = nullptr;
	std::vector<promotion *> promotions;
};

}
