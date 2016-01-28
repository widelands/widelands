dirname = path.dirname(__file__)

tribes:new_tribe {
   name = "atlanteans",

   animations = {
      -- Some blue fires would be fine, but just an idea
      frontier = {
         pictures = path.list_files(dirname .. "images/atlanteans/frontier_??.png"),
         hotspot = { 3, 12 },
      },
      flag = {
         -- Not just a plain color, maybe a cross or some stripes
         pictures = path.list_files(dirname .. "images/atlanteans/flag_??.png"),
         hotspot = { 15, 35 },
         fps = 10
      }
   },

   -- Image file paths for this tribe's road textures
   roads = {
      busy = {
         "tribes/images/atlanteans/roadt_busy.png",
      },
      normal = {
         "tribes/images/atlanteans/roadt_normal_00.png",
         "tribes/images/atlanteans/roadt_normal_01.png",
      },
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
         "atlanteans_bread"
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
         "buckets",
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
      "resi_water1",
      "resi_stones1",
      "resi_stones2",
      "atlanteans_shipconstruction",
   },

   -- The order here also determines the order in lists on screen.
   buildings = {
      -- Warehouses
      "atlanteans_headquarters",
      "atlanteans_warehouse",
      "atlanteans_port",

      -- Small
      "atlanteans_quarry",
      "atlanteans_woodcutters_house",
      "atlanteans_foresters_house",
      "atlanteans_fishers_house",
      "atlanteans_fishbreeders_house",
      "atlanteans_hunters_house",
      "atlanteans_well",
      "atlanteans_gold_spinning_mill",
      "atlanteans_scouts_house",

      -- Medium
      "atlanteans_sawmill",
      "atlanteans_smokery",
      "atlanteans_mill",
      "atlanteans_bakery",
      "atlanteans_charcoal_kiln",
      "atlanteans_smelting_works",
      "atlanteans_toolsmithy",
      "atlanteans_weaponsmithy",
      "atlanteans_armorsmithy",
      "atlanteans_shipyard",

      -- Big
      "atlanteans_horsefarm",
      "atlanteans_farm",
      "atlanteans_blackroot_farm",
      "atlanteans_spiderfarm",
      "atlanteans_weaving_mill",

      -- Mines
      "atlanteans_crystalmine",
      "atlanteans_coalmine",
      "atlanteans_ironmine",
      "atlanteans_goldmine",

      -- Training Sites
      "atlanteans_dungeon",
      "atlanteans_labyrinth",

      -- Military Sites
      "atlanteans_guardhouse",
      "atlanteans_guardhall",
      "atlanteans_tower_small",
      "atlanteans_tower",
      "atlanteans_tower_high",
      "atlanteans_castle",

      -- Partially Finished Buildings - these are the same 2 buildings for all tribes
      "constructionsite",
      "dismantlesite",
   },

   ship_names = {
      "Abaco",
      "Agate",
      "Alexandrite",
      "Amber",
      "Amethyst",
      "Anguilla",
      "Antigua",
      "Aquamarine",
      "Atlantean's Stronghold",
      "Atlantis",
      "Bahama",
      "Barbados",
      "Barbuda",
      "Beryl",
      "Blanquilla",
      "Caicos",
      "Cassiterite",
      "Cat's Eye",
      "Citrine",
      "Coliondor",
      "Dominica",
      "Eleuthera",
      "Emerald",
      "Grenada",
      "Guadelope",
      "Inagua",
      "Jundlina",
      "Juventud",
      "King Ajanthul",
      "King Askandor",
      "Kitts",
      "Loftomor",
      "Malachite",
      "Martinique", 
      "Montserrat",
      "Moonstone",
      "Mystic Quartz",
      "Nassau",
      "Nevis",
      "Obsidian",
      "Onyx",
      "Opol",
      "Orchila",
      "Ostur",
      "Pearl",
      "Sapphire",
      "Satul",
      "Sidolus",
      "Sphalerite",
      "Spider",
      "Spinel",
      "Sunstone",
      "Tiger Eye",
      "Tobago",
      "Topaz",
      "Tortuga",
      "Tourmaline",
      "Trinidad",
   },

   -- Special types
   builder = "atlanteans_builder",
   carrier = "atlanteans_carrier",
   carrier2 = "atlanteans_horse",
   geologist = "atlanteans_geologist",
   soldier = "atlanteans_soldier",
   ship = "atlanteans_ship",
   headquarters = "atlanteans_headquarters",
   port = "atlanteans_port",
}
