include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian Farm")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/farm/farm_i_00.png", p(
			_[[Text needed]])) ..
		rt("text-align=right",p("font-size=10 font-style=italic", _[[Source needed]])) ..
	--General Section
		rt(h2(_"General")) ..
		text_line(_"Purpose:", _"Sow and harvest wheat.","tribes/barbarians/wheat/menu.png") ..
		text_line(_"Working radius:", "2") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		text_line(_"Incoming:", "n/a") ..
		rt(h3(_"Outgoing:")) ..
		dependencies({"tribes/barbarians/farm/menu.png","tribes/barbarians/wheat/menu.png"},p(_"Wheat")) ..
		dependencies({"tribes/barbarians/wheat/menu.png","tribes/barbarians/bakery/menu.png"},p(_"Bakery")) ..
		dependencies({"tribes/barbarians/wheat/menu.png","tribes/barbarians/micro-brewery/menu.png"},p(_"Micro Brewery")) ..
		dependencies({"tribes/barbarians/wheat/menu.png","tribes/barbarians/brewery/menu.png"},p(_"Brewery")) ..
		dependencies({"tribes/barbarians/wheat/menu.png","tribes/barbarians/cattlefarm/menu.png"},p(_"Cattle Farm")) ..
	--Building Section
		rt(h2(_"Building")) ..
		text_line(_"Space required:",_"Big plot","pics/big.png") ..
		text_line(_"Upgraded from:", "n/a") ..
		rt(h3(_"Build cost:")) ..
		help_building_line("barbarians", "blackwood", ngettext("%i Blackwood", "%i Blackwood", 1), 1) ..
		help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 3), 3) ..
		help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 4), 4) ..
		rt(h3(_"Dismantle yields:")) ..
		help_building_line("barbarians", "blackwood", ngettext("%i Blackwood", "%i Blackwood", 1), 1) ..
		help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 2), 2) ..
		help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 2), 2) ..
		text_line(_"Upgradeable to:","n/a") ..
	--Workers Section
		rt(h2(_"Workers")) ..
		rt(h3(_"Crew required:")) ..
		image_line("tribes/barbarians/farmer/menu.png", 1, p(_"Farmer")) ..
		text_line(_"Worker uses:", _"Scythe","tribes/barbarians/scythe/menu.png") ..
		text_line(_"Experience levels:","n/a") ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"Calculation needed")
}
