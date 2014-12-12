# Comments are for graphic designers some ideas

[tribe]

# Workers positions in wares window. Columns are separated by ;,
# Entries in the columns separated by ,
workers_order=carrier, ox, cattlebreeder; stonemason, lumberjack, ranger, builder, lime-burner, gardener, weaver, shipwright; fisher , hunter , gamekeeper, farmer, baker, brewer, master-brewer, innkeeper; geologist,miner, chief-miner, master-miner, charcoal_burner, smelter; blacksmith, master-blacksmith; soldier, trainer, helmsmith, scout


[frontier]
pics=pics/frontier_??.png
hotspot=1 19


[flag]
pics=pics/flag_??.png
hotspot=10 38
fps=5

[carrier types]
carrier=_Carrier
ox=_Ox

[soldier types]
soldier=_Soldier

[worker types]
baker=_Baker
master-blacksmith=_Master Blacksmith
blacksmith=_Blacksmith
master-brewer=_Master Brewer
brewer=_Brewer
builder=_Builder
charcoal_burner=_Charcoal Burner
master-miner=_Master Miner
chief-miner=_Chief Miner
farmer=_Farmer
gardener=_Gardener
fisher=_Fisher
gamekeeper=_Gamekeeper
geologist=_Geologist
helmsmith=_Helmsmith
hunter=_Hunter
innkeeper=_Innkeeper
lime-burner=_Lime-Burner
lumberjack=_Lumberjack
miner=_Miner
ranger=_Ranger
shipwright=_Shipwright
smelter=_Smelter
stonemason=_Stonemason
trainer=_Trainer
weaver=_Weaver
scout=_Scout
cattlebreeder=_Cattle Breeder

[ship types]
ship=_Ship

[constructionsite types]
constructionsite=_Construction Site

[dismantlesite types]
dismantlesite=_Dismantle Site

[global militarysite types]
castle.atlanteans=_Castle
guardhall.atlanteans=_Guardhall
guardhouse.atlanteans=_Guardhouse
high_tower.atlanteans=_High Tower
small_tower.atlanteans=_Small Tower
tower.atlanteans=_Tower
sentry.empire=_Sentry
barracks.empire=_Barracks
barrier.empire=_Barrier
castle.empire=_Castle
fortress.empire=_Fortress
outpost.empire=_Outpost
tower.empire=_Tower

[warehouse types]
headquarters=_Headquarters
headquarters_interim=_Headquarters
warehouse=_Warehouse
port=_Port

[productionsite types]
quarry=_Quarry
lumberjacks_hut=_Lumberjack’s Hut
rangers_hut=_Ranger’s Hut
fishers_hut=_Fisher’s Hut
hunters_hut=_Hunter’s Hut
gamekeepers_hut=_Gamekeeper’s Hut
well=_Well
scouts_hut=_Scout’s Hut
hardener=_Wood Hardener
lime_kiln=_Lime Kiln
reed_yard=_Reed Yard
bakery=_Bakery
brewery=_Brewery
micro-brewery=_Micro Brewery
big_inn=_Big Inn
inn=_Inn
tavern=_Tavern
charcoal_kiln=_Charcoal Kiln
smelting_works=_Smelting Works
shipyard=_Shipyard
warmill=_War Mill
axfactory=_Axfactory
metalworks=_Metal Workshop
cattlefarm=_Cattle Farm
weaving-mill=_Weaving Mill
farm=_Farm
helmsmithy=_Helm Smithy
granitemine=_Granite Mine
deeper_coalmine=_Deeper Coal Mine
deep_coalmine=_Deep Coal Mine
coalmine=_Coal Mine
deeper_oremine=_Deeper Iron Ore Mine
deep_oremine=_Deep Iron Ore Mine
oremine=_Iron Ore Mine
deeper_goldmine=_Deeper Gold Mine
deep_goldmine=_Deep Gold Mine
goldmine=_Gold Mine

[militarysite types]
barrier=_Barrier
sentry=_Sentry
citadel=_Citadel
fortress=_Fortress
donjon=_Donjon

[trainingsite types]
battlearena=_Battle Arena
trainingcamp=_Training Camp



dirname = path.dirname(__file__)

tribes:new_tribe {
   name = "barbarians",
   author = _"The Widelands Development Team",
   -- TRANSLATORS: This is a tribe name
   descname = _"_Barbarians",
   helptext = _"The mighty tribes of the east-lands.",
   bob_vision_range = 2,
   uiposition = 10,
   carrier2 = "ox", -- NOCOM convert
   icon = pics/icon.png, -- NOCOM convert

   -- Wares positions in wares windows.
   -- This also gives us the information which wares the tribe uses.
   -- Each subtable is a column in the wares windows.
   wares_order = {
		{
			-- Building Materials
			"granite",
			"log",
			"blackwood",
			"grout",
			"thatch_reed",
			"cloth"
		},
		{
			-- Food
			 "fish",
			 "meat",
			 "water",
			 "wheat",
			 "bread_barbarians",
			 "beer",
			 "stout",
			 "ration",
			 "snack",
			 "meal"
		},
		{
			-- Mining
			 "coal",
			 "iron_ore",
			 "iron",
			 "gold_ore",
			 "gold"
		}
		{
			-- Tools
			 "pick",
			 "felling_ax",
			 "shovel",
			 "hammer",
			 "fishing_rod",
			 "hunting_spear",
			 "scythe",
			 "bread_paddle",
			 "kitchen_tools",
			 "fire_tongs"
		}
		{
			-- Weapons & Armor
			 "ax",
			 "ax_sharp",
			 "ax_broad",
			 "ax_bronze",
			 "ax_battle",
			 "ax_warriors",
			 "helmet",
			 "helmet_mask",
			 "helmet_warhelm"
		}
	},
   immovables = {
		"ashes",
		"destroyed_building",
		"field_tiny",
		"field_small",
		"field_medium",
		"field_ripe",
		"field_harvested",
		"reed_tiny",
		"reed_small",
		"reed_medium",
		"reed_ripe",
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
		"shipconstruction_barbarians",
	}
}
