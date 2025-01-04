#include "kobold.h"

#include "character/feat_model.h"

#include "character/character.h"
#include "character/character_game_data.h"
#include "character/feat.h"
#include "character/feat_type.h"
#include "util/assert_util.h"
#include "util/exception_util.h"

namespace kobold {

feat_model::feat_model()
{
}

int feat_model::rowCount(const QModelIndex &parent) const
{
	if (!parent.isValid()) {
		return 1;
	}

	if (parent.constInternalPointer() == this) {
		return static_cast<int>(this->feat_types.size());
	}

	const named_data_entry *parent_entry = reinterpret_cast<const named_data_entry *>(parent.constInternalPointer());
	const feat_type *parent_feat_type = dynamic_cast<const kobold::feat_type *>(parent_entry);
	if (parent_feat_type != nullptr) {
		return static_cast<int>(this->get_feats_of_type(parent_feat_type).size());
	}

	return 0;
}

int feat_model::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);

	return 1;
}

QVariant feat_model::data(const QModelIndex &index, const int role) const
{
	if (!index.isValid()) {
		return QVariant();
	}

	try {
		switch (role) {
			case Qt::DisplayRole:
			{
				if (!index.parent().isValid()) {
					return "Feats";
				}

				const QModelIndex grandparent_index = index.parent().parent();
				const bool is_feat_type = !grandparent_index.isValid();

				if (is_feat_type) {
					const feat_type *feat_type = reinterpret_cast<const kobold::feat_type *>(index.constInternalPointer());
					return feat_type->get_name_qstring();
				}

				const feat *feat = reinterpret_cast<const kobold::feat *>(index.constInternalPointer());
				std::string name = feat->get_name();
				const int feat_count = this->get_character()->get_game_data()->get_feat_count(feat);
				if (feat_count > 1) {
					name += std::format(" (x{})", feat_count);
				}
				return QString::fromStdString(name);
			}
			default:
				throw std::runtime_error(std::format("Invalid feat model role: {}.", role));
		}
	} catch (...) {
		exception::report(std::current_exception());
	}

	return QVariant();
}

QModelIndex feat_model::index(const int row, const int column, const QModelIndex &parent) const
{
	if (!this->hasIndex(row, column, parent)) {
		return QModelIndex();
	}

	if (!parent.isValid()) {
		return this->createIndex(row, column, this);
	}

	const bool is_feat_type = parent.constInternalPointer() == this;
	if (is_feat_type) {
		return this->createIndex(row, column, this->feat_types.at(row));
	} else {
		return this->createIndex(row, column, this->get_feats_of_type(reinterpret_cast<const feat_type *>(parent.constInternalPointer())).at(row));
	}
}

QModelIndex feat_model::parent(const QModelIndex &index) const
{
	if (!index.isValid()) {
		return QModelIndex();
	}

	const named_data_entry *entry = reinterpret_cast<const named_data_entry *>(index.constInternalPointer());
	if (entry != nullptr) {
		const feat_type *feat_type = dynamic_cast<const kobold::feat_type *>(entry);
		if (feat_type != nullptr) {
			return this->createIndex(0, 0, this);
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

void feat_model::set_character(const kobold::character *character)
{
	this->beginResetModel();

	this->character = character;

	this->feat_types.clear();
	this->feats_by_type.clear();

	if (character != nullptr) {
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

const std::vector<const feat *> &feat_model::get_feats_of_type(const feat_type *feat_type) const
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
