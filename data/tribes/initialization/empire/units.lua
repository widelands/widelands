tribes = wl.Tribes()

image_dirname = path.dirname(__file__) .. "images/"

push_textdomain("tribes_encyclopedia")

tribes:new_tribe {
   name = "empire",
   animation_directory = image_dirname,
   animations = {
      frontier = { hotspot = {1, 19} },
      flag = { hotspot = {14, 38}, fps = 10 },
      bridge_normal_e = { hotspot = {-2, 12} },
      bridge_busy_e = { hotspot = {-2, 12} },
      bridge_normal_se = { hotspot = {5, 2} },
      bridge_busy_se = { hotspot = {5, 2} },
      bridge_normal_sw = { hotspot = {36, 3} },
      bridge_busy_sw = { hotspot = {36, 3} }
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
         image_dirname .. "roadt_normal_02.png",
      },
      waterway = {
         image_dirname .. "waterway_0.png",
      },
   },

   resource_indicators = {
      [""] = {
         [0] = "empire_resi_none",
      },
      resource_coal = {
         [10] = "empire_resi_coal_1",
         [20] = "empire_resi_coal_2",
      },
      resource_iron = {
         [10] = "empire_resi_iron_1",
         [20] = "empire_resi_iron_2",
      },
      resource_gold = {
         [10] = "empire_resi_gold_1",
         [20] = "empire_resi_gold_2",
      },
      resource_stones = {
         [10] = "empire_resi_stones_1",
         [20] = "empire_resi_stones_2",
      },
      resource_water = {
         [100] = "empire_resi_water",
      },
   },

   -- Wares positions in wares windows.
   -- This also gives us the information which wares the tribe uses.
   -- Each subtable is a column in the wares windows.
   wares_order = {
      {
         -- Building Materials
         {
            name = "granite",
            default_target_quantity = 30,
            preciousness = 0,
            helptexts = {

            }
         },
         {
            name = "log",
            preciousness = 14,
            helptexts = {

            }
         },
         {
            name = "planks",
            default_target_quantity = 20,
            preciousness = 10,
            helptexts = {

            }
         },
         {
            name = "wool",
            default_target_quantity = 10,
            preciousness = 2,
            helptexts = {

            }
         },
         {
            name = "cloth",
            default_target_quantity = 15,
            preciousness = 1,
            helptexts = {

            }
         }
      },
      {
         -- Food
         {
            name = "fish",
            preciousness = 3,
            helptexts = {

            }
         },
         {
            name = "meat",
            default_target_quantity = 20,
            preciousness = 2,
            helptexts = {

            }
         },
         {
            name = "water",
            preciousness = 7,
            helptexts = {

            }
         },
         {
            name = "wheat",
            preciousness = 12,
            helptexts = {

            }
         },
         {
            name = "flour",
            default_target_quantity = 20,
            preciousness = 9,
            helptexts = {

            }
         },
         {
            name = "empire_bread",
            default_target_quantity = 20,
            preciousness = 7,
            helptexts = {

            }
         },
         {
            name = "beer",
            default_target_quantity = 15,
            preciousness = 5,
            helptexts = {

            }
         },
         {
            name = "grape",
            preciousness = 10,
            helptexts = {

            }
         },
         {
            name = "wine",
            preciousness = 8,
            helptexts = {

            }
         },
         {
            name = "ration",
            default_target_quantity = 20,
            preciousness = 4,
            helptexts = {

            }
         },
         {
            name = "meal",
            default_target_quantity = 20,
            preciousness = 4,
            helptexts = {

            }
         }
      },
      {
         -- Mining
         {
            name = "marble",
            default_target_quantity = 30,
            preciousness = 3,
            helptexts = {

            }
         },
         {
            name = "marble_column",
            default_target_quantity = 10,
            preciousness = 5,
            helptexts = {

            }
         },
         {
            name = "coal",
            default_target_quantity = 20,
            preciousness = 10,
            helptexts = {

            }
         },
         {
            name = "iron_ore",
            default_target_quantity = 15,
            preciousness = 4,
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
            preciousness = 2,
            helptexts = {

            }
         },
         {
            name = "gold",
            default_target_quantity = 20,
            preciousness = 2,
            helptexts = {

            }
         }
      },
      {
         -- Tools
         {
            name = "pick",
            default_target_quantity = 2,
            preciousness = 1,
            helptexts = {

            }
         },
         {
            name = "felling_ax",
            default_target_quantity = 3,
            preciousness = 1,
            helptexts = {

            }
         },
         {
            name = "saw",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {

            }
         },
         {
            name = "shovel",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {

            }
         },
         {
            name = "hammer",
            default_target_quantity = 2,
            preciousness = 1,
            helptexts = {

            }
         },
         {
            name = "fishing_rod",
            default_target_quantity = 1,
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
            default_target_quantity = 1,
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
            name = "basket",
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
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {

            }
         }
      },
      {
         -- Weapons & Armor
         {
            name = "spear_wooden",
            default_target_quantity = 30,
            preciousness = 1,
            helptexts = {

            }
         },
         {
            name = "spear",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {

            }
         },
         {
            name = "spear_advanced",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {

            }
         },
         {
            name = "spear_heavy",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {

            }
         },
         {
            name = "spear_war",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {

            }
         },
         {
            name = "armor_helmet",
            default_target_quantity = 30,
            preciousness = 2,
            helptexts = {

            }
         },
         {
            name = "armor",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {

            }
         },
         {
            name = "armor_chain",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {

            }
         },
         {
            name = "armor_gilded",
            default_target_quantity = 1,
            preciousness = 1,
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
            name = "empire_carrier",
            helptexts = {

            }
         },
         {
            name = "empire_ferry",
            helptexts = {

            }
         },
         {
            name = "empire_donkey",
            default_target_quantity = 10,
            preciousness = 2,
            helptexts = {

            }
         },
         {
            name = "empire_donkeybreeder",
            helptexts = {

            }
         }
      },
      {
         -- Building Materials
         {
            name = "empire_stonemason",
            helptexts = {

            }
         },
         {
            name = "empire_carpenter",
            helptexts = {

            }
         },
         {
            name = "empire_lumberjack",
            helptexts = {

            }
         },
         {
            name = "empire_forester",
            helptexts = {

            }
         },
         {
            name = "empire_builder",
            helptexts = {

            }
         },
         {
            name = "empire_shepherd",
            helptexts = {

            }
         },
         {
            name = "empire_weaver",
            helptexts = {

            }
         },
         {
            name = "empire_shipwright",
            helptexts = {

            }
         }
      },
      {
         -- Food
         {
            name = "empire_fisher",
            helptexts = {

            }
         },
         {
            name = "empire_hunter",
            helptexts = {

            }
         },
         {
            name = "empire_farmer",
            helptexts = {

            }
         },
         {
            name = "empire_miller",
            helptexts = {

            }
         },
         {
            name = "empire_baker",
            helptexts = {

            }
         },
         {
            name = "empire_brewer",
            helptexts = {

            }
         },
         {
            name = "empire_vinefarmer",
            helptexts = {

            }
         },
         {
            name = "empire_vintner",
            helptexts = {

            }
         },
         {
            name = "empire_pigbreeder",
            helptexts = {

            }
         },
         {
            name = "empire_innkeeper",
            helptexts = {

            }
         }
      },
      {
         -- Mining
         {
            name = "empire_geologist",
            helptexts = {

            }
         },
         {
            name = "empire_miner",
            helptexts = {

            }
         },
         {
            name = "empire_miner_master",
            helptexts = {

            }
         },
         {
            name = "empire_charcoal_burner",
            helptexts = {

            }
         },
         {
            name = "empire_smelter",
            helptexts = {

            }
         }
      },
      {
         -- Tools
         {
            name = "empire_toolsmith",
            helptexts = {

            }
         }
      },
      {
         -- Military
         {
            name = "empire_recruit",
            helptexts = {

            }
         },
         {
            name = "empire_soldier",
            default_target_quantity = 10,
            preciousness = 5,
            helptexts = {

            }
         },
         {
            name = "empire_trainer",
            helptexts = {

            }
         },
         {
            name = "empire_weaponsmith",
            helptexts = {

            }
         },
         {
            name = "empire_armorsmith",
            helptexts = {

            }
         },
         {
            name = "empire_scout",
            helptexts = {

            }
         }
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
         name = "wheatfield_tiny",
         helptexts = {

         }
      },
      {
         name = "wheatfield_small",
         helptexts = {

         }
      },
      {
         name = "wheatfield_medium",
         helptexts = {

         }
      },
      {
         name = "wheatfield_ripe",
         helptexts = {

         }
      },
      {
         name = "wheatfield_harvested",
         helptexts = {

         }
      },
      {
         name = "grapevine_tiny",
         helptexts = {

         }
      },
      {
         name = "grapevine_small",
         helptexts = {

         }
      },
      {
         name = "grapevine_medium",
         helptexts = {

         }
      },
      {
         name = "grapevine_ripe",
         helptexts = {

         }
      },
      {
         name = "empire_resi_none",
         helptexts = {

         }
      },
      {
         name = "empire_resi_water",
         helptexts = {

         }
      },
      {
         name = "empire_resi_coal_1",
         helptexts = {

         }
      },
      {
         name = "empire_resi_iron_1",
         helptexts = {

         }
      },
      {
         name = "empire_resi_gold_1",
         helptexts = {

         }
      },
      {
         name = "empire_resi_stones_1",
         helptexts = {

         }
      },
      {
         name = "empire_resi_coal_2",
         helptexts = {

         }
      },
      {
         name = "empire_resi_iron_2",
         helptexts = {

         }
      },
      {
         name = "empire_resi_gold_2",
         helptexts = {

         }
      },
      {
         name = "empire_resi_stones_2",
         helptexts = {

         }
      },
      {
         name = "empire_shipconstruction",
         helptexts = {

         }
      }
   },

   -- The order here also determines the order in lists on screen.
   buildings = {
      -- Warehouses
      {
         name = "empire_headquarters",
         helptexts = {

         }
      },
      {
         name = "empire_headquarters_shipwreck",
         helptexts = {

         }
      },
      {
         name = "empire_warehouse",
         helptexts = {

         }
      },
      {
         name = "empire_port",
         helptexts = {

         }
      },

      -- Small
      {
         name = "empire_quarry",
         helptexts = {

         }
      },
      {
         name = "empire_lumberjacks_house",
         helptexts = {

         }
      },
      {
         name = "empire_foresters_house",
         helptexts = {

         }
      },
      {
         name = "empire_fishers_house",
         helptexts = {

         }
      },
      {
         name = "empire_hunters_house",
         helptexts = {

         }
      },
      {
         name = "empire_well",
         helptexts = {

         }
      },
      {
         name = "empire_scouts_house",
         helptexts = {

         }
      },

      -- Medium
      {
         name = "empire_stonemasons_house",
         helptexts = {

         }
      },
      {
         name = "empire_sawmill",
         helptexts = {

         }
      },
      {
         name = "empire_mill",
         helptexts = {

         }
      },
      {
         name = "empire_bakery",
         helptexts = {

         }
      },
      {
         name = "empire_brewery",
         helptexts = {

         }
      },
      {
         name = "empire_vineyard",
         helptexts = {

         }
      },
      {
         name = "empire_winery",
         helptexts = {

         }
      },
      {
         name = "empire_tavern",
         helptexts = {

         }
      },
      {
         name = "empire_inn",
         helptexts = {

         }
      },
      {
         name = "empire_charcoal_kiln",
         helptexts = {

         }
      },
      {
         name = "empire_smelting_works",
         helptexts = {

         }
      },
      {
         name = "empire_toolsmithy",
         helptexts = {

         }
      },
      {
         name = "empire_armorsmithy",
         helptexts = {

         }
      },
      {
         name = "empire_barracks",
         helptexts = {

         }
      },

      -- Big
      {
         name = "empire_donkeyfarm",
         helptexts = {

         }
      },
      {
         name = "empire_farm",
         helptexts = {

         }
      },
      {
         name = "empire_piggery",
         helptexts = {

         }
      },
      {
         name = "empire_sheepfarm",
         helptexts = {

         }
      },
      {
         name = "empire_weaving_mill",
         helptexts = {

         }
      },
      {
         name = "empire_weaponsmithy",
         helptexts = {

         }
      },

      -- Mines
      {
         name = "empire_marblemine",
         helptexts = {

         }
      },
      {
         name = "empire_marblemine_deep",
         helptexts = {

         }
      },
      {
         name = "empire_coalmine",
         helptexts = {

         }
      },
      {
         name = "empire_coalmine_deep",
         helptexts = {

         }
      },
      {
         name = "empire_ironmine",
         helptexts = {

         }
      },
      {
         name = "empire_ironmine_deep",
         helptexts = {

         }
      },
      {
         name = "empire_goldmine",
         helptexts = {

         }
      },
      {
         name = "empire_goldmine_deep",
         helptexts = {

         }
      },

      -- Training Sites
      {
         name = "empire_arena",
         helptexts = {

         }
      },
      {
         name = "empire_colosseum",
         helptexts = {

         }
      },
      {
         name = "empire_trainingcamp",
         helptexts = {

         }
      },

      -- Military Sites
      {
         name = "empire_blockhouse",
         helptexts = {

         }
      },
      {
         name = "empire_sentry",
         helptexts = {

         }
      },
      {
         name = "empire_outpost",
         helptexts = {

         }
      },
      {
         name = "empire_barrier",
         helptexts = {

         }
      },
      {
         name = "empire_tower",
         helptexts = {

         }
      },
      {
         name = "empire_fortress",
         helptexts = {

         }
      },
      {
         name = "empire_castle",
         helptexts = {

         }
      },

      -- Seafaring/Ferry Sites - these are only displayed on seafaring/ferry maps
      {
         name = "empire_ferry_yard",
         helptexts = {

         }
      },
      {
         name = "empire_shipyard",
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
   builder = "empire_builder",
   carrier = "empire_carrier",
   carrier2 = "empire_donkey",
   geologist = "empire_geologist",
   soldier = "empire_soldier",
   ship = "empire_ship",
   ferry = "empire_ferry",
   port = "empire_port",
}

pop_textdomain()
