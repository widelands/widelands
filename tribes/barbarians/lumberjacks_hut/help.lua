include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian Lumberjack's Hut")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/lumberjacks_hut/lumberjacks_hut_i_00.png", p(
			_[[‘Take 200 hits to fell a tree and you’re a baby. Take 100 and you’re a soldier. Take 50 and you’re a hero. Take 20 and soon you will be a honorable lumberjack.’]])) ..
		rt("text-align=right",p("font-size=10 font-style=italic", _[[Krumta, carpenter of Chat’Karuth]])) ..
	--General Section
		rt(h2(_"General")) ..
		rt(h3(_"Purpose:")) ..
		image_line("tribes/barbarians/log/menu.png",1,p(_"Fell trees in the surrounding area and process them into logs.")) ..
		text_line(_"Working radius:", "10") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		rt(p(_"The Lumberjack’s Hut needs trees in the immediate vicinity to fell.")) ..
		text_line(_"Incoming:", "n/a") ..
		rt(h3(_"Outgoing:")) ..
		dependencies({"tribes/barbarians/lumberjacks_hut/menu.png","tribes/barbarians/log/menu.png"},p(_"Log")) ..
		dependencies({"tribes/barbarians/log/menu.png","tribes/barbarians/constructionsite/menu.png"},p(_"Construction Site")) ..
		dependencies({"tribes/barbarians/log/menu.png","tribes/barbarians/burners_house/menu.png"},p(_"Charcoal Burner’s House")) ..
		dependencies({"tribes/barbarians/log/menu.png","tribes/barbarians/metalworks/menu.png"},p(_"Metal Workshop")) ..
		dependencies({"tribes/barbarians/log/menu.png","tribes/barbarians/hardener/menu.png"},p(_"Wood Hardener")) ..
	--Building Section
		rt(h2(_"Building")) ..
		text_line(_"Space required:",_"Small plot","pics/small.png") ..
		text_line(_"Upgraded from:", "n/a") ..
		rt(h3(_"Build cost:")) ..
		help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 3), 3) ..
		rt(h3(_"Dismantle yields:")) ..
		help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 2), 2) ..
		text_line(_"Upgradeable to:","n/a") ..
	--Workers Section
		rt(h2(_"Workers")) ..
		rt(h3(_"Crew required:")) ..
		image_line("tribes/barbarians/lumberjack/menu.png", 1, p(_"Lumberjack")) ..
		text_line(_"Worker uses:",_"Felling Ax","tribes/barbarians/felling_ax/menu.png") ..
		text_line(_"Experience levels:","n/a") ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"The Lumberjack needs %s to fell a tree, not counting the time he needs to reach the destination and go home again.":format("12s"))
}
