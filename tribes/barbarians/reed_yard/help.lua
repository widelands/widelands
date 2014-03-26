include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian Reed Yard")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/reed_yard/reed_yard_i_00.png", p(
			_[[‘We grow roofs’]])) ..
		rt("text-align=right",p("font-size=10 font-style=italic", _[[Slogan of the Guild of Gardeners]])) ..
	--General Section
		rt(h2(_"General")) ..
		text_line(_"Purpose:", _"The Reed Yard cultivates reed that serves two different purposes for the Barbarian tribe.<br>It is the traditional material for roofing, and it is woven into the extremely durable cloth that they use for their ships’ sails.","tribes/barbarians/thatchreed/menu.png") ..
		text_line(_"Working radius:", "1") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		text_line(_"Incoming:","n/a") ..
		rt(h3(_"Outgoing:")) ..
		dependencies({"tribes/barbarians/reed_yard/menu.png","tribes/barbarians/thatchreed/menu.png"},p(_"Thatch Reed")) ..
		dependencies({"tribes/barbarians/thatchreed/menu.png","tribes/barbarians/constructionsite/menu.png"},p(_"Construction Site")) ..
		rt(p(_"The Reed Yard’s output will only go to construction sites that need it.")) ..
		dependencies({"tribes/barbarians/thatchreed/menu.png","tribes/barbarians/weaving-mill/menu.png"},p(_"Weaving-Mill")) ..
	--Building Section
		rt(h2(_"Building")) ..
		text_line(_"Space required:",_"Medium plot","pics/medium.png") ..
		text_line(_"Upgraded from:", "n/a") ..
		rt(h3(_"Build cost:")) ..
		help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 2), 2) ..
		help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 5), 5) ..
		rt(h3(_"Dismantle yields:")) ..
		help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 1), 1) ..
		help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 3), 3) ..
		text_line(_"Upgradeable to:","n/a") ..
	--Workers Section
		rt(h2(_"Workers")) ..
		rt(h3(_"Crew required:")) ..
		image_line("tribes/barbarians/gardener/menu.png", 1, p(_"Gardener")) ..
		text_line(_"Worker uses:", _"Shovel","tribes/barbarians/shovel/menu.png") ..
		text_line(_"Experience levels:","n/a") ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"A Reed Yard can produce a sheaf of Thatch Reed in about %s on average.":format("65s"))
}
