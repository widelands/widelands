include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return
		--rt(h1(_"The Barbarian Training Camp")) ..
	--Lore Section
	building_help_lore_string("barbarians", building_description, _[[‘He who is strong shall neither forgive nor forget, but revenge injustice suffered – in the past and for all future.’]], _[[Chief Chat’Karuth in a speech to his army.]]) ..

	--General Section TODO string design
	building_help_general_string("barbarians", building_description, "soldier",
		rt(_"Trains soldiers in Attack up to level 5, and in Health up to level 3.<br>Equips the soldiers with all necessary weapons and armor parts.") ..
		rt("_</p><p font-weight=bold font-decoration=underline font-size=12>" .. _"Note:" .. "</p><p font-size=12>" ..
		   _"Barbarian soldiers cannot be trained in ‘Defense’ and will remain at their initial level."
		   .. "<br><br></p>"),
		_"Trains soldiers in Evade up to level 2.",
		"--") ..

	--Dependencies TODO
		rt(h2(_"Dependencies")) ..
		dependencies_old({"tribes/barbarians/soldier/untrained.png","tribes/barbarians/trainingcamp/menu.png","tribes/barbarians/soldier/fulltrained-evade.png"}) ..
		dependencies_old({"tribes/barbarians/soldier/untrained+evade.png","tribes/barbarians/trainingcamp/menu.png","tribes/barbarians/soldier/fulltrained.png"}) ..
		rt(h3(_"Training Attack:")) ..
		dependencies_old({"tribes/barbarians/axfactory/menu.png","tribes/barbarians/sharpax/menu.png;tribes/barbarians/broadax/menu.png","tribes/barbarians/trainingcamp/menu.png"}) ..
		rt(p(_"Provided by the Axfactory")) ..
		dependencies_old({"tribes/barbarians/warmill/menu.png","tribes/barbarians/sharpax/menu.png;tribes/barbarians/broadax/menu.png;tribes/barbarians/bronzeax/menu.png;tribes/barbarians/battleax/menu.png;tribes/barbarians/warriorsax/menu.png","tribes/barbarians/trainingcamp/menu.png"}) ..
		rt(p(_"Provided by the War Mill")) ..
		rt(h3(_"Training Health:")) ..
		dependencies_old({"tribes/barbarians/helmsmithy/menu.png","tribes/barbarians/helm/menu.png;tribes/barbarians/mask/menu.png;tribes/barbarians/warhelm/menu.png","tribes/barbarians/trainingcamp/menu.png"}) ..
		rt(p(_"Provided by the Helm Smithy")) ..
		rt(h3(_"Training Both")) ..
		image_line("tribes/barbarians/pittabread/menu.png",1,p(_"%s and":format(_"Pitta Bread"))) ..
		image_line("tribes/barbarians/fish/menu.png;tribes/barbarians/meat/menu.png",1,p(_"%s or %s":format(_"Fish",_"Meat"))) ..
	--Building Section
	building_help_building_section("barbarians", building_description) ..

		--rt(h2(_"Building")) ..
		--text_line(_"Upgraded from:", "n/a") ..
		--rt(h3(_"Build cost:")) ..
		--help_building_line("barbarians", "gold", ngettext("%i Gold", "%i Gold", 4), 4) ..
		--help_building_line("barbarians", "thatchreed", ngettext("%i Thatch Reed", "%i Thatch Reeds", 3), 3) ..
		--help_building_line("barbarians", "grout", ngettext("%i Grout", "%i Grout", 6), 6) ..
		--help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 4), 4) ..
		--help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 6), 6) ..
		--rt(h3(_"Dismantle yields:")) ..
		--help_building_line("barbarians", "gold", ngettext("%i Gold", "%i Gold", 2), 2) ..
		--help_building_line("barbarians", "thatchreed", ngettext("%i Thatch Reed", "%i Thatch Reeds", 2), 2) ..
		--help_building_line("barbarians", "grout", ngettext("%i Grout", "%i Grout", 3), 3) ..
		--help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 2), 2) ..
		--help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 3), 3) ..
		--text_line(_"Upgradeable to:","n/a") ..
	--Workers Section
	building_help_crew_string("barbarians", building_description, {"trainer"}) ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"If all needed wares are delivered in time, a training camp can train one new soldier in %1$s and %2$s to the final level in %3$s on average.":bformat(_"attack",_"health",_"%1$im%2$is":bformat(4,40)))
   end
}
