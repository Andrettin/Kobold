#pragma once

#include "country/culture_base.h"
#include "database/data_type.h"

namespace kobold {

class cultural_group;

template <typename scope_type>
class condition;

class culture final : public culture_base, public data_type<culture>
{
	Q_OBJECT

	Q_PROPERTY(QColor color MEMBER color READ get_color NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "culture";
	static constexpr const char property_class_identifier[] = "kobold::culture*";
	static constexpr const char database_folder[] = "cultures";
	static constexpr bool history_enabled = true;

	explicit culture(const std::string &identifier);
	~culture();

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void initialize() override;
	virtual void check() const override;

	using culture_base::get_group;

	const QColor &get_color() const
	{
		return this->color;
	}

	const std::vector<const culture *> &get_derived_cultures() const
	{
		return this->derived_cultures;
	}

signals:
	void changed();

private:
	QColor color;
	std::vector<const culture *> derived_cultures;
};

}
