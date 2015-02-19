dirname = path.dirname(__file__)

tribes:new_tribe {
   name = "barbarians",

   animations = {
		frontier = {
			pictures = { dirname .. "images/barbarians/frontier_\\d+.png" },
			hotspot = { 1, 19 },
		},
		flag = {
			pictures = { dirname .. "images/barbarians/flag_\\d+.png" },
			hotspot = { 10, 38 },
			fps = 5
		}
	},

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
		},
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
		},
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

	-- Workers positions in workers windows.
   -- This also gives us the information which workers the tribe uses.
   -- Each subtable is a column in the workers windows.
   workers_order = {
		{
			-- Carriers
			"barbarians_builder",
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

	builder = "barbarians_builder",
	carrier = "barbarians_carrier",
	carrier2 = "barbarians_ox",
	geologist = "barbarians_geologist",
   soldier = "barbarians_soldier",
   ship = "ship_barbarians",

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
	},
   productionsites = {
		-- Carriers
		"barbarians_cattlefarm",

		-- Building Materials
		"barbarians_lime_kiln",
		"barbarians_lumberjacks_hut",
		"barbarians_quarry",
		"barbarians_rangers_hut",
		"barbarians_reed_yard",
		"barbarians_wood_hardener",

		-- Food
		"barbarians_bakery",
		"barbarians_big_inn",
		"barbarians_brewery",
		"barbarians_farm",
		"barbarians_fishers_hut",
		"barbarians_gamekeepers_hut",
		"barbarians_hunters_hut",
		"barbarians_inn",
		"barbarians_micro_brewery",
		"barbarians_tavern",
		"barbarians_well",

		-- Mining
		"barbarians_charcoal_kiln",
		"barbarians_coalmine",
		"barbarians_coalmine_deep",
		"barbarians_coalmine_deeper",
		"barbarians_goldmine",
		"barbarians_goldmine_deep",
		"barbarians_goldmine_deeper",
		"barbarians_granitemine",
		"barbarians_ironmine",
		"barbarians_ironmine_deep",
		"barbarians_ironmine_deeper",
		"barbarians_smelting_works",

		-- Tools, Military & Ships
		"barbarians_axfactory",
		"barbarians_helmsmithy",
		"barbarians_metal_workshop",
		"barbarians_scouts_hut",
		"barbarians_shipyard",
		"barbarians_warmill",
		"barbarians_weaving_mill",
	},
   warehouses = {
		"barbarians_headquarters",
		"barbarians_headquarters_interim",
		"barbarians_port",
		"barbarians_warehouse",
	},
   constructionsites = {
		"constructionsite"
	},
   dismantlesites = {
		"dismantlesite"
	},
}
