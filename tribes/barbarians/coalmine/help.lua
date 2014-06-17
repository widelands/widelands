include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian Coal Mine")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/coalmine/coalmine_i_00.png", p(
			_([[Ages ago, the Barbarians learned to delve into mountainsides for that black material that feeds their furnaces.<br>
]] ..
			[[Wood may serve for a household fire and to keep you warm, but when it comes to working with iron or gold, there is no way around coal.]])
		)) ..
	--General Section
		rt(h2(_"General")) ..
		rt(p(_"A %1$s exploits only %2$s of the resource. From there on out, it will only have a 5%% chance of finding %3$s.":bformat(_"Coal mine","1/3",_"Coal"))) ..
		rt(h3(_"Purpose:")) ..
		image_line("tribes/barbarians/coal/menu.png", 1, p(_"Dig %s out of the ground in mountain terrain.":format(_"Coal"))) ..
		text_line(_"Working radius:", "2") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		rt(h3(_"Incoming:")) ..
		dependencies({"tribes/barbarians/tavern/menu.png","tribes/barbarians/ration/menu.png","tribes/barbarians/coalmine/menu.png"}, p(_"%s from a Tavern":format(_"Ration"))) ..
		dependencies({"tribes/barbarians/inn/menu.png","tribes/barbarians/ration/menu.png","tribes/barbarians/coalmine/menu.png"}, p(_"%s from an Inn":format(_"Ration"))) ..
		dependencies({"tribes/barbarians/big_inn/menu.png","tribes/barbarians/ration/menu.png","tribes/barbarians/coalmine/menu.png"}, p(_"%s from a Big Inn":format(_"Ration"))) ..
		rt(h3(_"Outgoing:")) ..
		dependencies({"tribes/barbarians/resi_coal2/resi_00.png","tribes/barbarians/coalmine/menu.png","tribes/barbarians/coal/menu.png"}, p(_"Coal")) ..
		dependencies({"tribes/barbarians/coal/menu.png","tribes/barbarians/axfactory/menu.png"}, p(_"Axfactory")) ..
		dependencies({"tribes/barbarians/coal/menu.png","tribes/barbarians/warmill/menu.png"}, p(_"War Mill")) ..
		dependencies({"tribes/barbarians/coal/menu.png","tribes/barbarians/helmsmithy/menu.png"}, p(_"Helm Smithy")) ..
		dependencies({"tribes/barbarians/coal/menu.png","tribes/barbarians/smelting_works/menu.png"}, p(_"Smelting Works")) ..
		dependencies({"tribes/barbarians/coal/menu.png","tribes/barbarians/lime_kiln/menu.png"}, p(_"Lime Kiln")) ..
	--Building Section
		rt(h2(_"Building")) ..
		text_line(_"Space required:",_"Mine plot","pics/mine.png") ..
		text_line(_"Upgraded from:", "n/a") ..
		rt(h3(_"Build cost:")) ..
		help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 2), 2) ..
		help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 4), 4) ..
		rt(h3(_"Dismantle yields:")) ..
		help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 1), 1) ..
		help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 2), 2) ..
		text_line(_"Upgradeable to:",_"Deep Coal Mine","tribes/barbarians/deep_coalmine/menu.png") ..
	--Workers Section
		rt(h2(_"Workers")) ..
		rt(h3(_"Crew required:")) ..
		image_line("tribes/barbarians/miner/menu.png", 1, p(_"%s or better":format(_"Miner"))) ..
		text_line(_"Worker uses:",_"Pick","tribes/barbarians/pick/menu.png") ..
		rt(h3(_"Experience levels:")) ..
		rt("text-align=right", p(_"%s to %s (%s EP)":format(_"Miner",_"Chief Miner","19") .. "<br>" .. _"%s to %s (%s EP)":format(_"Chief Miner",_"Master Miner","28"))) ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"If the food supply is steady, this mine can produce %1$s in %2$s on average.":bformat(_"Coal","32.5s"))
}
