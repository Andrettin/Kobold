#include "kobold.h"

#include "character/character_data_model.h"

#include "character/alignment.h"
#include "character/character.h"
#include "character/character_attribute.h"
#include "character/character_class.h"
#include "character/character_game_data.h"
#include "character/feat.h"
#include "character/feat_type.h"
#include "character/saving_throw_type.h"
#include "character/skill.h"
#include "character/skill_category.h"
#include "item/item_type.h"
#include "religion/deity.h"
#include "religion/pantheon.h"
#include "religion/religion.h"
#include "species/species.h"
#include "species/subspecies.h"
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

	const character_data_row *parent_row_data = reinterpret_cast<const character_data_row *>(parent.constInternalPointer());
	return static_cast<int>(parent_row_data->child_rows.size());
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
				const character_data_row *row_data = reinterpret_cast<const character_data_row *>(index.constInternalPointer());

				if (!row_data->value.empty()) {
					return QString::fromStdString(std::format("{} {}", row_data->name, row_data->value));
				} else {
					return QString::fromStdString(row_data->name);
				}
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
		return this->createIndex(row, column, this->top_rows.at(row).get());
	}

	const character_data_row *parent_row_data = reinterpret_cast<const character_data_row *>(parent.constInternalPointer());
	return this->createIndex(row, column, parent_row_data->child_rows.at(row).get());
}

QModelIndex character_data_model::parent(const QModelIndex &index) const
{
	if (!index.isValid()) {
		return QModelIndex();
	}

	const character_data_row *row_data = reinterpret_cast<const character_data_row *>(index.constInternalPointer());
	if (row_data->parent_row == nullptr) {
		return QModelIndex();
	}

	if (row_data->parent_row->parent_row == nullptr) {
		for (size_t i = 0; i < this->top_rows.size(); ++i) {
			if (this->top_rows.at(i).get() == row_data->parent_row) {
				return this->createIndex(static_cast<int>(i), 0, row_data->parent_row);
			}
		}

		assert_throw(false);
	}

	for (size_t i = 0; i < row_data->parent_row->parent_row->child_rows.size(); ++i) {
		if (row_data->parent_row->parent_row->child_rows.at(i).get() == row_data->parent_row) {
			return this->createIndex(static_cast<int>(i), 0, row_data->parent_row);
		}
	}
	assert_throw(false);

	return QModelIndex();
}

void character_data_model::set_character(const kobold::character *character)
{
	this->beginResetModel();

	this->character = character;

	this->top_rows.clear();

	if (character != nullptr) {
		const character_game_data *character_game_data = this->get_character()->get_game_data();

		this->top_rows.push_back(std::make_unique<character_data_row>("Species:", character->get_species()->get_name()));

		if (character->get_subspecies() != nullptr) {
			this->top_rows.push_back(std::make_unique<character_data_row>("Subspecies:", character->get_subspecies()->get_name()));
		}

		if (character_game_data->get_character_class() != nullptr) {
			this->top_rows.push_back(std::make_unique<character_data_row>("Class:", character_game_data->get_character_class()->get_name()));
		}

		this->top_rows.push_back(std::make_unique<character_data_row>("Level:", std::to_string(character_game_data->get_level())));

		if (character->is_deity()) {
			this->top_rows.push_back(std::make_unique<character_data_row>("Divine Rank:", std::to_string(character->get_deity()->get_divine_rank())));
		}

		this->top_rows.push_back(std::make_unique<character_data_row>("Experience:", number::to_formatted_string(character_game_data->get_experience())));

		this->top_rows.push_back(std::make_unique<character_data_row>("Challenge Rating:", character_game_data->get_challenge_rating().to_string()));

		this->top_rows.push_back(std::make_unique<character_data_row>("Age:", number::to_formatted_string(character_game_data->get_age())));

		if (character->is_deity()) {
			this->top_rows.push_back(std::make_unique<character_data_row>("Pantheon:", character->get_deity()->get_pantheon()->get_name()));
		} else if (character->get_patron_deity() != nullptr) {
			std::string patron_deity_name = character->get_patron_deity()->get_cultural_name(character->get_culture());
			
			if (patron_deity_name != character->get_patron_deity()->get_name()) {
				patron_deity_name += std::format(" ({})", character->get_patron_deity()->get_name());
			}
			
			this->top_rows.push_back(std::make_unique<character_data_row>("Patron Deity:", patron_deity_name));
		} else {
			this->top_rows.push_back(std::make_unique<character_data_row>("Religion:", character->get_religion()->get_name()));
		}

		this->create_alignment_row();
		this->create_attribute_rows();

		this->top_rows.push_back(std::make_unique<character_data_row>("Hit Points:", std::to_string(character_game_data->get_hit_points())));

		this->top_rows.push_back(std::make_unique<character_data_row>("Armor Class:", std::to_string(character_game_data->get_armor_class())));

		this->create_attack_bonus_rows();
		this->create_saving_throw_rows();
		this->create_skill_rows();
		this->create_feat_rows();
	}

	this->endResetModel();
	emit character_changed();
}

