import QtQuick
import QtQuick.Controls
import ".."

Item {
	id: menu
	focus: true

	property string title: ""
	//property string background: wyrmgus.defines.default_menu_background_file
	property string music_type: "menu"
	readonly property var title_item: title_text
	
	/*
	Image {
		anchors.fill: parent
		source: "file:///" + background
		fillMode: Image.PreserveAspectCrop
	}
	*/
	
	LargeText {
		id: title_text
		text: highlight(menu.title)
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.top: parent.top
		anchors.topMargin: 36 * scale_factor
	}
	
	/*
	onFocusChanged: {
		if (focus && !wyrmgus.game.running) {
			wyrmgus.play_music(music_type)
		}
	}
	*/
}
