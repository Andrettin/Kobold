import QtQuick
import QtQuick.Controls

Rectangle {
	id: infopanel
	color: interface_background_color
	width: 64 * scale_factor
	
	Rectangle {
		color: "gray"
		anchors.top: parent.top
		anchors.topMargin: 15 * scale_factor
		anchors.bottom: parent.bottom
		anchors.bottomMargin: 15 * scale_factor
		anchors.right: parent.right
		width: 1 * scale_factor
	}
	
	IndustryCounter {
		id: labor_counter
		anchors.left: parent.left
		anchors.top: parent.top
		anchors.topMargin: 96 * scale_factor
		name: "Labor"
		icon_identifier: "labor"
		count: (country_game_data.commodity_outputs && country_game_data.commodity_inputs) ? (country_game_data.get_commodity_output("labor") - country_game_data.get_commodity_input("labor")) : 0 //the conditional is there to make the counter be updated when the labor output or input changes
	}
	
	TextButton {
		id: back_button
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.bottom: parent.bottom
		anchors.bottomMargin: 16 * scale_factor
		text: qsTr("Back")
		width: 48 * scale_factor
		height: 24 * scale_factor
		
		onClicked: {
			menu_stack.pop()
		}
	}
}
