#pragma once

#include "database/data_entry_container.h"

#pragma warning(push, 0)
#include <QAbstractItemModel> 
#pragma warning(pop)

namespace kobold {

class feat;
class feat_type;

class feat_model : public QAbstractItemModel
{
	Q_OBJECT

	Q_PROPERTY(const kobold::character* character READ get_character WRITE set_character NOTIFY character_changed)

public:
	feat_model();

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override final;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override final;
	virtual QVariant data(const QModelIndex &index, int role) const override final;
	virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override final;
	virtual QModelIndex parent(const QModelIndex &index) const override final;

	const kobold::character *get_character() const
	{
		return this->character;
	}

	void set_character(const kobold::character *character);

	const std::vector<const feat *> &get_feats_of_type(const feat_type *feat_type) const;

signals:
	void character_changed();

private:
	const kobold::character *character = nullptr;
	std::vector<const feat_type *> feat_types;
	data_entry_map<feat_type, std::vector<const feat *>> feats_by_type;
};

}
