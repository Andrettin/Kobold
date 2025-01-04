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
	readonly property string cultural_patron_deity_name: character && character.patron_deity ? character.patron_deity.get_cultural_name_qstring(character.culture) : ""
	readonly property string patron_deity_name: character && character.patron_deity ? (cultural_patron_deity_name + (cultural_patron_deity_name !== character.patron_deity.name ? (" (" + character.patron_deity.name + ")") : "")) : ""
	
	SmallText {
		id: description_text
		anchors.top: title_item.bottom
		anchors.topMargin: 16 * scale_factor
		anchors.left: parent.left
		anchors.leftMargin: 8 * scale_factor
		anchors.right: parent.right
		anchors.rightMargin: 8 * scale_factor
		text: format_text(character ? (
			"Species: " + character.species.name
			+ "\nClass: " + character.game_data.character_class.name
			+ "\nLevel: " + character.game_data.level
			+ (character.deity ? ("\nDivine Rank: " + character.deity.divine_rank) : "")
			+ "\nExperience: " + number_string(character.game_data.experience)
			+ "\nChallenge Rating: " + character.game_data.challenge_rating
			+ "\nAge: " + number_string(character.game_data.age)
			+ (character.deity ? ("\nPantheon: " + character.deity.pantheon.name) : (character.patron_deity ? ("\nPatron Deity: " + patron_deity_name) : ("\nReligion: " + character.religion.name)))
			+ "\nAlignment: " + (character.alignments.length == 0 ? "Neutral" : string_list_to_string(object_list_to_name_list(character.alignments), " "))
		) : "")
		wrapMode: Text.WordWrap
	}
	
	Rectangle {
		id: character_data_tree_view_top_divisor
		color: "gray"
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.top: description_text.bottom
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
		height: Math.min(192 * scale_factor, contentHeight)
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
		anchors.topMargin: 16 * scale_factor + (192 * scale_factor - character_data_tree_view.height)
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
