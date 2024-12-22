#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"

namespace kobold {

class alignment;

class alignment_axis final : public named_data_entry, public data_type<alignment_axis>
{
	Q_OBJECT

public:
	static constexpr const char class_identifier[] = "alignment_axis";
	static constexpr const char property_class_identifier[] = "kobold::alignment_axis*";
	static constexpr const char database_folder[] = "alignment_axes";

	explicit alignment_axis(const std::string &identifier) : named_data_entry(identifier)
	{
	}

	virtual void check() const override;

	const std::vector<const alignment *> &get_alignments() const
	{
		return this->alignments;
	}

	void add_alignment(const alignment *alignment)
	{
		this->alignments.push_back(alignment);
	}

signals:
	void changed();

private:
	std::vector<const alignment *> alignments;
};

}
