-- RST
-- .. _lua_tribes_<tribename>.lua:
--
-- <tribename>.lua
-- ---------------------
--
-- This file contains all the units for a tribe.
--
-- .. function:: new_tribe{table}
--
--    This function adds all units to a tribe.
--
--    :arg table: This table contains all the data that the game engine will add to the tribe.
--                It contains the following entries:
--
--    **name**: A string containing the internal name of the tribe.
--
--    **animations**: Global animations. Contains subtables for ``frontier`` and ``flag``. Each animation needs the parameters ``pictures`` (table of filenames) and ``hotspot`` (2 integer coordinates), and may also define ``fps`` (integer frames per second).
--
--    **animation_directory**: The location of the animation png files.
--
--    **bridges**: Contains animations for ``normal_e``, ``normal_se``, ``normal_sw``, ``busy_e``, ``busy_se`` and ``busy_sw``.
--
--    **bridge_height**: The height in pixels of each bridge at it's summit at 1x scale.
--
--    **roads**: The file paths for the tribe's road textures in 3 subtables ``busy``, ``normal`` and ``waterway``.
--
--    **resource_indicators**: The names for the resource indicators. This table contains a subtable for each resource name plus a subtable named "" for no resources. Each subtable is an array, in which the index of each entry is the highest amount of resources the indicator may indicate.
--
--    **wares_order**: This defines all the wares that this tribe uses and their display order in the user interface. Each subtable defines a column in the user interface.
--
--    **workers_order**:  This defines all the workers that this tribe uses and their display order in the user interface. Each subtable defines a column in the user interface.
--
--    **immovables**: This defines all the immovables that this tribe uses.
--
--    **buildings**: This defines all the buildings that this tribe uses and their display order in the user interface.
--
--    **ship_names**: A list of strings with ship names presented to the user - be creative :)
--
--    **builder**:  The internal name of the tribe's builder. This unit needs to be defined in the ``workers_order`` table too.
--
--    **carrier**:  The internal name of the tribe's carrier. This unit needs to be defined in the ``workers_order`` table too.
--
--    **carrier2**:  The internal name of the tribe's beast of burden. This unit needs to be defined in the ``workers_order`` table too.
--
--    **geologist**:  The internal name of the tribe's geologist. This unit needs to be defined in the ``workers_order`` table too.
--
--    **soldier**: The internal name of the tribe's soldier. This unit needs to be defined in the ``workers_order`` table too.
--
--    **ship**: The internal name of the tribe's ship.
--
--    **ferry**: The internal name of the tribe's ferry.
--
--    **port**: The internal name of the tribe's port building. This unit needs to be defined in the ``buildings`` table too.
--
--    **toolbar**: *Optional*. Replace the default toolbar images with these custom images. Example:
--
--    .. code-block:: lua
--
--       toolbar = {
--          left_corner = dirname .. "images/atlanteans/toolbar_left_corner.png",
--          left = dirname .. "images/atlanteans/toolbar_left.png", -- Will be tiled
--          center = dirname .. "images/atlanteans/toolbar_center.png",
--          right = dirname .. "images/atlanteans/toolbar_right.png", -- Will be tiled
--          right_corner = dirname .. "images/atlanteans/toolbar_right_corner.png"
--       }
--

tribes = wl.Tribes()

image_dirname = path.dirname(__file__) .. "images/atlanteans/"

