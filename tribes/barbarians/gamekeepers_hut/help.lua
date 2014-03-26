include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian Gamekeeper's Hut")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/gamekeepers_hut/gamekeeper_i_00.png", p(
			_[[‘He loves the animals and to breed them<br>as we love to cook and eat them.’]])) ..
		rt("text-align=right",p("font-size=10 font-style=italic", _[[Barbarian nursery rhyme]])) ..
	--General Section
		rt(h2(_"General")) ..
		text_line(_"Purpose:", _"Releases animals into the wild to steady the meat production.") ..
		text_line(_"Working radius:", "3") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		text_line(_"Incoming:", "n/a") ..
		text_line(_"Outgoing:","n/a") ..
	--Building Section
		rt(h2(_"Building")) ..
		text_line(_"Space required:",_"Small plot","pics/small.png") ..
		text_line(_"Upgraded from:", "n/a") ..
		rt(h3(_"Build cost:")) ..
		help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 1), 1) ..
		help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 4), 4) ..
		rt(h3(_"Dismantle yields:")) ..
		help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 1), 1) ..
		help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 2), 2) ..
		text_line(_"Upgradeable to:","n/a") ..
	--Workers Section
		rt(h2(_"Workers")) ..
		rt(h3(_"Crew required:")) ..
		image_line("tribes/barbarians/gamekeeper/menu.png", 1, p(_"Gamekeeper")) ..
		text_line(_"Worker uses:","n/a") ..
		text_line(_"Experience levels:","n/a") ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"The gamekeeper pauses 52.5s before going to work again.")
}
