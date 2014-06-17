include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian Deeper Gold Mine")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/deeper_goldmine/deeper_goldmine_i_00.png", p(--text identical to goldmine
			_[[‘Soft and supple.<br> And yet untouched by time and weather.<br> Rays of sun, wrought into eternity ...’]])) ..
		rt("text-align=right",p("font-size=10 font-style=italic", _[[Excerpt from ‘Our Treasures Underground’,<br> a traditional Barbarian song.]])) ..
	--General Section
		rt(h2(_"General")) ..
		rt(p(_"A %1$s exploits all of the resource down to the deepest level. But even after having done so, it will still have a %2$s chance of finding some more %3$s.":bformat(_"Deeper Gold Mine","10%",_"Gold Ore"))) ..
		rt(h3(_"Purpose:")) ..
		image_line("tribes/barbarians/goldore/menu.png", 1, p(_"Dig %s out of the ground in mountain terrain.":format(_"Gold Ore"))) ..
		text_line(_"Working radius:", "2") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		rt(h3(_"Incoming:")) ..
		dependencies({"tribes/barbarians/big_inn/menu.png","tribes/barbarians/meal/menu.png","tribes/barbarians/deeper_goldmine/menu.png"}, p(_"%s from a Big Inn":format(_"Meal"))) ..
		rt(h3(_"Outgoing:")) ..
		dependencies({"tribes/barbarians/resi_gold2/resi_00.png","tribes/barbarians/deeper_goldmine/menu.png","tribes/barbarians/goldore/menu.png"}, p(_"Gold Ore")) ..
		dependencies({"tribes/barbarians/goldore/menu.png","tribes/barbarians/smelting_works/menu.png"}, p(_"Smelting Works")) ..
		rt(p(_"%s always goes to the %s. It has no other use.":format(_"Gold Ore",_"Smelting Works"))) ..
	--Building Section
		rt(h2(_"Building")) ..
		text_line(_"Space required:",_"Mine plot","pics/mine.png") ..
		text_line(_"Upgraded from:",_"Deep Gold Mine","tribes/barbarians/deep_goldmine/menu.png") ..
		rt(h3(_"Upgrade cost:")) ..
		help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 2), 2) ..
		help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 4), 4) ..
		rt(h3(_"Cost cumulative:")) ..
		help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 6), 6) ..
		help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 12), 12) ..
		rt(h3(_"Dismantle yields:")) ..
		help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 3), 3) ..
		help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 6), 6) ..
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
		text_line(_"Performance:", _"If the food supply is steady, this mine can produce %s in %s on average.":format(_"Gold Ore","18.5s"))
}
