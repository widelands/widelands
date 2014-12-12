# Comments are for graphic designers some ideas

[tribe]

# Workers positions in wares window. Columns are separated by ;,
# Entries in the columns separated by ,
workers_order=carrier, horse, horsebreeder; stonecutter, woodcutter, sawyer, forester, builder, spiderbreeder, weaver, shipwright; fisher, fish_breeder, hunter, smoker, farmer, blackroot_farmer, miller, baker; geologist, miner, charcoal_burner, smelter; toolsmith; soldier, trainer, weaponsmith, armorsmith, scout

# Some blue fires would be fine, but just an idea
[frontier]
pics=pics/frontier_??.png
hotspot=3 12

# Not just a plain color, maybe a cross or some stribes
[flag]
pics=pics/flag_??.png
hotspot=15 35
fps=10

[carrier types]
carrier=_Carrier
horse=_Horse

[soldier types]
soldier=_Soldier

[worker types]
armorsmith=_Armorsmith
baker=_Baker
blackroot_farmer=_Blackroot Farmer
builder=_Builder
charcoal_burner=_Charcoal Burner
shipwright=_Shipwright
farmer=_Farmer
fish_breeder=_Fish Breeder
fisher=_Fisher
forester=_Forester
geologist=_Geologist
hunter=_Hunter
miller=_Miller
miner=_Miner
sawyer=_Sawyer
smelter=_Smelter
smoker=_Smoker
spiderbreeder=_Spider Breeder
stonecutter=_Stonecutter
toolsmith=_Toolsmith
trainer=_Trainer
weaponsmith=_Weaponsmith
weaver=_Weaver
woodcutter=_Woodcutter
scout=_Scout
horsebreeder=_Horse Breeder

[ship types]
ship=_Ship

[constructionsite types]
constructionsite=_Construction Site

[dismantlesite types]
dismantlesite=_Dismantle Site

[global militarysite types]
barrier.barbarians=_Barrier
citadel.barbarians=_Citadel
donjon.barbarians=_Donjon
fortress.barbarians=_Fortress
sentry.barbarians=_Sentry
sentry.empire=_Sentry
barracks.empire=_Barracks
barrier.empire=_Barrier
castle.empire=_Castle
fortress.empire=_Fortress
outpost.empire=_Outpost
tower.empire=_Tower

[warehouse types]
headquarters=_Headquarters
warehouse=_Warehouse
port=_Port

[productionsite types]
quarry=_Quarry
woodcutters_house=_Woodcutter’s House
foresters_house=_Forester’s House
gold-spinning-mill=_Gold Spinning Mill
fishers_house=_Fisher’s House
fish_breeders_house=_Fish Breeder’s House
hunters_house=_Hunter’s House
well=_Well
scouts_house=_Scout’s House
sawmill=_Sawmill
smokery=_Smokery
mill=_Mill
shipyard=_Shipyard
bakery=_Bakery
charcoal_kiln=_Charcoal Kiln
smelting_works=_Smelting Works
toolsmithy=_Toolsmithy
weaponsmithy=_Weapon Smithy
armorsmithy=_Armor Smithy
horsefarm=_Horse Farm
spiderfarm=_Spider Farm
weaving-mill=_Weaving Mill
farm=_Farm
blackroot_farm=_Blackroot Farm
crystalmine=_Crystal Mine
coalmine=_Coal Mine
ironmine=_Iron Mine
goldmine=_Gold Mine


[militarysite types]
guardhouse=_Guardhouse
guardhall=_Guardhall
high_tower=_High Tower
small_tower=_Small Tower
tower=_Tower
castle=_Castle


[trainingsite types]
dungeon=_Dungeon
labyrinth=_Labyrinth


dirname = path.dirname(__file__)

tribes:new_tribe {
   name = "atlanteans",
   author = _"The Widelands Development Team",
   -- TRANSLATORS: This is a tribe name
   descname = _"Atlanteans",
   helptext = _"This tribe is known from the oldest tales. The sons and daughters of Atlantis.",
   bob_vision_range = 2,
   uiposition = 100,
   carrier2 = "horse", -- NOCOM convert
   icon = pics/icon.png, -- NOCOM convert

   -- Wares positions in wares windows.
   -- This also gives us the information which wares the tribe uses.
   -- Each subtable is a column in the wares windows.
   wares_order = {
		{
			-- Building Materials
			"granite",
			"log",
			"planks",
			"spider_silk",
			"spidercloth"
		},
		{
			-- Food
			"fish",
			"smoked_fish",
			"meat",
			"smoked_meat",
			"water",
			"corn",
			"cornmeal",
			"blackroot",
			"blackroot_flour",
			"bread_atlanteans"
		},
		{
			-- Mining
			"quartz",
			"diamond",
			"coal",
			"iron_ore",
			"iron",
			"gold_ore",
			"gold"
		}
		{
			-- Tools
			"pick",
			"saw",
			"shovel",
			"hammer",
			"milking_tongs",
			"fishing_net",
			"bucket",
			"hunting_bow",
			"hook_pole",
			"scythe",
			"bread_paddle",
			"fire_tongs"
		}
		{
			-- Weapons & Armor
			"trident_light",
			"trident_long",
			"trident_steel",
			"trident_double",
			"trident_heavy_double",
			"shield_steel",
			"shield_advanced",
			"tabard",
			"gold_thread",
			"tabard_golden"
		}
	},
   immovables = {
		"ashes",
		"blackrootfield_tiny",
		"blackrootfield_small",
		"blackrootfield_medium",
		"blackrootfield_ripe",
		"blackrootfield_harvested",
		"cornfield_tiny",
		"cornfield_small",
		"cornfield_medium",
		"cornfield_ripe",
		"cornfield_harvested",
		"destroyed_building",
		"resi_coal1",
		"resi_coal2",
		"resi_gold1",
		"resi_gold2",
		"resi_iron1",
		"resi_iron2",
		"resi_none",
		"resi_water",
		"resi_stones1",
		"resi_stones2",
		"shipconstruction_atlanteans",
	}
}
