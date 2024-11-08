#pragma once

#include "database/data_entry.h"
#include "database/data_entry_container.h"
#include "database/data_type.h"

Q_MOC_INCLUDE("ui/icon.h")

namespace kobold {

class feat;
class skill;

class skill_feat_template final : public data_entry, public data_type<skill_feat_template>
{
	Q_OBJECT

public:
	static constexpr const char class_identifier[] = "skill_feat_template";
	static constexpr const char property_class_identifier[] = "kobold::skill_feat_template*";
	static constexpr const char database_folder[] = "skill_feat_templates";

	static const std::set<std::string> database_dependencies;

	explicit skill_feat_template(const std::string &identifier);
	~skill_feat_template();

	virtual void process_gsml_property(const gsml_property &property) override;
	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void initialize() override;
	virtual void check() const override;

	static std::string process_string_for_skill(const std::string &str, const skill *skill);
	static gsml_property process_property_for_skill(const gsml_property &property, const skill *skill);
	static gsml_data process_scope_for_skill(const gsml_data &scope, const skill *skill);

signals:
	void changed();

private:
	std::vector<feat *> feats;
};

}
