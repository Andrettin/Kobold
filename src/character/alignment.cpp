#include "kobold.h"

#include "character/alignment.h"

#include "character/alignment_axis.h"

namespace kobold {

void alignment::process_gsml_property(const gsml_property &property)
{
	const std::string &key = property.get_key();
	const std::string &value = property.get_value();

	if (key == "axis") {
		alignment_axis *axis = alignment_axis::get_or_add(value, this->get_module());
		axis->add_alignment(this);
		this->axis = axis;
	} else {
		data_entry::process_gsml_property(property);
	}
}

void alignment::check() const
{
	if (this->get_axis() == nullptr) {
		throw std::runtime_error(std::format("Alignment \"{}\" has no axis.", this->get_identifier()));
	}
}

}
