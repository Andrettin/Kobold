#include "kobold.h"

#include "economy/transaction.h"

#include "economy/commodity.h"
#include "util/assert_util.h"
#include "util/number_util.h"
#include "util/string_util.h"

namespace kobold {

const std::string &transaction::get_object_name() const
{
	if (std::holds_alternative<const commodity *>(this->get_object())) {
		return std::get<const commodity *>(this->get_object())->get_name();
	} else {
		assert_throw(false);
	}

	return string::empty_str;
}

QString transaction::get_name() const
{
	return QString::fromStdString(std::format("{} ${}", this->get_object_name(), number::to_formatted_string(this->get_amount() / (this->get_object_quantity() != 0 ? this->get_object_quantity() : 1))));
}

const icon *transaction::get_icon() const
{
	if (std::holds_alternative<const commodity *>(this->get_object())) {
		return std::get<const commodity *>(this->get_object())->get_icon();
	} else {
		assert_throw(false);
	}

	return nullptr;
}

}
