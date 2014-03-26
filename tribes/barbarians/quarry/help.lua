include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian Quarry")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/quarry/b_quarry_i_00.png", p(
			_[[Text needed]])) ..
		rt("text-align=right",p("font-size=10 font-style=italic", _[[Source needed]])) ..
	--General Section
		rt(h2(_"General")) ..
		rt(h2(_"Purpose:")) ..
		image_line("tribes/barbarians/raw_stone/menu.png",1,p(_"Carve %s out of stones lying near the hut.":format(_"Raw Stone"))) ..
		text_line(_"Working radius:", "6") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		rt(p(_"The Quarry needs stones in the vicinity.")) ..
		text_line(_"Incoming:", "n/a") ..
		rt(h2(_"Outgoing:")) ..
		dependencies({"tribes/barbarians/quarry/menu.png","tribes/barbarians/raw_stone/menu.png"},p(_"Raw Stone")) ..
		dependencies({"tribes/barbarians/raw_stone/menu.png","tribes/barbarians/constructionsite/menu.png"},p(_"Construction Site")) ..
		dependencies({"tribes/barbarians/raw_stone/menu.png","tribes/barbarians/lime_kiln/menu.png"}, p(_"Lime Kiln")) ..
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
		image_line("tribes/barbarians/stonemason/menu.png", 1, p(_"Stonemason")) ..
		text_line(_"Worker uses:",_"Pick","tribes/barbarians/pick/menu.png") ..
		text_line(_"Experience levels:","n/a") ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"The stonemason pauses 25s before going to work again.")
}
