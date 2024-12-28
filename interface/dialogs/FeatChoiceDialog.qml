import QtQuick
import QtQuick.Controls
import ".."

DialogBase {
	id: feat_choice_dialog
	title: feat_type && feat_type.identifier !== "general" ? ("Choose " + feat_type.name + " Feat") : "Choose Feat"
	width: 256 * scale_factor
	height: content_height
	
	readonly property int content_height: feat_button_column_view.y + feat_button_column_view.height + 8 * scale_factor
	
	property var character: null
	property var feat_type: null
	property var potential_feats: []
	
	SmallText {
		id: text_label
		anchors.top: title_item.bottom
		anchors.topMargin: 16 * scale_factor
		anchors.left: parent.left
		anchors.leftMargin: 8 * scale_factor
		anchors.right: parent.right
		anchors.rightMargin: 8 * scale_factor
		text: "Which feat shall you acquire?"
		wrapMode: Text.WordWrap
	}
	
	Flickable {
		id: feat_button_column_view
		anchors.top: text_label.bottom
		anchors.topMargin: 16 * scale_factor
		anchors.left: parent.left
		anchors.right: parent.right
		height: Math.min(contentHeight, 384 * scale_factor)
		contentHeight: contentItem.childrenRect.height
		leftMargin: 0
		rightMargin: 0
		topMargin: 0
		bottomMargin: 0
		boundsBehavior: Flickable.StopAtBounds
		clip: true
		
		Column {
			id: feat_button_column
			anchors.top: parent.top
			anchors.horizontalCenter: parent.horizontalCenter
			spacing: 8 * scale_factor
			
			Repeater {
				model: potential_feats
				
				TextButton {
					id: feat_button
					text: format_text(feat.name)
					width: feat_choice_dialog.width - 16 * scale_factor
					tooltip: format_text(small_text(feat.get_modifier_string(character)))
					
					readonly property var feat: model.modelData
					
					onClicked: {
						character.game_data.on_feat_chosen(feat)
						feat_choice_dialog.close()
						feat_choice_dialog.destroy()
					}
				}
			}
		}
	}
}
