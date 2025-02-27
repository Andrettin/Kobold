#pragma once

#include "economy/transaction.h"
#include "util/fractional_int.h"

namespace kobold {

enum class income_transaction_type;

class income_transaction final : public transaction
{
	Q_OBJECT

public:
	explicit income_transaction(const income_transaction_type type, const int amount, const object_variant &object, const int object_quantity, const kobold::country *country)
		: transaction(amount, object, object_quantity, country), type(type)
	{
	}

	income_transaction_type get_type() const
	{
		return this->type;
	}

	virtual QString get_name() const override;
	virtual const icon *get_icon() const override;
	virtual QString get_description() const override;

private:
	income_transaction_type type{};
};

}
