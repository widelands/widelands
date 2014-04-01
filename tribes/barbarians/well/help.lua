include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian Well")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/well/well_i_00.png", p(
			_[[‘Oh how sweet is the source of life,<br> that comes down from the sky <br> and lets the earth drink.’]])) ..
		rt("text-align=right",p("font-size=10 font-style=italic", _[[Song written by Sigurd the Bard<br>when the first rain fell after the Great Drought in the 21st year of Chat’Karuth’s reign.]])) ..
	--General Section
		rt(h2(_"General")) ..
		text_line(_"Purpose:", _"Draw Water out of the deep.","tribes/barbarians/water/menu.png") ..
		text_line(_"Working radius:", "1") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		text_line(_"Incoming:", "n/a") ..
		rt(h3(_"Outgoing:")) ..
		dependencies({"tribes/barbarians/resi_water1/resi_00.png","tribes/barbarians/well/menu.png","tribes/barbarians/water/menu.png"},p(_"Water")) ..
		dependencies({"tribes/barbarians/water/menu.png","tribes/barbarians/bakery/menu.png"},p(_"Bakery")) ..
		dependencies({"tribes/barbarians/water/menu.png","tribes/barbarians/micro-brewery/menu.png"},p(_"Micro Brewery")) ..
		dependencies({"tribes/barbarians/water/menu.png","tribes/barbarians/brewery/menu.png"},p(_"Brewery")) ..
		dependencies({"tribes/barbarians/water/menu.png","tribes/barbarians/cattlefarm/menu.png"},p(_"Cattle Farm")) ..
		dependencies({"tribes/barbarians/water/menu.png","tribes/barbarians/lime_kiln/menu.png"},p(_"Lime Kiln")) ..
	--Building Section
		rt(h2(_"Building")) ..
		text_line(_"Space required:",_"Small plot","pics/small.png") ..
		text_line(_"Upgraded from:", "n/a") ..
		rt(h3(_"Build cost:")) ..
		help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 4), 4) ..

		rt(h3(_"Dismantle yields:")) ..
		help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 2), 2) ..
		text_line(_"Upgradeable to:","n/a") ..
	--Workers Section
		rt(h2(_"Workers")) ..
		rt(h3(_"Crew required:")) ..
		image_line("tribes/barbarians/carrier/menu.png", 1, p(_"Carrier")) ..
		text_line(_"Worker uses:", "n/a") ..
		text_line(_"Experience levels:","n/a") ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"The Carrier needs %s to get one bucket full of water.":format("40s"))
}
