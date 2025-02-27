import QtQuick
import QtQuick.Controls
import ".."

ModifierDialog {
	id: tradition_dialog
	title: tradition ? tradition.name : ""
	type_string: tradition ? tradition.category_name : ""
	requirements_string: tradition ? tradition.get_requirements_string(kobold.game.player_country) : ""
	modifier_string: tradition ? tradition.get_modifier_string(kobold.game.player_country) : ""
	
	property var tradition: null
}