tribes:new_tribe {
   name = "atlanteans",
   animation_directory = image_dirname,
   animations = {
      frontier = { hotspot = {3, 12} },
      flag = { hotspot = {15, 35}, fps = 10 },
      bridge_normal_e = { hotspot = {-2, 11} },
      bridge_busy_e = { hotspot = {-2, 11} },
      bridge_normal_se = { hotspot = {5, 2} },
      bridge_busy_se = { hotspot = {5, 2} },
      bridge_normal_sw = { hotspot = {36, 6} },
      bridge_busy_sw = { hotspot = {36, 6} }
   },

   bridge_height = 8,

   -- Image file paths for this tribe's road and waterway textures
   roads = {
      busy = {
         image_dirname .. "roadt_busy.png",
      },
      normal = {
         image_dirname .. "roadt_normal_00.png",
         image_dirname .. "roadt_normal_01.png",
      },
      waterway = {
         "tribes/images/atlanteans/waterway_0.png",
      },
   },

   resource_indicators = {
      [""] = {
         [0] = "atlanteans_resi_none",
      },
      coal = {
         [10] = "atlanteans_resi_coal_1",
         [20] = "atlanteans_resi_coal_2",
      },
      iron = {
         [10] = "atlanteans_resi_iron_1",
         [20] = "atlanteans_resi_iron_2",
      },
      gold = {
         [10] = "atlanteans_resi_gold_1",
         [20] = "atlanteans_resi_gold_2",
      },
      stones = {
         [10] = "atlanteans_resi_stones_1",
         [20] = "atlanteans_resi_stones_2",
      },
      water = {
         [100] = "atlanteans_resi_water",
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
         "atlanteans_ferry",
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
         "atlanteans_recruit",
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
      "atlanteans_resi_none",
      "atlanteans_resi_water",
      "atlanteans_resi_coal_1",
      "atlanteans_resi_iron_1",
      "atlanteans_resi_gold_1",
      "atlanteans_resi_stones_1",
      "atlanteans_resi_coal_2",
      "atlanteans_resi_iron_2",
      "atlanteans_resi_gold_2",
      "atlanteans_resi_stones_2",
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
      "atlanteans_barracks",

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

      -- Seafaring/Ferry Sites - these are only displayed on seafaring/ferry maps
      "atlanteans_ferry_yard",
      "atlanteans_shipyard",

      -- Partially Finished Buildings - these are the same 2 buildings for all tribes
      "constructionsite",
      "dismantlesite",
   },

   ship_names = {
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Abaco"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Agate"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Alexandrite"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Amber"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Amethyst"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Anguilla"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Antigua"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Aquamarine"),
      pgettext("shipname", "Atlantean’s Stronghold"),
      pgettext("shipname", "Atlantis"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Bahama"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Barbados"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Barbuda"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Beryl"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Blanquilla"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Caicos"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Cassiterite"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Cat’s Eye"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Citrine"),
      -- TRANSLATORS: This Atlantean ship is named after an in-game character
      pgettext("shipname", "Colionder"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Dominica"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Eleuthera"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Emerald"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Grenada"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Guadeloupe"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Inagua"),
      -- TRANSLATORS: This Atlantean ship is named after an in-game character
      pgettext("shipname", "Jundlina"),
      pgettext("shipname", "Juventud"),
      -- TRANSLATORS: This Atlantean ship is named after an in-game character
      pgettext("shipname", "King Ajanthul"),
      -- TRANSLATORS: This Atlantean ship is named after an in-game character
      pgettext("shipname", "King Askandor"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Kitts"),
      -- TRANSLATORS: This Atlantean ship is named after an in-game character
      pgettext("shipname", "Loftomor"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Malachite"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Martinique"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Montserrat"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Moonstone"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Mystic Quartz"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Nassau"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Nevis"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Obsidian"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Onyx"),
      -- TRANSLATORS: This Atlantean ship is named after an in-game character
      pgettext("shipname", "Opol"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Orchila"),
      -- TRANSLATORS: This Atlantean ship is named after an in-game character
      pgettext("shipname", "Ostur"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Pearl"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Sapphire"),
      -- TRANSLATORS: This Atlantean ship is named after an in-game character
      pgettext("shipname", "Satul"),
      -- TRANSLATORS: This Atlantean ship is named after an in-game character
      pgettext("shipname", "Sidolus"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Sphalerite"),
      pgettext("shipname", "Spider"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Spinel"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Sunstone"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Tiger Eye"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Tobago"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Topaz"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Tortuga"),
      -- TRANSLATORS: This Atlantean ship is named after a mineral
      pgettext("shipname", "Tourmaline"),
      -- TRANSLATORS: This Atlantean ship is named after an island
      pgettext("shipname", "Trinidad"),
   },

   -- Special types
   builder = "atlanteans_builder",
   carrier = "atlanteans_carrier",
   carrier2 = "atlanteans_horse",
   geologist = "atlanteans_geologist",
   soldier = "atlanteans_soldier",
   ship = "atlanteans_ship",
   ferry = "atlanteans_ferry",
   port = "atlanteans_port",

   toolbar = {
      left_corner = image_dirname .. "toolbar_left_corner.png",
      left = image_dirname .. "toolbar_left.png",
      center = image_dirname .. "toolbar_center.png",
      right = image_dirname .. "toolbar_right.png",
      right_corner = image_dirname .. "toolbar_right_corner.png"
   }
}
