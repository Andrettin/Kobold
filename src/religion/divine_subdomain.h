#pragma once

#include "database/data_type.h"
#include "religion/divine_domain_base.h"

namespace kobold {

class divine_domain;

class divine_subdomain final : public divine_domain_base, public data_type<divine_subdomain>
{
	Q_OBJECT

	Q_PROPERTY(const kobold::divine_domain* domain MEMBER domain READ get_domain NOTIFY changed)

public:
	static constexpr const char class_identifier[] = "divine_subdomain";
	static constexpr const char property_class_identifier[] = "kobold::divine_subdomain*";
	static constexpr const char database_folder[] = "divine_subdomains";

	explicit divine_subdomain(const std::string &identifier) : divine_domain_base(identifier)
	{
	}

	const divine_domain *get_domain() const
	{
		return this->domain;
	}

signals:
	void changed();

private:
	const divine_domain *domain = nullptr;
};

}
