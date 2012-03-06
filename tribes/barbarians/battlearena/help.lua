use("aux", "formatting")
use("aux", "format_help")

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian Battle Arena")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/battlearena/battlearena_i_00.png", p(
			_[["No better friend you have in battle than the the enemy's blow that misses."]])) ..
		rt("text-align=right",p("font-size=10 font-style=italic", [[Said to originate from Neidhardt, the famous trainer.]])) ..
	--General Section
		rt(h2(_"General")) ..
		text_line(_"Purpose:", _"Trains soldiers in Evade up to level 2.") ..
		rt(p(_[["Evade" increases the soldier's chance not to be hit by the enemy and so to remain totally unaffected.]])) ..
		rt("_<p font-weight=bold font-decoration=underline font-size=12>Note:</p>_<p font-size=12> Barbarian soldiers cannot be trained in \"Defense\" and will remain at the level with which they came.<br><br></p>") ..
		text_line(_"Working radius:", "n/a") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		image_2_arrow("pics/soldier_untrained.png","tribes/barbarians/battlearena/menu.png","pics/soldier_untrained+evade.png") ..
		image_2_arrow("pics/soldier_fulltrained-evade.png","tribes/barbarians/battlearena/menu.png","pics/soldier_fulltrained.png") ..
		rt(h3(_"Evade Training:")) ..
		image_line("tribes/barbarians/pittabread/menu.png",1,p(_"Pitta Bread" .. _" and")) ..
		image_line("tribes/barbarians/strongbeer/menu.png",1,p(_"Strong Beer" .. _" and")) ..
		image_line("tribes/barbarians/fish/menu.png;tribes/barbarians/meat/menu.png",1,p(_"Fish" .. _" or " .. _"Meat")) ..
	--Building Section
		rt(h2(_"Building")) ..
		text_line(_"Space required:",_"Big plot","pics/big.png") ..
		text_line(_"Upgraded from:", "n/a") ..
		rt(h3(_"Build Cost:")) ..
		image_line("tribes/barbarians/gold/menu.png",4,p("4 " .. _"Gold")) ..
		image_line("tribes/barbarians/thatchreed/menu.png",3,p("3 " .. _"Thatch Reed")) ..
		image_line("tribes/barbarians/raw_stone/menu.png",4,p("4 " .. _"Raw Stone")) ..
		image_line("tribes/barbarians/grout/menu.png",6,p("6 " .. _"Grout")) ..
		image_line("tribes/barbarians/trunk/menu.png",6,p("6 " .. _"Trunk")) ..
		rt(h3(_"Dismantle yields:")) ..
		image_line("tribes/barbarians/gold/menu.png",2,p("2 " .. _"Gold")) ..
		image_line("tribes/barbarians/thatchreed/menu.png",2,p("2 " .. _"Thatch Reed")) ..
		image_line("tribes/barbarians/raw_stone/menu.png",2,p("2 " .. _"Raw Stone")) ..
		image_line("tribes/barbarians/grout/menu.png",3,p("3 " .. _"Grout")) ..
		image_line("tribes/barbarians/trunk/menu.png",3,p("3 " .. _"Trunk")) ..
		text_line(_"Upgradeable to:","n/a") ..
	--Workers Section
		rt(h2(_"Workers")) ..
		rt(h3(_"Crew required:")) ..
		image_line("tribes/barbarians/trainer/menu.png", 1, p(_"Trainer")) ..
		text_line(_"Worker uses:","n/a") ..
		text_line(_"Experience levels:", "n/a") ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"If all needed wares are delivered in time, a Battle Arena can train evade for one soldier from 0 to the highest in level in 1m10s on average.")
}