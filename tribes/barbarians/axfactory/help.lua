include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian Axfactory")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/axfactory/axfactory_i_00.png", p(
			_[[‘A new Warrior’s Ax brings forth the best in its wielder – or the worst in its maker.’]])) ..
		rt("text-align=right",p("font-size=10 font-style=italic", _[[An old Barbarian proverb<br> meaning that you need to take some risks sometimes.]])) ..
	--General Section
		rt(h2(_"General")) ..
		text_line(_"Purpose:", _"The Barbarian Axfactory is the intermediate production site in a series of three buildings. It is an upgrade from the Metal Workshop but doesn’t require additional qualification for the worker.") ..
		rt("image=tribes/barbarians/ax/menu.png;tribes/barbarians/sharpax/menu.png;tribes/barbarians/broadax/menu.png image-align=right",p(_"It can make axes, Sharp axes and Broad axes.")) ..
		text_line(_"Working radius:", "n/a") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		rt(h3(_"Incoming:")) ..
		dependencies({"tribes/barbarians/smelting_works/menu.png","tribes/barbarians/iron/menu.png","tribes/barbarians/axfactory/menu.png"},p(_"%s from the Smelting Works":format(_"Iron"))) ..
		dependencies({"tribes/barbarians/coalmine/menu.png;tribes/barbarians/burners_house/menu.png","tribes/barbarians/coal/menu.png","tribes/barbarians/axfactory/menu.png"}) ..
		rt(p(_"%s from the Coal mine or the Charcoal Burner’s House":format(_"Coal"))) ..
		rt(h3(_"Outgoing:")) ..
		dependencies({"tribes/barbarians/axfactory/menu.png","tribes/barbarians/ax/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/soldier/menu.png"},p(_"Soldier")) ..
		dependencies({"tribes/barbarians/axfactory/menu.png","tribes/barbarians/sharpax/menu.png;tribes/barbarians/broadax/menu.png","tribes/barbarians/trainingcamp/menu.png"},p(_"Training Camp")) ..
	--Building Section
		rt(h2(_"Building")) ..
		text_line(_"Space required:",_"Medium plot","pics/medium.png") ..
		text_line(_"Upgraded from:", _"Metal Workshop","tribes/barbarians/metalworks/menu.png") ..
		rt(h3(_"Upgrade cost:")) ..
		help_building_line("barbarians", "blackwood", ngettext("%i Blackwood", "%i Blackwood", 1), 1) ..
		help_building_line("barbarians", "thatchreed", ngettext("%i Thatch Reed", "%i Thatch Reeds", 1), 1) ..
		help_building_line("barbarians", "grout", ngettext("%i Grout", "%i Grout", 1), 1) ..
		help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 2), 2) ..
		help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 1), 1) ..
		rt(h3(_"Cost cumulative:")) ..
		help_building_line("barbarians", "blackwood", ngettext("%i Blackwood", "%i Blackwood", 2), 2) ..
		help_building_line("barbarians", "thatchreed", ngettext("%i Thatch Reed", "%i Thatch Reeds", 2), 2) ..
		help_building_line("barbarians", "grout", ngettext("%i Grout", "%i Grout", 2), 2) ..
		help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 4), 4) ..
		help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 2), 2) ..
		rt(h3(_"Dismantle yields:")) ..
		help_building_line("barbarians", "blackwood", ngettext("%i Blackwood", "%i Blackwood", 1), 1) ..
		help_building_line("barbarians", "thatchreed", ngettext("%i Thatch Reed", "%i Thatch Reeds", 1), 1) ..
		help_building_line("barbarians", "grout", ngettext("%i Grout", "%i Grout", 1), 1) ..
		help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 2), 2) ..
		help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 1), 1) ..
		text_line(_"Upgradeable to:",_"War Mill","tribes/barbarians/warmill/menu.png") ..
	--Workers Section
		rt(h2(_"Workers")) ..
		rt(h3(_"Crew required:")) ..
		image_line("tribes/barbarians/blacksmith/menu.png", 1, p(_"%s or better":format(_"Blacksmith"))) ..
		text_line(_"Worker uses:",_"Hammer","tribes/barbarians/hammer/menu.png") ..
		rt(h3(_"Experience levels:")) ..
		dependencies({"tribes/barbarians/blacksmith/menu.png","tribes/barbarians/master-blacksmith/menu.png"}) ..
		rt("text-align=right", p(_"%s to %s (%s EP)":format(_"Blacksmith",_"Master Blacksmith","24"))) ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"If all needed wares are delivered in time, this building can produce %1$s in about %2$s on average.":bformat(_"each item","57s")) ..
		rt(p(_"All three weapons take the same time for making, but the required raw materials vary."))
}
