use("aux", "formatting")
use("aux", "format_help")

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian Hunter's Hut")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/hunters_hut/hunter_i_00.png", p(
			_[["As silent as a panther,<br> as deft as a weasel,<br> as swift as an arrow,<br> as deadly as a viper."]])) ..
		rt("text-align=right",p("font-size=10 font-style=italic", _[["The Art of Hunting"]])) ..
	--General Section
		rt(h2(_"General")) ..
		rt(h3(_"Purpose:")) ..
		image_line("tribes/barbarians/meat/menu.png",1,p(_"Hunt animals to produce meat.")) ..
		text_line(_"Working radius:", "13") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		rt(p(_"The Hunter's Hut needs animals in the vicinity to hunt.")) ..
		text_line(_"Incoming:", "n/a") ..
		rt(h3(_"Outgoing:")) ..
		dependencies({"tribes/barbarians/hunters_hut/menu.png","tribes/barbarians/meat/menu.png"},p(_"Meat")) ..
		dependencies({"tribes/barbarians/meat/menu.png","tribes/barbarians/battlearena/menu.png"},p(_"Battle Arena")) ..
		dependencies({"tribes/barbarians/meat/menu.png","tribes/barbarians/trainingscamp/menu.png"},p(_"Trainingscamp")) ..
		dependencies({"tribes/barbarians/meat/menu.png","tribes/barbarians/tavern/menu.png"},p(_"Tavern")) ..
		dependencies({"tribes/barbarians/meat/menu.png","tribes/barbarians/inn/menu.png"},p(_"Inn")) ..
		dependencies({"tribes/barbarians/meat/menu.png","tribes/barbarians/big_inn/menu.png"},p(_"Big Inn")) ..
	--Building Section
		rt(h2(_"Building")) ..
		text_line(_"Space required:",_"Small plot","pics/small.png") ..
		text_line(_"Upgraded from:", "n/a") ..
		rt(h3(_"Build Cost:")) ..
		image_line("tribes/barbarians/raw_stone/menu.png", 1, p("1 " .. _"Raw Stone")) ..
		image_line("tribes/barbarians/trunk/menu.png", 4, p("4 " .. _"Trunk")) ..
		rt(h3(_"Dismantle yields:")) ..
		image_line("tribes/barbarians/raw_stone/menu.png", 1, p("1 " .. _"Raw Stone")) ..
		image_line("tribes/barbarians/trunk/menu.png", 2, p("2 " .. _"Trunk")) ..
		text_line(_"Upgradeable to:","n/a") ..
	--Workers Section
		rt(h2(_"Workers")) ..
		rt(h3(_"Crew required:")) ..
		image_line("tribes/barbarians/hunter/menu.png", 1, p(_"Hunter")) ..
		text_line(_"Worker uses:",_"Hunting Spear","tribes/barbarians/hunting_spear/menu.png") ..
		text_line(_"Experience levels:","n/a") ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"The %s pauses %s before going to work again.":format(_"Hunter","35s"))
}
