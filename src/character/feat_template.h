#pragma once

#include "database/data_entry.h"
#include "database/data_entry_container.h"
#include "database/data_type.h"

Q_MOC_INCLUDE("ui/icon.h")

namespace kobold {

class feat;
class skill;
enum class feat_template_type;

class feat_template final : public data_entry, public data_type<feat_template>
{
	Q_OBJECT

public:
	static constexpr const char class_identifier[] = "feat_template";
	static constexpr const char property_class_identifier[] = "kobold::feat_template*";
	static constexpr const char database_folder[] = "feat_templates";

	static const std::set<std::string> database_dependencies;

	explicit feat_template(const std::string &identifier);
	~feat_template();

	virtual void process_gsml_property(const gsml_property &property) override;
	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void initialize() override;
	virtual void check() const override;

	feat_template_type get_template_type() const
	{
		return this->template_type;
	}

	std::string process_string_for_object(const std::string &str, const named_data_entry *object) const;
	gsml_property process_property_for_object(const gsml_property &property, const named_data_entry *object) const;
	gsml_data process_scope_for_object(const gsml_data &scope, const named_data_entry *object) const;

signals:
	void changed();

private:
	feat_template_type template_type{};
	std::vector<feat *> feats;
	std::vector<country_feat *> country_feats;
};

}
