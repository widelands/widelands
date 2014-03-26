include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian Deeper Coal Mine")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/deeper_coalmine/deeper_coalmine_i_00.png", p(--text identical to coalmine
			_([[Ages ago, the Barbarians learned to delve into mountainsides for that black material that feeds their furnaces.<br>
]] ..
			[[Wood may serve for a household fire and to keep you warm, but when it comes to working with iron or gold, there is no way around coal.]])
		)) ..
	--General Section
		rt(h2(_"General")) ..
		rt(p(_"A %1$s exploits all of the resource down to the deepest level. But even after having done so, it will still have a %2$s chance of finding some more %3$s.":bformat(_"Deeper Coal Mine","10%",_"Coal"))) ..
		rt(h3(_"Purpose:")) ..
		image_line("tribes/barbarians/coal/menu.png", 1, p(_"Dig %s out of the ground in mountain terrain.":format(_"Coal"))) ..
		text_line(_"Working radius:", "2") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		rt(h3(_"Incoming:")) ..
		dependencies({"tribes/barbarians/big_inn/menu.png","tribes/barbarians/meal/menu.png","tribes/barbarians/deeper_coalmine/menu.png"}, p(_"%s from a Big Inn":format(_"Meal"))) ..
		rt(h3(_"Outgoing:")) ..
		dependencies({"tribes/barbarians/resi_coal2/resi_00.png","tribes/barbarians/deeper_coalmine/menu.png","tribes/barbarians/coal/menu.png"}, p(_"Coal")) ..
		dependencies({"tribes/barbarians/coal/menu.png","tribes/barbarians/axfactory/menu.png"}, p(_"Axfactory")) ..
		dependencies({"tribes/barbarians/coal/menu.png","tribes/barbarians/warmill/menu.png"}, p(_"War Mill")) ..
		dependencies({"tribes/barbarians/coal/menu.png","tribes/barbarians/helmsmithy/menu.png"}, p(_"Helm Smithy")) ..
		dependencies({"tribes/barbarians/coal/menu.png","tribes/barbarians/smelting_works/menu.png"}, p(_"Smelting Works")) ..
		dependencies({"tribes/barbarians/coal/menu.png","tribes/barbarians/lime_kiln/menu.png"}, p(_"Lime Kiln")) ..
	--Building Section
		rt(h2(_"Building")) ..
		text_line(_"Space required:",_"Mine plot","pics/mine.png") ..
		text_line(_"Upgraded from:",_"Deep Coal Mine","tribes/barbarians/deep_coalmine/menu.png") ..
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
		text_line(_"Performance:", _"If the food supply is steady, this mine can produce %s in %s on average.":format(_"Coal","14.4s"))
}
