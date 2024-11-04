import QtQuick
import QtQuick.Controls
import "./dialogs"

Item {
	id: offices_view
	
	PortraitGridItem {
		id: ruler_portrait
		anchors.top: parent.top
		anchors.topMargin: portrait_grid.spacing
		anchors.horizontalCenter: parent.horizontalCenter
		portrait_identifier: ruler ? ruler.game_data.portrait.identifier : ""
		visible: ruler !== null
		
		onClicked: {
			character_dialog.character = ruler
			character_dialog.open()
		}
		
		onEntered: {
			status_text = ruler.game_data.titled_name
		}
		
		onExited: {
			status_text = ""
		}
	}
	
	Grid {
		id: portrait_grid
		anchors.top: ruler_portrait.visible ? ruler_portrait.bottom : parent.top
		anchors.topMargin: spacing
		anchors.bottom: parent.bottom
		anchors.horizontalCenter: parent.horizontalCenter
		width: columns * (portrait_width + spacing) - spacing
		columns: Math.floor((parent.width - min_spacing) / (portrait_width + min_spacing))
		spacing: Math.max(min_spacing, Math.floor((parent.width - columns * portrait_width) / (columns + 1)))
		
		readonly property int portrait_width: 64 * scale_factor + 2 * scale_factor
		readonly property int min_spacing: 16 * scale_factor
		
		Repeater {
			model: filter_office_holders(country_game_data.office_holders)
			
			PortraitGridItem {
				portrait_identifier: office_holder.game_data.portrait.identifier
				
				readonly property var office: model.modelData.key
				readonly property var office_holder: model.modelData.value
				
				onClicked: {
					character_dialog.character = office_holder
					character_dialog.open()
				}
				
				onEntered: {
					status_text = office_holder.game_data.titled_name
				}
				
				onExited: {
					status_text = ""
				}
			}
		}
	}
	
	CharacterDialog {
		id: character_dialog
	}
	
	function filter_office_holders(office_holders) {
		var result = []
		
		for (var kv_pair of office_holders) {
			if (kv_pair.key == kobold.defines.ruler_office) {
				continue
			}
			
			result.push(kv_pair)
		}
		
		return result
	}
}
