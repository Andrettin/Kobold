#pragma once

#include "database/data_entry_container.h"

#pragma warning(push, 0)
#include <QAbstractItemModel> 
#pragma warning(pop)

namespace archimedes {
	class named_data_entry;
}

namespace kobold {

class feat;
class feat_type;
class item_type;
class saving_throw_type;
class skill;
class skill_category;

class character_data_model : public QAbstractItemModel
{
	Q_OBJECT

	Q_PROPERTY(const kobold::character* character READ get_character WRITE set_character NOTIFY character_changed)

public:
	struct character_data_row final
	{
		explicit character_data_row(const std::string &name, const std::string &value = "", const character_data_row *parent_row = nullptr)
			: name(name), value(value), parent_row(parent_row)
		{
		}

		std::string name;
		std::string value;
		const character_data_row *parent_row = nullptr;
		std::vector<std::unique_ptr<character_data_row>> child_rows;
	};

	character_data_model();

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

	void create_attribute_rows();
	void create_attack_bonus_rows();
	void create_saving_throw_rows();
	void create_skill_rows();
	void create_feat_rows();

signals:
	void character_changed();

private:
	const kobold::character *character = nullptr;
	std::vector<std::unique_ptr<const character_data_row>> top_rows;
};

}