void character_data_model::create_alignment_row()
{
	std::string alignments_string;

	const std::vector<const alignment *> alignments = this->get_character()->get_alignments();
	for (const alignment *alignment : alignments) {
		if (!alignments_string.empty()) {
			alignments_string += " ";
		}

		alignments_string += alignment->get_name();
	}

	if (alignments_string.empty()) {
		alignments_string = "Neutral";
	}

	this->top_rows.push_back(std::make_unique<character_data_row>("Alignment:", alignments_string));
}

void character_data_model::create_attribute_rows()
{
	auto top_row = std::make_unique<character_data_row>("Attributes");

	const character_game_data *character_game_data = this->get_character()->get_game_data();
	for (const auto &[attribute, value] : character_game_data->get_attribute_values()) {
		auto row = std::make_unique<character_data_row>(attribute->get_name(), std::to_string(value), top_row.get());
		top_row->child_rows.push_back(std::move(row));
	}

	this->top_rows.push_back(std::move(top_row));
}

void character_data_model::create_attack_bonus_rows()
{
	const character_game_data *character_game_data = this->get_character()->get_game_data();

	auto attack_bonus_row = std::make_unique<character_data_row>("Attack Bonus:", number::to_signed_string(character_game_data->get_attack_bonus()));

	auto base_attack_bonus_row = std::make_unique<character_data_row>("Base Attack Bonus:", number::to_signed_string(character_game_data->get_base_attack_bonus()), attack_bonus_row.get());
	attack_bonus_row->child_rows.push_back(std::move(base_attack_bonus_row));

	for (const auto &[weapon_type, bonus] : character_game_data->get_weapon_type_attack_bonuses()) {
		auto row = std::make_unique<character_data_row>(weapon_type->get_name(), number::to_signed_string(character_game_data->get_attack_bonus() + bonus), attack_bonus_row.get());
		attack_bonus_row->child_rows.push_back(std::move(row));
	}

	this->top_rows.push_back(std::move(attack_bonus_row));
}

void character_data_model::create_saving_throw_rows()
{
	auto top_row = std::make_unique<character_data_row>("Saving Throws");

	const character_game_data *character_game_data = this->get_character()->get_game_data();
	for (const auto &[saving_throw_type, bonus] : character_game_data->get_saving_throw_bonuses()) {
		auto row = std::make_unique<character_data_row>(saving_throw_type->get_name(), number::to_signed_string(bonus), top_row.get());
		top_row->child_rows.push_back(std::move(row));
	}

	this->top_rows.push_back(std::move(top_row));
}

void character_data_model::create_skill_rows()
{
	auto top_row = std::make_unique<character_data_row>("Skills");

	const character_game_data *character_game_data = this->get_character()->get_game_data();
	data_entry_map<skill_category, character_data_row *> skill_category_rows;
	for (const auto &[skill, bonus] : character_game_data->get_skill_bonuses()) {
		character_data_row *parent_row = top_row.get();

		if (skill->get_category() != nullptr) {
			const auto find_iterator = skill_category_rows.find(skill->get_category());
			if (find_iterator != skill_category_rows.end()) {
				parent_row = find_iterator->second;
			} else {
				auto category_row = std::make_unique<character_data_row>(skill->get_category()->get_name(), "", top_row.get());
				parent_row = category_row.get();
				skill_category_rows[skill->get_category()] = category_row.get();
				top_row->child_rows.push_back(std::move(category_row));
			}
		}

		auto row = std::make_unique<character_data_row>(skill->get_name(), number::to_signed_string(bonus), parent_row);
		parent_row->child_rows.push_back(std::move(row));
	}

	this->top_rows.push_back(std::move(top_row));
}

void character_data_model::create_feat_rows()
{
	auto top_row = std::make_unique<character_data_row>("Feats");

	const character_game_data *character_game_data = this->get_character()->get_game_data();
	data_entry_map<feat_type, std::vector<const feat *>> feats_by_type;

	for (const auto &[feat, count] : character->get_game_data()->get_feat_counts()) {
		feats_by_type[feat->get_types().at(0)].push_back(feat);
	}

	for (const auto &[feat_type, feats] : feats_by_type) {
		auto feat_type_row = std::make_unique<character_data_row>(feat_type->get_name(), "", top_row.get());

		for (const feat *feat : feats) {
			const int feat_count = character_game_data->get_feat_count(feat);
			auto row = std::make_unique<character_data_row>(feat->get_name(), feat_count > 1 ? std::format("(x{})", feat_count) : "", feat_type_row.get());
			feat_type_row->child_rows.push_back(std::move(row));
		}

		top_row->child_rows.push_back(std::move(feat_type_row));
	}

	this->top_rows.push_back(std::move(top_row));
}

}
