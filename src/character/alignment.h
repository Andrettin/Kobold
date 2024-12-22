#pragma once

#include "database/data_type.h"
#include "database/named_data_entry.h"

namespace kobold {

class alignment_axis;

class alignment final : public named_data_entry, public data_type<alignment>
{
	Q_OBJECT

public:
	static constexpr const char class_identifier[] = "alignment";
	static constexpr const char property_class_identifier[] = "kobold::alignment*";
	static constexpr const char database_folder[] = "alignments";

	explicit alignment(const std::string &identifier) : named_data_entry(identifier)
	{
	}

	virtual void process_gsml_property(const gsml_property &property) override;
	virtual void check() const override;

	const alignment_axis *get_axis() const
	{
		return this->axis;
	}

signals:
	void changed();

private:
	const alignment_axis *axis = nullptr;
};

}
