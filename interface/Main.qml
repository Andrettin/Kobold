import QtQuick
import QtQuick.Controls
import QtQuick.Window
import "./menus"

Window {
	id: window
	visible: true
	title: qsTr("Kobold")
	width: Screen.width
	height: Screen.height + 1 //it needs to be +1 otherwise it becomes (non-borderless) fullscreen automatically
	flags: Qt.FramelessWindowHint | Qt.Window
	color: "black"
	
	readonly property real scale_factor: kobold.scale_factor
	readonly property color interface_background_color: Qt.rgba(48.0 / 255.0, 48.0 / 255.0, 48.0 / 255.0, 1)
	
	property int politics_view_mode: PoliticsView.Offices
	property int trade_view_mode: TradeView.TradeOrders
	
	FontLoader {
		id: berenika_font
		source: "../fonts/berenika.ttf"
	}
	
	FontLoader {
		id: berenika_bold_font
		source: "../fonts/berenika_bold.ttf"
	}
	
	Item { //tooltips need to be attached to an item
		id: tooltip_manager
		
		property int tooltip_y_override: 0
	}
	
	MenuStack {
		id: menu_stack
		initialItem: "menus/MainMenu.qml"
		visible: kobold.running
	}
	
	Connections {
		target: kobold
		function onRunningChanged() {
			if (kobold.running) {
				//kobold.get_world("earth").write_province_image(0, 1)
				//kobold.get_map_template("earth").write_province_image()
				//kobold.get_map_template("earth").write_terrain_image()
				//kobold.get_map_template("earth").write_river_image()
				//kobold.get_map_template("earth").write_border_river_image()
				//kobold.get_map_template("earth").write_route_image()
			}
		}
	}
	
	Connections {
		target: kobold.game
		function onRunningChanged() {
			if (kobold.game.running) {
				//replace the scenario menu or the random map menu with the map view
				menu_stack.replace("CustomMapView.qml")
			}
		}
	}
	
	function colored_text(text, color) {
		var font_color_str = "<font color=\"" + color + "\">"
		
		text = text.replace(/<font color=\"gold\">/g, font_color_str)
		text = text.replace(/(?:<font color=\"#)(.{6})(?:\">)/g, font_color_str)
		return font_color_str + text + "</font>"
	}
	
	function highlight(text) {
		//highlight text
		return colored_text(text, "gold")
	}
	
	function format_text(text) {
		var str = text
		str = str.replace(/\n/g, "<br>")
		str = str.replace(/\t/g, "<font color=\"transparent\">aaaa</font>") //whitespaces are ignored after a <br>
		str = str.replace(/~</g, "<font color=\"gold\">")
		str = str.replace(/~>/g, "</font>")
		return str
	}
	
	function font_size_text(text, font_size) {
		return "<span style='font-size: " + font_size + "px;'>" + text + "</span>"
	}
	
	function small_text(text) {
		return font_size_text(text, 10 * scale_factor)
	}
	
	//generate a random number
	function random(n) {
		return Math.floor(Math.random() * n)
	}
	
	//format a number as text
	function number_string(n) {
		return n.toLocaleString(Qt.locale("en_US"), 'f', 0)
	}
	
	function roman_number_string(n) {
		const dict = {
			M: 1000,
			CM:900,
			D:500,
			CD:400,
			C:100,
			XC:90,
			L:50,
			XL:40,
			X:10,
			IX:9,
			V:5,
			IV:4,
			I:1
		}
		
		var str = ""
		
		for (var key in dict) {
			while (n >= dict[key]) {
				str += key
				n -= dict[key]
			}
		}
		
		return str
	}
	
	function signed_number_string(n) {
		if (n < 0) {
			return n
		}
		return "+" + n
	}
	
	function date_year(date) {
		var year = date.getUTCFullYear()
		
		if (year < 0) {
			year -= 1 //-1 is needed, as otherwise negative dates are off by one
		}
		
		return year
	}
	
	function date_year_string(date) {
		var year = date_year(date)
		return year_string(year)
	}
	
	function year_string(year) {
		var year_suffix = ""
		
		if (year < 0) {
			year_suffix = " BC"
			year = Math.abs(year)
		}
		
		var year_str
		if (year >= 10000) {
			year_str = number_string(year)
		} else {
			year_str = year
		}
		
		return year_str + year_suffix
	}
	
	function date_year_range_string(date1, date2) {
		return year_range_string(date_year(date1), date_year(date2))
	}
	
	function year_range_string(year1, year2) {
		return Math.abs(year1) + "-" + year_string(year2)
	}
	
	function color_hex_string(color) {
		var red_hex_str = Math.floor(color.r * 255).toString(16)
		if (red_hex_str.length < 2) {
			red_hex_str = "0" + red_hex_str
		}
		
		var green_hex_str = Math.floor(color.g * 255).toString(16)
		if (green_hex_str.length < 2) {
			green_hex_str = "0" + green_hex_str
		}
		
		var blue_hex_str = Math.floor(color.b * 255).toString(16)
		if (blue_hex_str.length < 2) {
			blue_hex_str = "0" + blue_hex_str
		}
		
		return red_hex_str + green_hex_str + blue_hex_str
	}
	
	function string_list_to_string(str_list, separator, ignore_empty_strings = false) {
		var str = ""
		var first = true
		
		for (var i = 0; i < str_list.length; i++) {
			if (ignore_empty_strings && str_list[i].length == 0) {
				continue
			}
			
			if (first) {
				first = false
			} else {
				str += separator
			}
			
			str += str_list[i]
		}
		
		return str
	}
	
	function object_list_to_name_list(object_list, prefix = "", suffix = "") {
		var name_list = []
		
		for (var object of object_list) {
			name_list.push(prefix + object.name + suffix)
		}
		
		return name_list
	}
	
	function object_list_to_full_name_list(object_list, prefix = "", suffix = "") {
		var name_list = []
		
		for (var object of object_list) {
			name_list.push(prefix + object.full_name + suffix)
		}
		
		return name_list
	}
	
	function object_counts_to_name_list(object_counts, prefix = "", suffix = "") {
		var name_list = []
		
		for (var kv_pair of object_counts) {
			var object = kv_pair.key
			var count = kv_pair.value
			name_list.push(prefix + object.name + (count > 1 ? (" (x" + count + ")") : "") + suffix)
		}
		
		return name_list
	}
	
	function costs_to_string(costs, modifier) {
		var str = "Costs:"
		
		for (var i = 0; i < costs.length; i++) {
			var commodity = costs[i].key
			var cost = costs[i].value
			
			str += "\n\t" + Math.floor(cost * modifier / 100) + " " + highlight(commodity.name)
		}
		
		return str
	}
	
	function values_to_string(values, base_value = 0, separator = ", ", signed = false) {
		var str = ""
		
		for (var i = 0; i < values.length; i++) {
			var value_type = values[i].key
			var value = values[i].value + base_value
			
			if (i > 0) {
				str += separator
			}
			
			str += value_type.name + " " + (signed ? signed_number_string(value) : number_string(value))
		}
		
		return str
	}
	
	function bonuses_to_string(bonuses, base_value = 0, separator = ", ") {
		return values_to_string(bonuses, base_value, separator, true)
	}
	
	function get_plural_form(str) {
		if (str.endsWith("y")) {
			return str.substr(0, str.length - 1) + "ies"
		}
		
		return str + "s"
	}
}
