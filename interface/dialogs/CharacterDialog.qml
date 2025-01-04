import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import character_data_model 1.0
import ".."

DialogBase {
	id: character_dialog
	width: 320 * scale_factor
	height: ok_button.y + ok_button.height + 8 * scale_factor
	title: character ? character.game_data.titled_name : ""
	
	property var character: null
	
	Rectangle {
		id: character_data_tree_view_top_divisor
		color: "gray"
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.top: title_item.bottom
		anchors.topMargin: 16 * scale_factor
		height: 1 * scale_factor
	}
	
	TreeView {
		id: character_data_tree_view
		anchors.left: parent.left
		anchors.leftMargin: 1 * scale_factor
		anchors.right: parent.right
		anchors.rightMargin: 1 * scale_factor
		anchors.top: character_data_tree_view_top_divisor.bottom
		height: Math.min(256 * scale_factor, contentHeight)
		boundsBehavior: Flickable.StopAtBounds
		clip: true
		model: CharacterDataModel {
			character: character_dialog.character
			
			onCharacterChanged: {
				character_data_tree_view.contentY = 0
			}
		}
		delegate: TreeViewDelegate {
			implicitWidth: character_data_tree_view.width
			font.family: berenika_font.name
			font.pixelSize: 10 * scale_factor
			Material.background: "transparent"
			Material.theme: Material.Dark
		}
	}
	
	Rectangle {
		id: character_data_tree_view_bottom_divisor
		color: "gray"
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.top: character_data_tree_view.bottom
		height: 1 * scale_factor
	}
	
	TextButton {
		id: ok_button
		anchors.top: character_data_tree_view_bottom_divisor.bottom
		anchors.topMargin: 16 * scale_factor + (256 * scale_factor - character_data_tree_view.height)
		anchors.horizontalCenter: parent.horizontalCenter
		text: "OK"
		onClicked: {
			character_dialog.close()
		}
	}
	
	onClosed: {
		character = null
	}
}
