# Comments are for graphic designers some ideas

[tribe]

[frontier]
pics=pics/frontier_??.png
hotspot=1 19


[flag]
pics=pics/flag_??.png
hotspot=10 38
fps=5


[ship types]
ship=_Ship

[constructionsite types]
constructionsite=_Construction Site

[dismantlesite types]
dismantlesite=_Dismantle Site

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

	-- Workers positions in wares windows.
   -- This also gives us the information which workers the tribe uses.
   -- Each subtable is a column in the wares windows.
   -- NOCOM(#GunChleoc): We have worker, carrier and solder types here.
   -- Maybe we need an extra list after all.
   workers_order = {
		{
			-- Carriers
			"barbarians_carrier",
			"barbarians_ox",
			"barbarians_cattlebreeder"
		},
		{
			-- Building Materials
			"barbarians_stonemason",
			"barbarians_lumberjack",
			"barbarians_ranger",
			"barbarians_builder",
			"barbarians_lime_burner",
			"barbarians_gardener",
			"barbarians_weaver",
			"barbarians_shipwright"
		},
		{
			-- Food
			"barbarians_fisher",
			"barbarians_hunter",
			"barbarians_gamekeeper",
			"barbarians_farmer"
			"barbarians_baker",
			"barbarians_brewer",
			"barbarians_brewer_master",
			"barbarians_innkeeper"
		},
		{
			-- Mining
			"barbarians_geologist",
			"barbarians_miner",
			"barbarians_miner_chief"
			"barbarians_miner_master",
			"barbarians_charcoal_burner",
			"barbarians_smelter"
		}
		{
			-- Tools
			"barbarians_blacksmith",
			"barbarians_blacksmith_master"
		}
		{
			-- Military
			"barbarians_soldier",
			"barbarians_trainer",
			"barbarians_helmsmith",
			"barbarians_scout"
		}
	},
   carriers = {
		"barbarians_carrier",
		"barbarians_ox"
   },
   soldiers = {
		"barbarians_soldier",
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
	},
	militarysites = {
		"barbarians_barrier",
		"barbarians_citadel",
		"barbarians_fortress",
		"barbarians_sentry",
		"barbarians_tower",
	},
	trainingsites = {
		"barbarians_battlearena",
		"barbarians_trainingcamp",
	}
}
