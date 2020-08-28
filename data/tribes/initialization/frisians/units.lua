tribes = wl.Tribes()

image_dirname = path.dirname(__file__) .. "images/"

push_textdomain("tribes_encyclopedia")

tribes:new_tribe {
   name = "frisians",
   animation_directory = image_dirname,
   animations = {
      frontier = { hotspot = {8, 26} },
      bridge_normal_e = { hotspot = {-2, 12} },
      bridge_busy_e = { hotspot = {-2, 12} },
      bridge_normal_se = { hotspot = {5, 2} },
      bridge_busy_se = { hotspot = {5, 2} },
      bridge_normal_sw = { hotspot = {36, 3} },
      bridge_busy_sw = { hotspot = {36, 3} }
   },
   spritesheets = {
      flag = {
         hotspot = {11, 41},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
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
         image_dirname .. "waterway_0.png",
      },
   },

   resource_indicators = {
      [""] = {
         [0] = "frisians_resi_none",
      },
      resource_coal = {
         [10] = "frisians_resi_coal_1",
         [20] = "frisians_resi_coal_2",
      },
      resource_iron = {
         [10] = "frisians_resi_iron_1",
         [20] = "frisians_resi_iron_2",
      },
      resource_gold = {
         [10] = "frisians_resi_gold_1",
         [20] = "frisians_resi_gold_2",
      },
      resource_stones = {
         [10] = "frisians_resi_stones_1",
         [20] = "frisians_resi_stones_2",
      },
      resource_water = {
         [100] = "frisians_resi_water",
      },
   },

   -- Wares positions in wares windows.
   -- This also gives us the information which wares the tribe uses.
   -- Each subtable is a column in the wares windows.
   wares_order = {
      {
         -- Building Materials
         {
            name = "log",
            preciousness = 4,
            helptexts = {

            }
         },
         {
            name = "granite",
            default_target_quantity = 30,
            preciousness = 3,
            helptexts = {

            }
         },
         {
            name = "clay",
            default_target_quantity = 30,
            preciousness = 9,
            helptexts = {

            }
         },
         {
            name = "brick",
            default_target_quantity = 40,
            preciousness = 3,
            helptexts = {

            }
         },
         {
            name = "reed",
            preciousness = 8,
            helptexts = {

            }
         },
         {
            name = "fur",
            default_target_quantity = 10,
            preciousness = 1,
            helptexts = {

            }
         },
         {
            name = "cloth",
            default_target_quantity = 10,
            preciousness = 0,
            helptexts = {

            }
         },
      },
      {
         -- Food
         {
            name = "fruit",
            preciousness = 1,
            helptexts = {

            }
         },
         {
            name = "water",
            preciousness = 2,
            helptexts = {

            }
         },
         {
            name = "barley",
            preciousness = 25,
            helptexts = {

            }
         },
         {
            name = "honey",
            preciousness = 1,
            helptexts = {

            }
         },
         {
            name = "bread_frisians",
            default_target_quantity = 20,
            preciousness = 3,
            helptexts = {

            }
         },
         {
            name = "honey_bread",
            default_target_quantity = 20,
            preciousness = 5,
            helptexts = {

            }
         },
         {
            name = "beer",
            default_target_quantity = 15,
            preciousness = 3,
            helptexts = {

            }
         },
         {
            name = "mead",
            default_target_quantity = 15,
            preciousness = 5,
            helptexts = {

            }
         },
         {
            name = "fish",
            default_target_quantity = 20,
            preciousness = 1,
            helptexts = {

            }
         },
         {
            name = "meat",
            preciousness = 2,
            helptexts = {

            }
         },
         {
            name = "smoked_fish",
            default_target_quantity = 20,
            preciousness = 5,
            helptexts = {

            }
         },
         {
            name = "smoked_meat",
            default_target_quantity = 10,
            preciousness = 7,
            helptexts = {

            }
         },
         {
            name = "ration",
            default_target_quantity = 20,
            preciousness = 3,
            helptexts = {

            }
         },
         {
            name = "meal",
            default_target_quantity = 5,
            preciousness = 6,
            helptexts = {

            }
         }
      },
      {
         -- Mining
         {
            name = "coal",
            default_target_quantity = 20,
            preciousness = 40,
            helptexts = {

            }
         },
         {
            name = "iron_ore",
            default_target_quantity = 15,
            preciousness = 2,
            helptexts = {

            }
         },
         {
            name = "iron",
            default_target_quantity = 20,
            preciousness = 4,
            helptexts = {

            }
         },
         {
            name = "gold_ore",
            default_target_quantity = 15,
            preciousness = 3,
            helptexts = {

            }
         },
         {
            name = "gold",
            default_target_quantity = 20,
            preciousness = 6,
            helptexts = {

            }
         },
         {
            name = "scrap_iron",
            preciousness = 0,
            helptexts = {

            }
         },
         {
            name = "scrap_metal_mixed",
            preciousness = 1,
            helptexts = {

            }
         },
         {
            name = "fur_garment_old",
            preciousness = 0,
            helptexts = {

            }
         }
      },
      {
         -- Tools
         {
            name = "pick",
            default_target_quantity = 3,
            preciousness = 0,
            helptexts = {

            }
         },
         {
            name = "felling_ax",
            default_target_quantity = 3,
            preciousness = 0,
            helptexts = {

            }
         },
         {
            name = "shovel",
            default_target_quantity = 4,
            preciousness = 0,
            helptexts = {

            }
         },
         {
            name = "hammer",
            default_target_quantity = 2,
            preciousness = 0,
            helptexts = {

            }
         },
         {
            name = "fishing_net",
            default_target_quantity = 2,
            preciousness = 0,
            helptexts = {

            }
         },
         {
            name = "hunting_spear",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {

            }
         },
         {
            name = "scythe",
            default_target_quantity = 2,
            preciousness = 0,
            helptexts = {

            }
         },
         {
            name = "bread_paddle",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {

            }
         },
         {
            name = "kitchen_tools",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {

            }
         },
         {
            name = "fire_tongs",
            default_target_quantity = 2,
            preciousness = 0,
            helptexts = {

            }
         },
         {
            name = "basket",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {

            }
         },
         {
            name = "needles",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {

            }
         }
      },
      {
         -- Weapons & Armor
         {
            name = "sword_short",
            default_target_quantity = 30,
            preciousness = 4,
            helptexts = {

            }
         },
         {
            name = "sword_long",
            default_target_quantity = 2,
            preciousness = 3,
            helptexts = {

            }
         },
         {
            name = "sword_broad",
            default_target_quantity = 2,
            preciousness = 3,
            helptexts = {

            }
         },
         {
            name = "sword_double",
            default_target_quantity = 2,
            preciousness = 3,
            helptexts = {

            }
         },
         {
            name = "fur_garment",
            default_target_quantity = 30,
            preciousness = 3,
            helptexts = {

            }
         },
         {
            name = "fur_garment_studded",
            default_target_quantity = 2,
            preciousness = 3,
            helptexts = {

            }
         },
         {
            name = "fur_garment_golden",
            default_target_quantity = 2,
            preciousness = 3,
            helptexts = {

            }
         },
         {
            name = "helmet",
            default_target_quantity = 2,
            preciousness = 3,
            helptexts = {

            }
         },
         {
            name = "helmet_golden",
            default_target_quantity = 2,
            preciousness = 3,
            helptexts = {

            }
         }
      }
   },

   -- Workers positions in workers windows.
   -- This also gives us the information which workers the tribe uses.
   -- Each subtable is a column in the workers windows.
   workers_order = {
      {
         -- Carriers
         {
            name = "frisians_carrier",
            helptexts = {

            }
         },
         {
            name = "frisians_ferry",
            helptexts = {

            }
         },
         {
            name = "frisians_reindeer",
            default_target_quantity = 10,
            preciousness = 2,
            helptexts = {

            }
         },
         {
            name = "frisians_reindeer_breeder",
            helptexts = {

            }
         }
      },
      {
         -- Building Materials
         {
            name = "frisians_stonemason",
            helptexts = {

            }
         },
         {
            name = "frisians_woodcutter",
            helptexts = {

            }
         },
         {
            name = "frisians_forester",
            helptexts = {

            }
         },
         {
            name = "frisians_claydigger",
            helptexts = {

            }
         },
         {
            name = "frisians_brickmaker",
            helptexts = {

            }
         },
         {
            name = "frisians_builder",
            helptexts = {

            }
         },
         {
            name = "frisians_reed_farmer",
            helptexts = {

            }
         },
         {
            name = "frisians_seamstress",
            helptexts = {

            }
         },
         {
            name = "frisians_seamstress_master",
            helptexts = {

            }
         },
         {
            name = "frisians_shipwright",
            helptexts = {

            }
         }
      },
      {
         -- Food
         {
            name = "frisians_fisher",
            helptexts = {

            }
         },
         {
            name = "frisians_hunter",
            helptexts = {

            }
         },
         {
            name = "frisians_farmer",
            helptexts = {

            }
         },
         {
            name = "frisians_berry_farmer",
            helptexts = {

            }
         },
         {
            name = "frisians_fruit_collector",
            helptexts = {

            }
         },
         {
            name = "frisians_smoker",
            helptexts = {

            }
         },
         {
            name = "frisians_beekeeper",
            helptexts = {

            }
         },
         {
            name = "frisians_baker",
            helptexts = {

            }
         },
         {
            name = "frisians_baker_master",
            helptexts = {

            }
         },
         {
            name = "frisians_brewer",
            helptexts = {

            }
         },
         {
            name = "frisians_brewer_master",
            helptexts = {

            }
         },
         {
            name = "frisians_landlady",
            helptexts = {

            }
         }
      },
      {
         -- Mining
         { name = "frisians_geologist" },
         { name = "frisians_miner" },
         { name = "frisians_miner_master" },
         { name = "frisians_charcoal_burner" },
         { name = "frisians_smelter" },
         { name = "frisians_blacksmith" },
         { name = "frisians_blacksmith_master" }
      },
      {
         -- Military
         {
            name = "frisians_soldier",
            default_target_quantity = 10,
            preciousness = 5
         },
         { name = "frisians_trainer" },
         { name = "frisians_scout" }
      }
   },

   immovables = {
      {
         name = "ashes",
         helptexts = {

         }
      },
      {
         name = "destroyed_building",
         helptexts = {

         }
      },
      {
         name = "berry_bush_blueberry_tiny",
         helptexts = {

         }
      },
      {
         name = "berry_bush_blueberry_small",
         helptexts = {

         }
      },
      {
         name = "berry_bush_blueberry_medium",
         helptexts = {

         }
      },
      {
         name = "berry_bush_blueberry_ripe",
         helptexts = {

         }
      },
      {
         name = "berry_bush_currant_red_tiny",
         helptexts = {

         }
      },
      {
         name = "berry_bush_currant_red_small",
         helptexts = {

         }
      },
      {
         name = "berry_bush_currant_red_medium",
         helptexts = {

         }
      },
      {
         name = "berry_bush_currant_red_ripe",
         helptexts = {

         }
      },
      {
         name = "berry_bush_juniper_tiny",
         helptexts = {

         }
      },
      {
         name = "berry_bush_juniper_small",
         helptexts = {

         }
      },
      {
         name = "berry_bush_juniper_medium",
         helptexts = {

         }
      },
      {
         name = "berry_bush_juniper_ripe",
         helptexts = {

         }
      },
      {
         name = "berry_bush_raspberry_tiny",
         helptexts = {

         }
      },
      {
         name = "berry_bush_raspberry_small",
         helptexts = {

         }
      },
      {
         name = "berry_bush_raspberry_medium",
         helptexts = {

         }
      },
      {
         name = "berry_bush_raspberry_ripe",
         helptexts = {

         }
      },
      {
         name = "berry_bush_currant_black_tiny",
         helptexts = {

         }
      },
      {
         name = "berry_bush_currant_black_small",
         helptexts = {

         }
      },
      {
         name = "berry_bush_currant_black_medium",
         helptexts = {

         }
      },
      {
         name = "berry_bush_currant_black_ripe",
         helptexts = {

         }
      },
      {
         name = "berry_bush_strawberry_tiny",
         helptexts = {

         }
      },
      {
         name = "berry_bush_strawberry_small",
         helptexts = {

         }
      },
      {
         name = "berry_bush_strawberry_medium",
         helptexts = {

         }
      },
      {
         name = "berry_bush_strawberry_ripe",
         helptexts = {

         }
      },
      {
         name = "berry_bush_stink_tree_tiny",
         helptexts = {

         }
      },
      {
         name = "berry_bush_stink_tree_small",
         helptexts = {

         }
      },
      {
         name = "berry_bush_stink_tree_medium",
         helptexts = {

         }
      },
      {
         name = "berry_bush_stink_tree_ripe",
         helptexts = {

         }
      },
      {
         name = "berry_bush_desert_hackberry_tiny",
         helptexts = {

         }
      },
      {
         name = "berry_bush_desert_hackberry_small",
         helptexts = {

         }
      },
      {
         name = "berry_bush_desert_hackberry_medium",
         helptexts = {

         }
      },
      {
         name = "berry_bush_desert_hackberry_ripe",
         helptexts = {

         }
      },
      {
         name = "berry_bush_sea_buckthorn_tiny",
         helptexts = {

         }
      },
      {
         name = "berry_bush_sea_buckthorn_small",
         helptexts = {

         }
      },
      {
         name = "berry_bush_sea_buckthorn_medium",
         helptexts = {

         }
      },
      {
         name = "berry_bush_sea_buckthorn_ripe",
         helptexts = {

         }
      },
      {
         name = "barleyfield_tiny",
         helptexts = {

         }
      },
      {
         name = "barleyfield_small",
         helptexts = {

         }
      },
      {
         name = "barleyfield_medium",
         helptexts = {

         }
      },
      {
         name = "barleyfield_ripe",
         helptexts = {

         }
      },
      {
         name = "barleyfield_harvested",
         helptexts = {

         }
      },
      {
         name = "pond_dry",
         helptexts = {

         }
      },
      {
         name = "pond_growing",
         helptexts = {

         }
      },
      {
         name = "pond_mature",
         helptexts = {

         }
      },
      {
         name = "pond_burning",
         helptexts = {

         }
      },
      {
         name = "pond_coal",
         helptexts = {

         }
      },
      {
         name = "reedfield_tiny",
         helptexts = {

         }
      },
      {
         name = "reedfield_small",
         helptexts = {

         }
      },
      {
         name = "reedfield_medium",
         helptexts = {

         }
      },
      {
         name = "reedfield_ripe",
         helptexts = {

         }
      },
      {
         name = "frisians_resi_none",
         helptexts = {

         }
      },
      {
         name = "frisians_resi_water",
         helptexts = {

         }
      },
      {
         name = "frisians_resi_coal_1",
         helptexts = {

         }
      },
      {
         name = "frisians_resi_iron_1",
         helptexts = {

         }
      },
      {
         name = "frisians_resi_gold_1",
         helptexts = {

         }
      },
      {
         name = "frisians_resi_stones_1",
         helptexts = {

         }
      },
      {
         name = "frisians_resi_coal_2",
         helptexts = {

         }
      },
      {
         name = "frisians_resi_iron_2",
         helptexts = {

         }
      },
      {
         name = "frisians_resi_gold_2",
         helptexts = {

         }
      },
      {
         name = "frisians_resi_stones_2",
         helptexts = {

         }
      },
      {
         name = "frisians_shipconstruction",
         helptexts = {

         }
      },
      -- These non-frisian immovables can be used by bee-keepers
      -- NOCOM check whether we still need to define these here
      {
         name = "wheatfield_medium",
         helptexts = {

         }
      },
      {
         name = "cornfield_medium",
         helptexts = {

         }
      },
      {
         name = "blackrootfield_medium",
         helptexts = {

         }
      },
      {
         name = "grapevine_medium",
         helptexts = {

         }
      },
      -- Used by the fruit collector
      {
         name = "grapevine_ripe",
         helptexts = {

         }
      }
   },

   -- The order here also determines the order in lists on screen.
   buildings = {
      -- Warehouses
      {
         name = "frisians_headquarters",
         helptexts = {

         }
      },
      {
         name = "frisians_warehouse",
         helptexts = {

         }
      },
      {
         name = "frisians_port",
         helptexts = {

         }
      },

      -- Small
      {
         name = "frisians_quarry",
         helptexts = {

         }
      },
      {
         name = "frisians_woodcutters_house",
         helptexts = {

         }
      },
      {
         name = "frisians_foresters_house",
         helptexts = {

         }
      },
      {
         name = "frisians_hunters_house",
         helptexts = {

         }
      },
      {
         name = "frisians_fishers_house",
         helptexts = {

         }
      },
      {
         name = "frisians_reed_farm",
         helptexts = {

         }
      },
      {
         name = "frisians_well",
         helptexts = {

         }
      },
      {
         name = "frisians_clay_pit",
         helptexts = {

         }
      },
      {
         name = "frisians_charcoal_burners_house",
         helptexts = {

         }
      },
      {
         name = "frisians_berry_farm",
         helptexts = {

         }
      },
      {
         name = "frisians_collectors_house",
         helptexts = {

         }
      },
      {
         name = "frisians_beekeepers_house",
         helptexts = {

         }
      },
      {
         name = "frisians_aqua_farm",
         helptexts = {

         }
      },
      {
         name = "frisians_scouts_house",
         helptexts = {

         }
      },

      -- Medium
      {
         name = "frisians_brick_kiln",
         helptexts = {

         }
      },
      {
         name = "frisians_furnace",
         helptexts = {

         }
      },
      {
         name = "frisians_recycling_center",
         helptexts = {

         }
      },
      {
         name = "frisians_blacksmithy",
         helptexts = {

         }
      },
      {
         name = "frisians_armor_smithy_small",
         helptexts = {

         }
      },
      {
         name = "frisians_armor_smithy_large",
         helptexts = {

         }
      },
      {
         name = "frisians_sewing_room",
         helptexts = {

         }
      },
      {
         name = "frisians_tailors_shop",
         helptexts = {

         }
      },
      {
         name = "frisians_charcoal_kiln",
         helptexts = {

         }
      },
      {
         name = "frisians_smokery",
         helptexts = {

         }
      },
      {
         name = "frisians_bakery",
         helptexts = {

         }
      },
      {
         name = "frisians_honey_bread_bakery",
         helptexts = {

         }
      },
      {
         name = "frisians_brewery",
         helptexts = {

         }
      },
      {
         name = "frisians_mead_brewery",
         helptexts = {

         }
      },
      {
         name = "frisians_tavern",
         helptexts = {

         }
      },
      {
         name = "frisians_drinking_hall",
         helptexts = {

         }
      },
      {
         name = "frisians_barracks",
         helptexts = {

         }
      },

      -- Big
      {
         name = "frisians_reindeer_farm",
         helptexts = {

         }
      },
      {
         name = "frisians_farm",
         helptexts = {

         }
      },

      -- Mines
      {
         name = "frisians_rockmine",
         helptexts = {

         }
      },
      {
         name = "frisians_rockmine_deep",
         helptexts = {

         }
      },
      {
         name = "frisians_coalmine",
         helptexts = {

         }
      },
      {
         name = "frisians_coalmine_deep",
         helptexts = {

         }
      },
      {
         name = "frisians_ironmine",
         helptexts = {

         }
      },
      {
         name = "frisians_ironmine_deep",
         helptexts = {

         }
      },
      {
         name = "frisians_goldmine",
         helptexts = {

         }
      },
      {
         name = "frisians_goldmine_deep",
         helptexts = {

         }
      },

      -- Training Sites
      {
         name = "frisians_training_camp",
         helptexts = {

         }
      },
      {
         name = "frisians_training_arena",
         helptexts = {

         }
      },

      -- Military Sites
      {
         name = "frisians_wooden_tower",
         helptexts = {

         }
      },
      {
         name = "frisians_wooden_tower_high",
         helptexts = {

         }
      },
      {
         name = "frisians_sentinel",
         helptexts = {

         }
      },
      {
         name = "frisians_outpost",
         helptexts = {

         }
      },
      {
         name = "frisians_tower",
         helptexts = {

         }
      },
      {
         name = "frisians_fortress",
         helptexts = {

         }
      },

      -- Seafaring/Ferry Sites - these are only displayed on seafaring/ferry maps
      {
         name = "frisians_ferry_yard",
         helptexts = {

         }
      },
      {
         name = "frisians_shipyard",
         helptexts = {

         }
      },
      {
         name = "frisians_weaving_mill",
         helptexts = {

         }
      },

      -- Partially Finished Buildings - these are the same 2 buildings for all tribes
      {
         name = "constructionsite",
         helptexts = {

         }
      },
      {
         name = "dismantlesite",
         helptexts = {

         }
      }
   },

   -- Special types
   builder = "frisians_builder",
   carrier = "frisians_carrier",
   carrier2 = "frisians_reindeer",
   geologist = "frisians_geologist",
   soldier = "frisians_soldier",
   ship = "frisians_ship",
   ferry = "frisians_ferry",
   port = "frisians_port",
}

pop_textdomain()
