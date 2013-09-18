use("aux", "formatting")
use("aux", "format_help")

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian Deep Gold Mine")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/deep_goldmine/deep_goldmine_i_00.png", p(--text identical to goldmine
			_[["Soft and supple.<br> And yet untouched by time and weather.<br> Rays of sun, wrought into eternity ..."]])) ..
		rt("text-align=right",p("font-size=10 font-style=italic", _[[Excerpt from "Our Treasures Underground",<br> a traditional Barbarian song.]])) ..
	--General Section
		rt(h2(_"General")) ..
		rt(p(_"A %s exploits only %s of the resource.<br>From there on, it will only have a 5%% chance of finding %s.":format(_"Deep Gold Mine","2/3",_"Gold Ore"))) ..
		rt(h3(_"Purpose:")) ..
		image_line("tribes/barbarians/goldstone/menu.png", 1, p(_"Dig %s out of the ground in mountain terrain.":format(_"Gold Ore"))) ..
		text_line(_"Working radius:", "2") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		rt(h3(_"Incoming:")) ..
		dependencies({"tribes/barbarians/inn/menu.png","tribes/barbarians/snack/menu.png","tribes/barbarians/deep_goldmine/menu.png"}, p(_"%s from an Inn":format(_"Snack"))) ..
		dependencies({"tribes/barbarians/big_inn/menu.png","tribes/barbarians/snack/menu.png","tribes/barbarians/deep_goldmine/menu.png"}, p(_"%s from a Big Inn":format(_"Snack"))) ..
		rt(h3(_"Outgoing:")) ..
		dependencies({"tribes/barbarians/resi_gold2/resi_00.png","tribes/barbarians/deep_goldmine/menu.png","tribes/barbarians/goldstone/menu.png"}, p(_"Gold Ore")) ..
		dependencies({"tribes/barbarians/goldstone/menu.png","tribes/barbarians/smelting_works/menu.png"}, p(_"Smelting Works")) ..
		rt(p(_"%s always goes to the %s. It has no other use.":format(_"Gold Ore",_"Smelting Works"))) ..
	--Building Section
		rt(h2(_"Building")) ..
		text_line(_"Space required:",_"Mine plot","pics/mine.png") ..
		text_line(_"Upgraded from:",_"Gold Mine","tribes/barbarians/goldmine/menu.png") ..
		rt(h3(_"Upgrade cost:")) ..
		image_line("tribes/barbarians/raw_stone/menu.png", 2, p("2 " .. _"Raw Stone")) ..
		image_line("tribes/barbarians/trunk/menu.png", 4, p("4 " .. _"Trunk")) ..
		rt(h3(_"Cost cumulative:")) ..
		image_line("tribes/barbarians/raw_stone/menu.png", 4, p("4 " .. _"Raw Stone")) ..
		image_line("tribes/barbarians/trunk/menu.png", 8, p("8 " .. _"Trunk")) ..
		rt(h3(_"Dismantle yields:")) ..
		image_line("tribes/barbarians/raw_stone/menu.png", 2, p("2 " .. _"Raw Stone")) ..
		image_line("tribes/barbarians/trunk/menu.png", 4, p("4 " .. _"Trunk")) ..
		text_line(_"Upgradeable to:",_"Deeper Gold Mine","tribes/barbarians/deeper_goldmine/menu.png") ..
	--Workers Section
		rt(h2(_"Workers")) ..
		rt(h3(_"Crew required:")) ..
		image_line("tribes/barbarians/chief-miner/menu.png", 1, p(_"%s or better":format(_"Chief Miner"))) ..
		image_line("tribes/barbarians/miner/menu.png", 1, p(_"%s or better":format(_"Miner"))) ..
		text_line(_"Workers use:",_"Pick","tribes/barbarians/pick/menu.png") ..
		rt(h3(_"Experience levels:")) ..
		rt("text-align=right", p(_"%s to %s (%s EP)":format(_"Miner",_"Chief Miner","19") .. "<br>" .. _"%s to %s (%s EP)":format(_"Chief Miner",_"Master Miner","28"))) ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"If the food supply is steady, this mine can produce %s in %s on average.":format(_"Gold Ore","19.5s"))
}