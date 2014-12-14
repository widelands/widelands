# Comments are for graphic designers some ideas

[tribe]


# No idea for the frontier. Maybe some javelins?
[frontier]
pics=pics/frontier_??.png
hotspot=1 19

# Not just a plain color, maybe a cross or some stribes
[flag]
pics=pics/flag_??.png
hotspot=14 38
fps=10


[ship types]
ship=_Ship

[constructionsite types]
constructionsite=_Construction Site

[dismantlesite types]
dismantlesite=_Dismantle Site

[warehouse types]
headquarters=_Headquarters
headquarters_shipwreck=_Headquarters Shipwreck
warehouse=_Warehouse
port=_Port


dirname = path.dirname(__file__)

tribes:new_tribe {
   name = "empire",
   author = _"The Widelands Development Team",
   -- TRANSLATORS: This is a tribe name
   descname = _"_Empire",
   helptext = _"This is the culture of the Roman Empire.",
   bob_vision_range = 2,
   uiposition = 20,
   carrier2 = "donkey", -- NOCOM convert
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
			"wool",
			"cloth"
		},
		{
			-- Food
			"fish",
			"meat",
			"water",
			"wheat",
			"flour",
			"bread_empire",
			"beer",
			"grape",
			"wine",
			"ration",
			"meal"
		},
		{
			-- Mining
			"marble",
			"marble_column",
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
			"saw",
			"shovel",
			"hammer",
			"fishing_rod",
			"hunting_spear",
			"scythe",
			"bread_paddle",
			"basket",
			"kitchen_tools",
			"fire_tongs"
		}
		{
			-- Weapons & Armor
			"spear_wooden",
			"spear",
			"spear_advanced",
			"spear_heavy",
			"spear_war",
			"armor_helmet",
			"armor",
			"armor_chain",
			"armor_gilded"
		}
	},
	-- Workers positions in wares windows.
   -- This also gives us the information which workers the tribe uses.
   -- Each subtable is a column in the wares windows.
   workers_order = {
		{
			-- Carriers
			"empire_carrier",
			"empire_donkey",
			"empire_donkeybreeder"
		},
		{
			-- Building Materials
			"empire_stonemason",
			"empire_carpenter",
			"empire_lumberjack",
			"empire_forester",
			"empire_builder",
			"empire_shepherd",
			"empire_weaver",
			"empire_shipwright"
		},
		{
			-- Food
			"empire_fisher",
			"empire_hunter",
			"empire_farmer",
			"empire_miller",
			"empire_baker",
			"empire_brewer",
			"empire_pigbreeder",
			"empire_vinefarmer",
			"empire_innkeeper"
		},
		{
			-- Mining
			"empire_geologist",
			"empire_miner",
			"empire_miner_master",
			"empire_charcoal_burner",
			"empire_smelter"
		}
		{
			-- Tools
			"empire_toolsmith"
		}
		{
			-- Military
			"empire_soldier",
			"empire_trainer",
			"empire_weaponsmith",
			"empire_armorsmith",
			"empire_scout"
		}
	},
   carriers = {
		"empire_carrier",
		"empire_donkey"
   },
   soldiers = {
		"empire_soldier"
   },
   immovables = {
		"ashes",
		"destroyed_building",
		"field_tiny",
		"field_small",
		"field_medium",
		"field_ripe",
		"field_harvested",
		"grapevine_tiny",
		"grapevine_small",
		"grapevine_medium",
		"grapevine_ripe",
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
		"shipconstruction_empire",
	},
	militarysites = {
		"empire_barrier",
		"empire_blockhouse",
		"empire_castle",
		"empire_fortress",
		"empire_outpost",
		"empire_sentry",
		"empire_tower",
	},
	trainingsites = {
		"empire_arena",
		"empire_colosseum",
		"empire_trainingcamp",
	},
   productionsites = {
		-- Carriers
		"empire_donkeyfarm",

		-- Building Materials
		"empire_foresters_house",
		"empire_lumberjacks_house",
		"empire_quarry",
		"empire_sawmill",
		"empire_stonemasons_house",
		"empire_sheepfarm",
		"empire_weaving_mill",

		-- Food
		"empire_bakery",
		"empire_brewery",
		"empire_farm",
		"empire_fishers_house",
		"empire_hunters_house",
		"empire_inn",
		"empire_mill",
		"empire_piggery",
		"empire_tavern",
		"empire_vineyard",
		"empire_well",
		"empire_winery",

		-- Mining
		"empire_charcoal_kiln",
		"empire_coalmine",
		"empire_coalmine_deep",
		"empire_goldmine",
		"empire_goldmine_deep",
		"empire_ironmine",
		"empire_ironmine_deep",
		"empire_marblemine",
		"empire_marblemine_deep",
		"empire_smelting_works",

		-- Tools, Military & Ships
		"empire_armorsmithy",
		"empire_scouts_house",
		"empire_shipvard",
		"empire_toolsmithy",
		"empire_weaponsmithy",
	},
}
