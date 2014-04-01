include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian Cattle Farm")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/cattlefarm/cattlefarm_i_00.png", p(
			_[[Text needed]])) ..
		rt("text-align=right",p("font-size=10 font-style=italic", _[[Source needed]])) ..
	--General Section
		rt(h2(_"General")) ..
		text_line(_"Purpose:", _"Breed Oxen for adding them to the transportation system.","tribes/barbarians/ox/menu.png") ..
		text_line(_"Working radius:", "n/a") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		rt(h3(_"Incoming:")) ..
		dependencies({"tribes/barbarians/farm/menu.png","tribes/barbarians/wheat/menu.png","tribes/barbarians/cattlefarm/menu.png"},p(_"Wheat from the Farm")) ..
		dependencies({"tribes/barbarians/well/menu.png","tribes/barbarians/water/menu.png","tribes/barbarians/cattlefarm/menu.png"},p(_"Water from the Well")) ..
		rt(h3(_"Outgoing:")) ..
		dependencies({"tribes/barbarians/cattlefarm/menu.png","tribes/barbarians/ox/menu.png"},p(_"Ox")) ..
	--Building Section
		rt(h2(_"Building")) ..
		text_line(_"Space required:",_"Big plot","pics/big.png") ..
		text_line(_"Upgraded from:", "n/a") ..
		rt(h3(_"Build cost:")) ..
		help_building_line("barbarians", "blackwood", ngettext("%i Blackwood", "%i Blackwood", 1), 1) ..
		help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 2), 2) ..
		help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 2), 2) ..
		rt(h3(_"Dismantle yields:")) ..
		help_building_line("barbarians", "blackwood", ngettext("%i Blackwood", "%i Blackwood", 1), 1) ..
		help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 1), 1) ..
		text_line(_"Upgradeable to:","n/a") ..
	--Workers Section
		rt(h2(_"Workers")) ..
		rt(h3(_"Crew required:")) ..
		image_line("tribes/barbarians/cattlebreeder/menu.png", 1, p(_"Cattle Breeder")) ..
		text_line(_"Worker uses:", "n/a") ..
		text_line(_"Experience levels:","n/a") ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"If all needed wares are delivered in time, this building can produce %1$s in %2$s on average.":bformat(_"an Ox","30s"))
}
