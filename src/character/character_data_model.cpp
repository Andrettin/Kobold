#include "kobold.h"

#include "character/character_data_model.h"

#include "character/character.h"
#include "character/character_game_data.h"
#include "character/feat.h"
#include "character/feat_type.h"
#include "character/saving_throw_type.h"
#include "character/skill.h"
#include "character/skill_category.h"
#include "util/assert_util.h"
#include "util/exception_util.h"
#include "util/map_util.h"
#include "util/vector_util.h"

namespace kobold {

character_data_model::character_data_model()
{
}

int character_data_model::rowCount(const QModelIndex &parent) const
{
	if (this->get_character() == nullptr) {
		return 0;
	}

	if (!parent.isValid()) {
		return static_cast<int>(this->top_rows.size());
	}

	if (parent.constInternalPointer() == &this->saving_throws_row) {
		return static_cast<int>(this->saving_throws.size());
	} else if (parent.constInternalPointer() == &this->skills_row) {
		return static_cast<int>(this->skills.size());
	} else if (parent.constInternalPointer() == &this->feats_row) {
		return static_cast<int>(this->feat_types.size());
	}

	const named_data_entry *parent_entry = reinterpret_cast<const named_data_entry *>(parent.constInternalPointer());
	const skill_category *parent_skill_category = dynamic_cast<const skill_category *>(parent_entry);
	if (parent_skill_category != nullptr) {
		return static_cast<int>(this->get_category_skills(parent_skill_category).size());
	}

	const feat_type *parent_feat_type = dynamic_cast<const feat_type *>(parent_entry);
	if (parent_feat_type != nullptr) {
		return static_cast<int>(this->get_feats_of_type(parent_feat_type).size());
	}

	return 0;
}

int character_data_model::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);

	return 1;
}

QVariant character_data_model::data(const QModelIndex &index, const int role) const
{
	if (!index.isValid()) {
		return QVariant();
	}

	try {
		switch (role) {
			case Qt::DisplayRole:
			{
				if (index.constInternalPointer() == &this->saving_throws_row) {
					return "Saving Throws";
				} else if (index.constInternalPointer() == &this->skills_row) {
					return "Skills";
				} else if (index.constInternalPointer() == &this->feats_row) {
					return "Feats";
				}

				const named_data_entry *entry = reinterpret_cast<const named_data_entry *>(index.constInternalPointer());
				if (entry != nullptr) {
					const saving_throw_type *saving_throw_type = dynamic_cast<const kobold::saving_throw_type *>(entry);
					if (saving_throw_type != nullptr) {
						const int saving_throw_bonus = this->get_character()->get_game_data()->get_saving_throw_bonus(saving_throw_type);
						assert_throw(saving_throw_bonus != 0);
						return QString::fromStdString(std::format("{} {}", saving_throw_type->get_name(), number::to_signed_string(saving_throw_bonus)));
					}

					const skill *skill = dynamic_cast<const kobold::skill *>(entry);
					if (skill != nullptr) {
						const int skill_bonus = this->get_character()->get_game_data()->get_skill_bonus(skill);
						assert_throw(skill_bonus != 0);
						return QString::fromStdString(std::format("{} {}", skill->get_name(), number::to_signed_string(skill_bonus)));
					}

					const feat *feat = dynamic_cast<const kobold::feat *>(entry);
					if (feat != nullptr) {
						std::string name = feat->get_name();
						const int feat_count = this->get_character()->get_game_data()->get_feat_count(feat);
						if (feat_count > 1) {
							name += std::format(" (x{})", feat_count);
						}
						return QString::fromStdString(name);
					}

					return entry->get_name_qstring();
				}

				return QString();
			}
			default:
				throw std::runtime_error(std::format("Invalid feat model role: {}.", role));
		}
	} catch (...) {
		exception::report(std::current_exception());
	}

	return QVariant();
}

QModelIndex character_data_model::index(const int row, const int column, const QModelIndex &parent) const
{
	if (!this->hasIndex(row, column, parent)) {
		return QModelIndex();
	}

	if (!parent.isValid()) {
		return this->createIndex(row, column, this->top_rows.at(row));
	}

	if (parent.constInternalPointer() == &this->saving_throws_row) {
		return this->createIndex(row, column, this->saving_throws.at(row));
	} else if (parent.constInternalPointer() == &this->skills_row) {
		return this->createIndex(row, column, this->skills.at(row));
	} else if (parent.constInternalPointer() == &this->feats_row) {
		return this->createIndex(row, column, this->feat_types.at(row));
	}

	const named_data_entry *parent_entry = reinterpret_cast<const named_data_entry *>(parent.constInternalPointer());
	const skill_category *parent_skill_category = dynamic_cast<const skill_category *>(parent_entry);
	if (parent_skill_category != nullptr) {
		return this->createIndex(row, column, this->get_category_skills(parent_skill_category).at(row));
	}

	const feat_type *parent_feat_type = dynamic_cast<const feat_type *>(parent_entry);
	if (parent_feat_type != nullptr) {
		return this->createIndex(row, column, this->get_feats_of_type(parent_feat_type).at(row));
	}

	return QModelIndex();
}

