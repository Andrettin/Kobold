import QtQuick
import QtQuick.Controls
import ".."

DialogBase {
	id: modifier_dialog
	width: Math.max(256 * scale_factor, modifier_text.contentWidth + 16 * scale_factor)
	height: ok_button.y + ok_button.height + 8 * scale_factor
	
	property string type_string: ""
	property string requirements_string: ""
	property string modifier_string: ""
	property string date_string: ""
	property string description: ""
	
	SmallText {
		id: type_text
		anchors.top: title_item.bottom
		anchors.topMargin: 16 * scale_factor
		anchors.horizontalCenter: parent.horizontalCenter
		text: format_text(type_string)
		visible: type_string.length > 0
	}
	
	SmallText {
		id: requirements_text
		anchors.top: type_text.visible ? type_text.bottom : title_item.bottom
		anchors.topMargin: 16 * scale_factor
		anchors.horizontalCenter: parent.horizontalCenter
		text: format_text(requirements_string)
		visible: requirements_string.length > 0
	}
	
	SmallText {
		id: modifier_text
		anchors.top: requirements_text.visible ? requirements_text.bottom : (type_text.visible ? type_text.bottom : title_item.bottom)
		anchors.topMargin: 16 * scale_factor
		anchors.horizontalCenter: parent.horizontalCenter
		text: format_text(modifier_string)
		visible: modifier_string.length > 0
	}
	
	SmallText {
		id: date_text
		anchors.top: modifier_text.visible ? modifier_text.bottom : (requirements_text.visible ? requirements_text.bottom : (type_text.visible ? type_text.bottom : title_item.bottom))
		anchors.topMargin: 16 * scale_factor
		anchors.horizontalCenter: parent.horizontalCenter
		text: date_string
		visible: date_string.length > 0
	}
	
	SmallText {
		id: description_text
		anchors.top: date_text.visible ? date_text.bottom : (modifier_text.visible ? modifier_text.bottom : (requirements_text.visible ? requirements_text.bottom : (type_text.visible ? type_text.bottom : title_item.bottom)))
		anchors.topMargin: 16 * scale_factor
		anchors.left: parent.left
		anchors.leftMargin: 8 * scale_factor
		anchors.right: parent.right
		anchors.rightMargin: 8 * scale_factor
		text: format_text(description)
		visible: description.length > 0
		wrapMode: Text.WordWrap
	}
	
	TextButton {
		id: ok_button
		anchors.top: description_text.visible ? description_text.bottom
			: (date_text.visible ? date_text.bottom
				: (modifier_text.visible ? modifier_text.bottom
					: (requirements_text.visible ? requirements_text.bottom
						: (type_text.visible ? type_text.bottom
							: title_item.bottom))))
		anchors.topMargin: 16 * scale_factor
		anchors.horizontalCenter: parent.horizontalCenter
		text: "OK"
		onClicked: {
			modifier_dialog.close()
		}
	}
}
