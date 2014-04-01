include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian Fisher's Hut")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/fishers_hut/fisher_i_00.png", p(
			_[[‘What do you mean, my fish ain’t fresh?!’]])) ..
		rt("text-align=right",p("font-size=10 font-style=italic", _[[Source needed]])) ..
	--General Section
		rt(h2(_"General")) ..
		rt(h2(_"Purpose:")) ..
		image_line("tribes/barbarians/fish/menu.png",1,p(_"Fishes on the coast near the hut.")) ..
		text_line(_"Working radius:", "7") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		rt(p(_"The Fisher’s Hut needs water full of fish in the vicinity.")) ..
		text_line(_"Incoming:", "n/a") ..
		rt(h2(_"Outgoing:")) ..
		dependencies({"tribes/barbarians/fishers_hut/menu.png","tribes/barbarians/fish/menu.png"},p(_"Fish")) ..
		dependencies({"tribes/barbarians/fish/menu.png","tribes/barbarians/battlearena/menu.png"},p(_"Battle Arena")) ..
		dependencies({"tribes/barbarians/fish/menu.png","tribes/barbarians/trainingcamp/menu.png"},p(_"Training Camp")) ..
		dependencies({"tribes/barbarians/fish/menu.png","tribes/barbarians/tavern/menu.png"},p(_"Tavern")) ..
		dependencies({"tribes/barbarians/fish/menu.png","tribes/barbarians/inn/menu.png"},p(_"Inn")) ..
		dependencies({"tribes/barbarians/fish/menu.png","tribes/barbarians/big_inn/menu.png"},p(_"Big Inn")) ..
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
		image_line("tribes/barbarians/fisher/menu.png", 1, p(_"Fisher")) ..
		text_line(_"Worker uses:",_"Fishing Rod","tribes/barbarians/fishing_rod/menu.png") ..
		text_line(_"Experience levels:","n/a") ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"The fisher pauses 18s before going to work again.")
}
