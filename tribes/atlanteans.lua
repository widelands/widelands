dirname = path.dirname(__file__)

tribes:new_tribe {
   name = "atlanteans",

   animations = {
		-- Some blue fires would be fine, but just an idea
		frontier = {
			pictures = path.list_directory(dirname .. "images/atlanteans/", "frontier_\\d+.png"),
			hotspot = { 3, 12 },
		},
		flag = {
			-- Not just a plain color, maybe a cross or some stripes
			pictures = path.list_directory(dirname .. "images/atlanteans/", "flag_\\d+.png"),
			hotspot = { 15, 35 },
			fps = 10
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
		},
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
		},
		{
			-- Military
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

   -- Workers positions in workers windows.
   -- This also gives us the information which workers the tribe uses.
   -- Each subtable is a column in the workers windows.
   workers_order = {
		{
			-- Carriers
			"atlanteans_builder",
			"atlanteans_carrier",
			"atlanteans_horse",
			"atlanteans_horsebreeder"
		},
		{
			-- Building Materials
			"atlanteans_stonecutter",
			"atlanteans_woodcutter",
			"atlanteans_sawyer",
			"atlanteans_forester",
			"atlanteans_builder",
			"atlanteans_spiderbreeder",
			"atlanteans_weaver",
			"atlanteans_shipwright"
		},
		{
			-- Food
			"atlanteans_fisher",
			"atlanteans_fishbreeder",
			"atlanteans_hunter",
			"atlanteans_smoker",
			"atlanteans_farmer",
			"atlanteans_blackroot_farmer",
			"atlanteans_miller",
			"atlanteans_baker"
		},
		{
			-- Mining
			"atlanteans_geologist",
			"atlanteans_miner",
			"atlanteans_charcoal_burner",
			"atlanteans_smelter"
		},
		{
			-- Tools
			"atlanteans_toolsmith"
		},
		{
			-- Military
			"atlanteans_soldier",
			"atlanteans_trainer",
			"atlanteans_weaponsmith",
			"atlanteans_armorsmith",
			"atlanteans_scout"
		}
	},

	builder = "atlanteans_builder",
	carrier = "atlanteans_carrier",
	carrier2 = "atlanteans_horse",
	geologist = "atlanteans_geologist",
   soldier = "atlanteans_soldier",
   ship = "ship_atlanteans",

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
	},
	militarysites = {
		"atlanteans_castle",
		"atlanteans_guardhall",
		"atlanteans_tower",
		"atlanteans_tower_high",
		"atlanteans_tower_small"
	},
	trainingsites = {
		"atlanteans_dungeon",
		"atlanteans_labyrinth",
	},
   productionsites = {
		-- Carriers
		"atlanteans_horsefarm",

		-- Building Materials
		"atlanteans_foresters_house",
		"atlanteans_quarry",
		"atlanteans_sawmill",
		"atlanteans_spiderfarm",
		"atlanteans_weaving_mill",
		"atlanteans_woodcutters_house",

		-- Food
		"atlanteans_bakery",
		"atlanteans_blackroot_farm",
		"atlanteans_farm",
		"atlanteans_fishbreeders_house",
		"atlanteans_fishers_house",
		"atlanteans_hunters_house",
		"atlanteans_mill",
		"atlanteans_smokery",
		"atlanteans_well",

		-- Mining
		"atlanteans_charcoal_kiln",
		"atlanteans_coalmine",
		"atlanteans_crystalmine",
		"atlanteans_goldmine",
		"atlanteans_gold_spinning_mill",
		"atlanteans_ironmine",
		"atlanteans_smelting_works",

		-- Tools, Military & Ships
		"atlanteans_armorsmithy",
		"atlanteans_scouts_house",
		"atlanteans_shipyard",
		"atlanteans_toolsmithy",
		"atlanteans_weaponsmithy",

	},
   warehouses = {
		"atlanteans_headquarters",
		"atlanteans_port",
		"atlanteans_warehouse",
	},
   constructionsites = {
		"constructionsite"
	},
   dismantlesites = {
		"dismantlesite"
	},
}
