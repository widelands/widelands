include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian Bakery")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/bakery/bakery_i_00.png", p(
			_[[Text needed]])) ..
		rt("text-align=right",p("font-size=10 font-style=italic", _[[Source needed]])) ..
	--General Section
		rt(h2(_"General")) ..
		text_line(_"Purpose:", _"Bake Pitta Bread for Soldiers and Miners alike.","tribes/barbarians/pittabread/menu.png") ..
		text_line(_"Working radius:", "n/a") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		rt(h3(_"Incoming:")) ..
		dependencies({"tribes/barbarians/farm/menu.png","tribes/barbarians/wheat/menu.png","tribes/barbarians/bakery/menu.png"},p(_"Wheat from the Farm")) ..
		dependencies({"tribes/barbarians/well/menu.png","tribes/barbarians/water/menu.png","tribes/barbarians/bakery/menu.png"},p(_"Water from the Well")) ..
		rt(h3(_"Outgoing:")) ..
		dependencies({"tribes/barbarians/bakery/menu.png","tribes/barbarians/pittabread/menu.png"},p(_"Pitta Bread")) ..
		dependencies({"tribes/barbarians/pittabread/menu.png","tribes/barbarians/tavern/menu.png"},p(_"Tavern")) ..
		dependencies({"tribes/barbarians/pittabread/menu.png","tribes/barbarians/inn/menu.png"},p(_"Inn")) ..
		dependencies({"tribes/barbarians/pittabread/menu.png","tribes/barbarians/big_inn/menu.png"},p(_"Big Inn")) ..
		dependencies({"tribes/barbarians/pittabread/menu.png","tribes/barbarians/battlearena/menu.png"},p(_"Battle Arena")) ..
		dependencies({"tribes/barbarians/pittabread/menu.png","tribes/barbarians/trainingcamp/menu.png"},p(_"Training Camp")) ..
	--Building Section
		rt(h2(_"Building")) ..
		text_line(_"Space required:",_"Medium plot","pics/medium.png") ..
		text_line(_"Upgraded from:", "n/a") ..
		rt(h3(_"Build cost:")) ..
		help_building_line("barbarians", "blackwood", ngettext("%i Blackwood", "%i Blackwood", 2), 2) ..
		help_building_line("barbarians", "thatchreed", ngettext("%i Thatch Reed", "%i Thatch Reeds", 2), 2) ..
		help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 2), 2) ..
		help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 2), 2) ..
		rt(h3(_"Dismantle yields:")) ..
		help_building_line("barbarians", "blackwood", ngettext("%i Blackwood", "%i Blackwood", 1), 1) ..
		help_building_line("barbarians", "thatchreed", ngettext("%i Thatch Reed", "%i Thatch Reeds", 1), 1) ..
		help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 1), 1) ..
		help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 1), 1) ..
		text_line(_"Upgradeable to:","n/a") ..
	--Workers Section
		rt(h2(_"Workers")) ..
		rt(h3(_"Crew required:")) ..
		image_line("tribes/barbarians/baker/menu.png", 1, p(_"Baker")) ..
		text_line(_"Worker uses:", _"Bread Paddle","tribes/barbarians/bread_paddle/menu.png") ..
		text_line(_"Experience levels:","n/a") ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"If all needed wares are delivered in time, this building can produce %1$s in %2$s on average.":bformat(_"a Pitta Bread","30s"))
}
