#include "kobold.h"

#include "character/alignment_axis.h"

namespace kobold {

void alignment_axis::check() const
{
	if (this->get_alignments().empty()) {
		throw std::runtime_error(std::format("Alignment axis \"{}\" has no alignments.", this->get_identifier()));
	}
}

}