QModelIndex character_data_model::parent(const QModelIndex &index) const
{
	if (!index.isValid()) {
		return QModelIndex();
	}

	if (vector::contains(this->top_rows, index.constInternalPointer())) {
		return QModelIndex();
	}

	const named_data_entry *entry = reinterpret_cast<const named_data_entry *>(index.constInternalPointer());
	if (entry != nullptr) {
		const saving_throw_type *saving_throw_type = dynamic_cast<const kobold::saving_throw_type *>(entry);
		if (saving_throw_type != nullptr) {
			return this->createIndex(static_cast<int>(vector::find_index(this->top_rows, &this->saving_throws_row).value()), 0, &this->saving_throws_row);
		}

		const skill_category *skill_category = dynamic_cast<const kobold::skill_category *>(entry);
		const skill *skill = dynamic_cast<const kobold::skill *>(entry);
		if (skill_category != nullptr || (skill != nullptr && skill->get_category() == nullptr)) {
			return this->createIndex(static_cast<int>(vector::find_index(this->top_rows, &this->skills_row).value()), 0, &this->skills_row);
		}

		if (skill != nullptr) {
			for (size_t i = 0; i < this->skills.size(); ++i) {
				if (this->skills.at(i) == skill->get_category()) {
					return this->createIndex(static_cast<int>(i), 0, skill->get_category());
				}
			}
		}

		const feat_type *feat_type = dynamic_cast<const kobold::feat_type *>(entry);
		if (feat_type != nullptr) {
			return this->createIndex(static_cast<int>(vector::find_index(this->top_rows, &this->feats_row).value()), 0, &this->feats_row);
		}

		const feat *feat = dynamic_cast<const kobold::feat *>(entry);
		if (feat != nullptr) {
			feat_type = feat->get_types().at(0);
			for (size_t i = 0; i < this->feat_types.size(); ++i) {
				if (this->feat_types.at(i) == feat_type) {
					return this->createIndex(static_cast<int>(i), 0, feat_type);
				}
			}
		}
	}

	return QModelIndex();
}

void character_data_model::set_character(const kobold::character *character)
{
	this->beginResetModel();

	this->character = character;

	this->top_rows.clear();
	this->saving_throws.clear();
	this->skills.clear();
	this->skills_by_category.clear();
	this->feat_types.clear();
	this->feats_by_type.clear();

	if (character != nullptr) {
		this->top_rows.push_back(&this->saving_throws_row);
		this->top_rows.push_back(&this->skills_row);
		this->top_rows.push_back(&this->feats_row);

		this->saving_throws = archimedes::map::get_keys(character->get_game_data()->get_saving_throw_bonuses());

		for (const auto &[skill, bonus] : character->get_game_data()->get_skill_bonuses()) {
			if (skill->get_category() != nullptr) {
				if (!vector::contains(this->skills, skill->get_category())) {
					this->skills.push_back(skill->get_category());
				}

				this->skills_by_category[skill->get_category()].push_back(skill);
			} else {
				this->skills.push_back(skill);
			}
		}
		std::sort(this->skills.begin(), this->skills.end(), data_entry_compare<named_data_entry>());

		for (const auto &[feat, count] : character->get_game_data()->get_feat_counts()) {
			this->feats_by_type[feat->get_types().at(0)].push_back(feat);
		}
		for (const auto &[feat_type, feats] : this->feats_by_type) {
			this->feat_types.push_back(feat_type);
		}
		std::sort(this->feat_types.begin(), this->feat_types.end(), data_entry_compare<feat_type>());
	}

	this->endResetModel();
	emit character_changed();
}

const std::vector<const skill *> &character_data_model::get_category_skills(const skill_category *skill_category) const
{
	const auto find_iterator = this->skills_by_category.find(skill_category);
	if (find_iterator != this->skills_by_category.end()) {
		return find_iterator->second;
	}

	assert_throw(false);

	static const std::vector<const skill *> empty_vector;
	return empty_vector;
}

const std::vector<const feat *> &character_data_model::get_feats_of_type(const feat_type *feat_type) const
{
	const auto find_iterator = this->feats_by_type.find(feat_type);
	if (find_iterator != this->feats_by_type.end()) {
		return find_iterator->second;
	}

	assert_throw(false);

	static const std::vector<const feat *> empty_vector;
	return empty_vector;
}

}
