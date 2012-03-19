use("aux", "formatting")
use("aux", "format_help")

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian Deeper Iron Ore Mine")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/deeper_oremine/deeper_oremine_i_00.png", p(--text identical to oremine
			_[["I look at my own pick wearing away day by day and I realize why my work is important."]])) ..
		rt("text-align=right",p("font-size=10 font-style=italic",_[[Quote from an anonymous miner.]])) ..
	--General Section
		rt(h2(_"General")) ..
		rt(p(_"A %s exploits all of the resource down to the deepest level.<br>But even after having done so, it will still have a %s chance of finding some more %s.":format(_"Deeper Iron Ore Mine","10%",_"Iron Ore"))) ..
		rt(h3(_"Purpose:")) ..
		image_line("tribes/barbarians/ironore/menu.png", 1, p(_"Dig %s out of the ground in mountain terrain.":format(_"Iron Ore"))) ..
		text_line(_"Working radius:", "2") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		rt(h3(_"Incoming:")) ..
		dependencies({"tribes/barbarians/big_inn/menu.png","tribes/barbarians/meal/menu.png","tribes/barbarians/deeper_oremine/menu.png"}, p(_"%s from a Big Inn":format(_"Meal"))) ..
		rt(h3(_"Outgoing:")) ..
		dependencies({"tribes/barbarians/resi_iron2/resi_00.png","tribes/barbarians/deeper_oremine/menu.png","tribes/barbarians/ironore/menu.png"}, p(_"Iron Ore")) ..
		dependencies({"tribes/barbarians/ironore/menu.png","tribes/barbarians/smelting_works/menu.png"}, p(_"Smelting Works")) ..
		rt(p(_"%s always goes to the %s. It has no other use.":format(_"Iron Ore",_"Smelting Works"))) ..
	--Building Section
		rt(h2(_"Building")) ..
		text_line(_"Space required:",_"Mine plot","pics/mine.png") ..
		text_line(_"Upgraded from:",_"Deep Iron Ore Mine","tribes/barbarians/deep_oremine/menu.png") ..
		rt(h3(_"Upgrade cost:")) ..
		image_line("tribes/barbarians/raw_stone/menu.png", 2, p("2 " .. _"Raw Stone")) ..
		image_line("tribes/barbarians/trunk/menu.png", 4, p("4 " .. _"Trunk")) ..
		rt(h3(_"Cost cumulative:")) ..
		image_line("tribes/barbarians/raw_stone/menu.png", 6, p("6 " .. _"Raw Stone")) ..
		image_line("tribes/barbarians/trunk/menu.png", 6, p("12 " .. _"Trunk")) ..
		image_line("tribes/barbarians/trunk/menu.png", 6) ..
		rt(h3(_"Dismantle yields:")) ..
		image_line("tribes/barbarians/raw_stone/menu.png", 3, p("3 " .. _"Raw Stone")) ..
		image_line("tribes/barbarians/trunk/menu.png", 6, p("6 " .. _"Trunk")) ..
		text_line(_"Upgradeable to:","n/a") ..
	--Workers Section
		rt(h2(_"Workers")) ..
		rt(h3(_"Crew required:")) ..
		image_line("tribes/barbarians/master-miner/menu.png", 1, p(_"%s and":format(_"Master Miner"))) ..
		image_line("tribes/barbarians/chief-miner/menu.png", 1, p(_"%s or better and":format(_"Chief Miner"))) ..
		image_line("tribes/barbarians/miner/menu.png", 1, p(_"%s or better":format(_"Miner"))) ..
		text_line(_"Workers use:",_"Pick","tribes/barbarians/pick/menu.png") ..
		rt(h3(_"Experience levels:")) ..
		rt("text-align=right", p(_"%s to %s (%s EP)":format(_"Miner",_"Chief Miner","19") .. "<br>" .. _"%s to %s (%s EP)":format(_"Chief Miner",_"Master Miner","28"))) ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"If the food supply is steady, this mine can produce %s in %s on average.":format(_"Iron Ore","17.6s"))
}