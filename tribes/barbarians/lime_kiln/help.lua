use("aux", "formatting")
use("aux", "format_help")

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian Limekiln")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/lime_kiln/stonegrinder_i_00.png", p(
			_[["Forming new stone from old with fire and water."]])) ..
		rt("text-align=right",p("font-size=10 font-style=italic", _[[Ragnald the Child's answer to the question, what he's doing in the dirt. His "new stone" now is an important building material.]])) ..
	--General Section
		rt(h2(_"General")) ..
		text_line(_"Purpose:", _[[The Limekiln processes Raw Stones to make "Grout", a substance that solidifies and so reinforces masonry.]],"tribes/barbarians/grout/menu.png") ..
		text_line(_"Working radius:", "n/a") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		rt(h3(_"Incoming:")) ..
		dependencies({"tribes/barbarians/granitemine/menu.png;tribes/barbarians/quarry/menu.png","tribes/barbarians/raw_stone/menu.png","tribes/barbarians/lime_kiln/menu.png"}) ..
		rt(p(_"Raw Stone from the Granite Mine or Quarry")) ..
		dependencies({"tribes/barbarians/coalmine/menu.png;tribes/barbarians/burners_house/menu.png","tribes/barbarians/coal/menu.png","tribes/barbarians/axefactory/menu.png"}) ..
		rt(p(_"%s from the Coal mine or the Charcoal Burner's House":format(_"Coal"))) ..
		dependencies({"tribes/barbarians/well/menu.png","tribes/barbarians/water/menu.png","tribes/barbarians/lime_kiln/menu.png"},p(_"Water from a Well")) ..
		--rt() ..
		rt(h3(_"Outgoing:")) ..
		dependencies({"tribes/barbarians/lime_kiln/menu.png","tribes/barbarians/grout/menu.png"},p(_"Grout")) ..
		dependencies({"tribes/barbarians/grout/menu.png","tribes/barbarians/constructionsite/menu.png"},p(_"Construction Site")) ..
		rt(p(_"The Limekiln's output will only go to construction sites that need it. Those are predominantly houses that work with fire, and some military sites.")) ..
	--Building Section
		rt(h2(_"Building")) ..
		text_line(_"Space required:",_"Medium plot","pics/medium.png") ..
		text_line(_"Upgraded from:", "n/a") ..
		rt(h3(_"Build Cost:")) ..
		image_line("tribes/barbarians/blackwood/menu.png", 1, p("1 " .. _"Blackwood")) ..
		image_line("tribes/barbarians/raw_stone/menu.png", 2, p("2 " .. _"Raw Stone")) ..
		image_line("tribes/barbarians/trunk/menu.png", 4, p("4 " .. _"Trunk")) ..
		rt(h3(_"Dismantle yields:")) ..
		image_line("tribes/barbarians/blackwood/menu.png", 1, p("1 " .. _"Blackwood")) ..
		image_line("tribes/barbarians/raw_stone/menu.png", 1, p("1 " .. _"Raw Stone")) ..
		image_line("tribes/barbarians/trunk/menu.png", 2, p("2 " .. _"Trunk")) ..
		text_line(_"Upgradeable to:","n/a") ..
	--Workers Section
		rt(h2(_"Workers")) ..
		rt(h3(_"Crew required:")) ..
		image_line("tribes/barbarians/lime-burner/menu.png", 1, p(_"Lime-Burner")) ..
		text_line(_"Worker uses:", "n/a") ..
		text_line(_"Experience levels:","n/a") ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"If all needed wares are delivered in time, this building can produce %s in about %s on average.":format(_"Grout","41s"))
}