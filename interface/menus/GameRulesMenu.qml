import QtQuick
import QtQuick.Controls
import ".."

MenuBase {
	id: game_rules_menu
	title: qsTr("Game Rules")
	//background: kobold.defines.default_menu_background_file
	
	property var selected_scenario: null
	property bool rules_changed: false
	
	CustomCheckBox {
		id: myths_enabled_checkbox
		anchors.left: parent.left
		anchors.leftMargin: 16 * scale_factor
		anchors.top: title_item.bottom
		anchors.topMargin: 32 * scale_factor
		text: colored_text(qsTr("Myths Enabled"), "white")
		checked: kobold.preferences.game_rules.myths_enabled
		checkable: true
		tooltip: small_text("Enable mythical elements in the game")
		onCheckedChanged: {
			if (kobold.preferences.game_rules.myths_enabled !== checked) {
				kobold.preferences.game_rules.myths_enabled = checked
				rules_changed = true
			}
		}
	}
	
	TextButton {
		id: previous_menu_button
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.bottom: parent.bottom
		anchors.bottomMargin: 16 * scale_factor
		text: qsTr("Previous Menu")
		width: 96 * scale_factor
		height: 24 * scale_factor
		
		onClicked: {
			if (rules_changed) {
				kobold.preferences.save()
				kobold.game.setup_scenario(selected_scenario).then(() => {
					menu_stack.pop()
				})
			} else {
				menu_stack.pop()
			}
		}
	}
}
