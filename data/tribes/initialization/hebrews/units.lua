descriptions = wl.Descriptions() -- TODO(matthiakl): only for savegame compatibility with 1.0, do not use.

-- Remaining TODOs:
-- seafaring (ship, shipyard, ferry_yard)
-- prophet's hut (mountain-only building, extended vision)
-- additional military sites (barrier, tower)
-- fisher works with net instead of rod
-- create unique graphics + animations for all buildings and workers

image_dirname = path.dirname(__file__) .. "images/"

push_textdomain("tribes_encyclopedia")

-- For formatting time strings
include "tribes/scripting/help/time_strings.lua"

wl.Descriptions():new_tribe {
   name = "hebrews",
   animation_directory = image_dirname,
   animations = {
      frontier = { hotspot = {8, 31} },
      pinned_note = { hotspot = {18, 67} },
      bridge_normal_e = { hotspot = {-1, 13} },
      bridge_busy_e = { hotspot = {-1, 13} },
      bridge_normal_se = { hotspot = {8, 3} },
      bridge_busy_se = { hotspot = {8, 3} },
      bridge_normal_sw = { hotspot = {41, 3} },
      bridge_busy_sw = { hotspot = {41, 3} }
   },
   spritesheets = {
      flag = {
         fps = 10,
         frames = 16,
         columns = 8,
         rows = 2,
         hotspot = { 12, 40 }
      },
   },

   bridge_height = 8,

   collectors_points_table = {
      { ware = "gold_leaf", points = 3},
      { ware = "menorah", points = 5},
      { ware = "slingshot", points = 2},
      { ware = "dagger", points = 3},
      { ware = "tefilin", points = 4},
      { ware = "tallit_katan", points = 3},
      { ware = "tallit", points = 5},
      { ware = "zizit", points = 1},
      { ware = "log", points = 4},
      { ware = "copper_ore", points = 2},
   },

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
         [0] = "hebrews_resi_none",
      },
      resource_iron = {
         [10] = "hebrews_resi_iron_1",
         [20] = "hebrews_resi_iron_2",
      },
      resource_gold = {
         [10] = "hebrews_resi_gold_1",
         [20] = "hebrews_resi_gold_2",
      },
      resource_stones = {
         [10] = "hebrews_resi_stones_1",
         [20] = "hebrews_resi_stones_2",
      },
      resource_water = {
         [100] = "hebrews_resi_water",
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
            preciousness = 14,
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew ware: Log
               purpose = pgettext("hebrews_ware", "Logs are a vital building material for the Hebrews. Since they cannot produce wood themselves, they must acquire it through trade at the market.")
            }
         },
         {
            name = "granite",
            default_target_quantity = 20,
            preciousness = 5,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a Hebrew ware: Granite, part 1
                  pgettext("ware", "Granite is a basic building material."),
                  -- TRANSLATORS: Helptext for a Hebrew ware: Granite, part 2
                  pgettext("hebrews_ware", "The Hebrews' stonemasons cut granite blocks in quarries and granite mines.")
               }
            }
         },
         {
            name = "clay",
            default_target_quantity = 30,
            preciousness = 3,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a Hebrew ware: Clay, part 1
                  pgettext("ware", "Clay is a basic building material."),
                  -- TRANSLATORS: Helptext for a Hebrew ware: Clay, part 2
                  pgettext("hebrews_ware", "The Hebrews produce clay by digging it from the earth in the clay pit.")
               }
            }
         },
      },
      {
         -- Food
         {
            name = "water",
            preciousness = 8,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a Hebrew ware: Water, part 1
                  pgettext("ware", "Water is the essence of life!"),
                  -- TRANSLATORS: Helptext for a Hebrew ware: Water, part 2
                  pgettext("hebrews_ware", "Water is drawn from wells. It is used in the bakery, the winery and for clay production.")
               }
            }
         },
         {
            name = "wheat",
            preciousness = 12,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a Hebrew ware: Wheat, part 1
                  pgettext("ware", "Wheat is essential for survival."),
                  -- TRANSLATORS: Helptext for a Hebrew ware: Wheat, part 2
                  pgettext("hebrews_ware", "Wheat is produced by farms. It is threshed into grains on the threshing floor and then milled into flour for baking bread.")
               }
            }
         },
         {
            name = "wheat_grains",
            preciousness = 12,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a Hebrew ware: Wheat Grains, part 1
                  pgettext("hebrews_ware", "Wheat grains are threshed from harvested wheat on the threshing floor."),
                  -- TRANSLATORS: Helptext for a Hebrew ware: Wheat Grains, part 2
                  pgettext("hebrews_ware", "They are ground into flour in the mill.")
               }
            }
         },
         {
            name = "flour",
            preciousness = 12,
            default_target_quantity = 10,
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew ware: Flour
               purpose = pgettext("hebrews_ware", "Flour is ground from wheat grains in the mill and used in the bakery to bake bread.")
            }
         },
         {
            name = "bread_hebrews",
            default_target_quantity = 20,
            preciousness = 4,
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew ware: Bread
               purpose = pgettext("hebrews_ware", "Bread is the main food for the Hebrews. It is baked from flour and water in the bakery and used to feed miners and soldiers.")
            }
         },
         {
            name = "fish",
            preciousness = 3,
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew ware: Fish
               purpose = pgettext("hebrews_ware", "Fish is caught by fishers and used to feed miners and soldiers.")
            }
         },
         {
            name = "meat",
            preciousness = 3,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a Hebrew ware: Meat, part 1
                  pgettext("hebrews_ware", "Meat is produced by the butchery from sheep."),
                  -- TRANSLATORS: Helptext for a Hebrew ware: Meat, part 2
                  pgettext("hebrews_ware", "It is used to feed miners and soldiers.")
               }
            }
         },
         {
            name = "olives",
            default_target_quantity = 20,
            preciousness = 4,
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew ware: Olives
               purpose = pgettext("hebrews_ware", "Olives are harvested in olive plantations and pressed into olive oil in the mill.")
            }
         },
         {
            name = "olive_oil",
            default_target_quantity = 20,
            preciousness = 4,
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew ware: Olive Oil
               purpose = pgettext("hebrews_ware", "Olive oil is pressed from olives in the mill. It is used for food and in religious ceremonies.")
            }
         },
         {
            name = "grape",
            default_target_quantity = 20,
            preciousness = 4,
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew ware: Grapes
               purpose = pgettext("hebrews_ware", "Grapes are harvested in vineyards and used in the winery to produce wine.")
            }
         },
         {
            name = "wine",
            default_target_quantity = 20,
            preciousness = 4,
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew ware: Wine
               purpose = pgettext("hebrews_ware", "Wine is produced in the winery from grapes and water. It is used to feed miners and soldiers.")
            }
         },
      },
      {
         -- Mining and Smelting
         {
            name = "branch",
            default_target_quantity = 20,
            preciousness = 8,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a Hebrew ware: Branch, part 1
                  pgettext("hebrews_ware", "Branches are collected from the ground near trees by the branch collector."),
                  -- TRANSLATORS: Helptext for a Hebrew ware: Branch, part 2
                  pgettext("hebrews_ware", "They are used to fuel the clay furnace for smelting copper.")
               }
            }
         },
         {
            name = "copper_ore",
            default_target_quantity = 15,
            preciousness = 4,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a Hebrew ware: Copper Ore, part 1
                  pgettext("ware", "Copper ore is mined in copper mines."),
                  -- TRANSLATORS: Helptext for a Hebrew ware: Copper Ore, part 2
                  pgettext("hebrews_ware", "It is smelted into copper in the clay furnace and used in the weapon smithy to forge daggers.")
               }
            }
         },
         {
            name = "copper",
            default_target_quantity = 15,
            preciousness = 4,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a Hebrew ware: Copper, part 1
                  pgettext("hebrews_ware", "Copper is smelted from copper ore in the clay furnace."),
                  -- TRANSLATORS: Helptext for a Hebrew ware: Copper, part 2
                  pgettext("hebrews_ware", "It is used in the weapon smithy to forge daggers.")
               }
            }
         },
         {
            name = "gold_ore",
            default_target_quantity = 15,
            preciousness = 2,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a Hebrew ware: Gold Ore, part 1
                  pgettext("ware", "Gold ore is mined in a gold mine."),
                  -- TRANSLATORS: Helptext for a Hebrew ware: Gold Ore, part 2
                  pgettext("hebrews_ware", "It is refined into gold leaf and menorahs in the gold beater's workshop.")
               }
            }
         },
         {
            name = "gold_leaf",
            default_target_quantity = 10,
            preciousness = 3,
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew ware: Gold Leaf
               purpose = pgettext("hebrews_ware", "Gold leaf is beaten from gold ore in the gold beater's workshop. It is used to produce tefilin and tallitot.")
            }
         },
         {
            name = "menorah",
            default_target_quantity = 5,
            preciousness = 5,
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew ware: Menorah
               purpose = pgettext("hebrews_ware", "The menorah is a sacred seven-branched candelabrum made from gold ore. It is used to train soldiers in the training camp.")
            }
         },
      },
      {
         -- Textile and Animal Products
         {
            name = "sheep2",
            default_target_quantity = 10,
            preciousness = 8,
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew ware: Sheep
               purpose = pgettext("hebrews_ware", "Sheep are raised by shepherds. They are brought to the butchery to produce wool, meat and fur.")
            }
         },
         {
            name = "wool",
            default_target_quantity = 10,
            preciousness = 8,
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew ware: Wool
               purpose = pgettext("hebrews_ware", "Wool is produced as a byproduct of the butchery when processing sheep. It is spun into yarn in the spinning mill.")
            }
         },
         {
            name = "fur",
            default_target_quantity = 10,
            preciousness = 8,
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew ware: Fur
               purpose = pgettext("hebrews_ware", "Fur is a byproduct of the butchery when processing sheep. It is used to produce tefilin and slingshots.")
            }
         },
         {
            name = "yarn",
            default_target_quantity = 10,
            preciousness = 8,
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew ware: Yarn
               purpose = pgettext("hebrews_ware", "Yarn is spun from wool in the spinning mill. It is used to weave cloth and to produce zizit.")
            }
         },
         {
            name = "cloth",
            default_target_quantity = 10,
            preciousness = 8,
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew ware: Cloth
               purpose = pgettext("hebrews_ware", "Cloth is woven from yarn in the weaving mill. It is used to produce tallitot and tunics in the dressmakery.")
            }
         },
      },
      {
         -- Tools
         {
            name = "pick",
            default_target_quantity = 2,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew ware: Pick
               purpose = pgettext("hebrews_ware", "Picks are used by stonemasons and miners. They are crafted in the workshop.")
            }
         },
         {
            name = "hammer",
            default_target_quantity = 2,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a Hebrew ware: Hammer, part 1
                  pgettext("ware", "The hammer is an essential tool."),
                  -- TRANSLATORS: Helptext for a Hebrew ware: Hammer, part 2
                  pgettext("hebrews_ware", "Geologists and builders all need a hammer. They are crafted in the workshop.")
               }
            }
         },
         {
            name = "fishing_rod",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a Hebrew ware: Fishing Rod, part 1
                  pgettext("ware", "Fishing rods are needed by fishers to catch fish."),
                  -- TRANSLATORS: Helptext for a Hebrew ware: Fishing Rod, part 2
                  pgettext("hebrews_ware", "Fishing rods are crafted in the workshop.")
               }
            }
         },
      },
      {
         -- Religious Items
         {
            name = "zizit",
            default_target_quantity = 10,
            preciousness = 3,
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew ware: Zizit
               purpose = pgettext("hebrews_ware", "Zizit are ritual fringes made from yarn. They are attached to garments as commanded by the Torah and used to train soldiers.")
            }
         },
         {
            name = "tallit_katan",
            default_target_quantity = 5,
            preciousness = 3,
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew ware: Tallit Katan
               purpose = pgettext("hebrews_ware", "A tallit katan is a ritual undergarment produced in the dressmakery. It provides spiritual protection and is used to train soldiers.")
            }
         },
         {
            name = "tefilin",
            default_target_quantity = 5,
            preciousness = 4,
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew ware: Tefilin
               purpose = pgettext("hebrews_ware", "Tefilin are small leather boxes containing Torah scrolls, produced in the workshop from fur and gold leaf. They are used to train soldiers.")
            }
         },
         {
            name = "tallit",
            default_target_quantity = 5,
            preciousness = 5,
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew ware: Tallit
               purpose = pgettext("hebrews_ware", "The tallit is a prayer shawl produced in the dressmakery from cloth and gold leaf. It provides the highest spiritual protection and is used to train soldiers.")
            }
         },
      },
      {
         -- Weapons & Armor
         {
            name = "tunic",
            default_target_quantity = 5,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew ware: Tunic
               purpose = pgettext("hebrews_ware", "A basic tunic is produced in the dressmakery from cloth. It is the initial armor for Hebrew soldiers.")
            }
         },
         {
            name = "slingshot",
            default_target_quantity = 10,
            preciousness = 2,
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew ware: Slingshot
               purpose = pgettext("hebrews_ware", "The slingshot is a basic ranged weapon forged in the weapon smithy from fur and granite. It is used to train soldiers in attack.")
            }
         },
         {
            name = "dagger",
            default_target_quantity = 5,
            preciousness = 3,
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew ware: Dagger
               purpose = pgettext("hebrews_ware", "A copper dagger is forged in the weapon smithy from copper and fur. It is the strongest melee weapon of Hebrew soldiers.")
            }
         },
      }
   },

   -- Workers positions in workers windows.
   -- This also gives us the information which workers the tribe uses.
   -- Each subtable is a column in the workers windows.
   workers_order = {
      {
         -- Carriers
         {
            name = "hebrews_carrier",
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew worker: Carrier
               purpose = pgettext("hebrews_worker", "Carries items along your roads.")
            }
         },
         {
            name = "hebrews_ferry",
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew worker: Ferry
               purpose = pgettext("hebrews_worker", "Ships wares across narrow rivers.")
            }
         },
         {
            name = "hebrews_donkey",
            default_target_quantity = 10,
            preciousness = 2,
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew worker: Donkey
               purpose = pgettext("hebrews_worker", "Donkeys help carry items along busy roads. They are raised on a donkey farm.")
            }
         },
      },
      {
         -- Building and Resources
         {
            name = "hebrews_builder",
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew worker: Builder
               purpose = pgettext("hebrews_worker", "Works at construction sites to raise new buildings.")
            }
         },
         {
            name = "hebrews_stonemason",
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew worker: Stonemason
               purpose = pgettext("hebrews_worker", "Cuts raw pieces of granite out of rocks in the vicinity.")
            }
         },
         {
            name = "hebrews_branch_collector",
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew worker: Branch Collector
               purpose = pgettext("hebrews_worker", "Collects fallen branches from the ground near trees. The trees are not harmed in the process.")
            }
         },
         {
            name = "hebrews_miner",
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew worker: Miner
               purpose = pgettext("hebrews_worker", "Works deep in the mines to obtain copper ore, gold ore or granite.")
            }
         },
         {
            name = "hebrews_geologist",
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew worker: Geologist
               purpose = pgettext("hebrews_worker", "Discovers resources for mining.")
            }
         },
         {
            name = "hebrews_scout",
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew worker: Scout
               purpose = pgettext("hebrews_worker", "Explores unknown territory.")
            }
         },
      },
      {
         -- Food and Farming
         {
            name = "hebrews_fisher",
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew worker: Fisher
               purpose = pgettext("hebrews_worker", "Catches fish in the nearby waters.")
            }
         },
         {
            name = "hebrews_farmer",
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew worker: Farmer
               purpose = pgettext("hebrews_worker", "Plants and harvests wheat fields.")
            }
         },
         {
            name = "hebrews_shepherd",
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew worker: Shepherd
               purpose = pgettext("hebrews_worker", "Raises sheep and brings them to the butchery for processing.")
            }
         },
      },
      {
         -- Education (Yeshiva track)
         {
            name = "hebrews_talmid",
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew worker: Talmid
               purpose = pgettext("hebrews_worker", "A student who has begun Torah study. Talmidim work in specialized buildings like the clay furnace. They are educated in the Yeshiva.")
            }
         },
         {
            name = "hebrews_talmid_chacham",
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew worker: Talmid Chacham
               purpose = pgettext("hebrews_worker", "A wise student who has completed advanced studies. Talmidei Chachamim work in the gold beater's workshop and other skilled positions. They are educated in the Yeshiva.")
            }
         },
         {
            name = "hebrews_rebbe",
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew worker: Rebbe
               purpose = pgettext("hebrews_worker", "A Torah scholar and teacher. Rebbes lead the Yeshiva and serve as geologists. They are educated in the Yeshiva from Talmidei Chachamim.")
            }
         },
      },
      {
         -- Military
         {
            name = "hebrews_soldier",
            default_target_quantity = 30,
            preciousness = 5,
            helptexts = {
               -- TRANSLATORS: Helptext for a Hebrew worker: Soldier
               purpose = pgettext("hebrews_worker", "Soldiers defend your territory and conquer new land. They are trained in the Machane and gain experience in the training camp.")
            }
         },
      }
   },

   immovables = {
      {
         name = "ashes",
         helptexts = {
            -- TRANSLATORS: Helptext for a Hebrew immovable: Ashes
            purpose = _("The remains of a destroyed building.")
         }
      },
      {
         name = "destroyed_building",
         helptexts = {
            -- TRANSLATORS: Helptext for a Hebrew immovable: Destroyed Building
            purpose = _("The remains of a destroyed building.")
         }
      },
      {
         name = "wheatfield_tiny",
         helptexts = {
            -- TRANSLATORS: Helptext for a Hebrew immovable: Wheat field
            purpose = _("This field has just been planted.")
         }
      },
      {
         name = "wheatfield_small",
         helptexts = {
            -- TRANSLATORS: Helptext for a Hebrew immovable: Wheat field
            purpose = _("This field is growing.")
         }
      },
      {
         name = "wheatfield_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for a Hebrew immovable: Wheat field
            purpose = _("This field is growing.")
         }
      },
      {
         name = "wheatfield_ripe",
         helptexts = {
            -- TRANSLATORS: Helptext for a Hebrew immovable: Wheat field
            purpose = _("This field is ready for harvesting.")
         }
      },
      {
         name = "wheatfield_harvested",
         helptexts = {
            -- TRANSLATORS: Helptext for a Hebrew immovable: Wheat field
            purpose = _("This field has been harvested.")
         }
      },
      {
         name = "hebrews_resi_none",
         helptexts = {
            -- TRANSLATORS: Helptext for a Hebrew resource indicator: No resources
            purpose = _("There are no resources in the ground here.")
         }
      },
      {
         name = "hebrews_resi_iron_1",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for a Hebrew resource indicator: Copper, part 1
               _("Copper veins contain copper ore that can be dug up by copper mines."),
               -- TRANSLATORS: Helptext for a Hebrew resource indicator: Copper, part 2
               _("There is only a little bit of copper here.")
            }
         }
      },
      {
         name = "hebrews_resi_gold_1",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for a Hebrew resource indicator: Gold, part 1
               _("Gold veins contain gold ore that can be dug up by gold mines."),
               -- TRANSLATORS: Helptext for a Hebrew resource indicator: Gold, part 2
               _("There is only a little bit of gold here.")
            }
         }
      },
      {
         name = "hebrews_resi_stones_1",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for a Hebrew resource indicator: Stones, part 1
               _("Granite is a basic building material and can be dug up by a granite mine."),
               -- TRANSLATORS: Helptext for a Hebrew resource indicator: Stones, part 2
               _("There is only a little bit of granite here."),
            }
         }
      },
      {
         name = "hebrews_resi_iron_2",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for a Hebrew resource indicator: Copper, part 1
               _("Copper veins contain copper ore that can be dug up by copper mines."),
               -- TRANSLATORS: Helptext for a Hebrew resource indicator: Copper, part 2
               _("There is a lot of copper here.")
            }
         }
      },
      {
         name = "hebrews_resi_gold_2",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for a Hebrew resource indicator: Gold, part 1
               _("Gold veins contain gold ore that can be dug up by gold mines."),
               -- TRANSLATORS: Helptext for a Hebrew resource indicator: Gold, part 2
               _("There is a lot of gold here.")
            }
         }
      },
      {
         name = "hebrews_resi_stones_2",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for a Hebrew resource indicator: Stones, part 1
               _("Granite is a basic building material and can be dug up by a granite mine."),
               -- TRANSLATORS: Helptext for a Hebrew resource indicator: Stones, part 2
               _("There is a lot of granite here.")
            }
         }
      },
      {
         name = "hebrews_resi_water",
         helptexts = {
            -- TRANSLATORS: Helptext for a Hebrew resource indicator: Water
            purpose = _("There is water in the ground here that can be pulled up by a well.")
         }
      },
      -- TODO: hebrews_shipconstruction (seafaring not yet implemented)
      -- Non-Hebrew immovables used by the woodcutter
      {
         name = "deadtree7",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable usable by the Hebrews: Dead Tree
            purpose = _("The remains of an old tree.")
         }
      },
      {
         name = "balsa_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable usable by the Hebrews: Balsa Tree
            purpose = _("This tree is only planted by the Amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_black_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable usable by the Hebrews: Balsa Tree
            purpose = _("This tree is only planted by the Amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_desert_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable usable by the Hebrews: Balsa Tree
            purpose = _("This tree is only planted by the Amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_winter_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable usable by the Hebrews: Balsa Tree
            purpose = _("This tree is only planted by the Amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "grapevine_tiny",
         helptexts = {
            -- TRANSLATORS: Helptext for a Hebrew immovable: Grapevine
            purpose = _("This grapevine has just been planted.")
         }
      },
      {
         name = "grapevine_small",
         helptexts = {
            -- TRANSLATORS: Helptext for a Hebrew immovable: Grapevine
            purpose = _("This grapevine is growing.")
         }
      },
      {
         name = "grapevine_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for a Hebrew immovable: Grapevine
            purpose = _("This grapevine is growing.")
         }
      },
      {
         name = "grapevine_ripe",
         helptexts = {
            -- TRANSLATORS: Helptext for a Hebrew immovable: Grapevine
            purpose = _("This grapevine is ready for harvesting.")
         }
      },
      {
         name = "pond_dry",
         helptexts = {
            -- TRANSLATORS: Helptext for a Hebrew immovable: Dry Pond
            purpose = _("A shallow depression left by clay digging. It will fill in naturally over time.")
         }
      },
      {
         name = "ironwood_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable usable by the Hebrews: Ironwood Tree
            purpose = _("This tree is only planted by the Amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_black_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable usable by the Hebrews: Ironwood Tree
            purpose = _("This tree is only planted by the Amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_desert_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable usable by the Hebrews: Ironwood Tree
            purpose = _("This tree is only planted by the Amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_winter_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable usable by the Hebrews: Ironwood Tree
            purpose = _("This tree is only planted by the Amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable usable by the Hebrews: Rubber Tree
            purpose = _("This tree is only planted by the Amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_black_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable usable by the Hebrews: Rubber Tree
            purpose = _("This tree is only planted by the Amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_desert_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable usable by the Hebrews: Rubber Tree
            purpose = _("This tree is only planted by the Amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_winter_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable usable by the Hebrews: Rubber Tree
            purpose = _("This tree is only planted by the Amazon tribe but can be harvested for logs.")
         }
      },
   },

   -- The order here also determines the order in lists on screen.
   buildings = {
      -- Warehouses
      {
         name = "hebrews_headquarters",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew warehouse: Headquarters
            lore = pgettext("hebrews_building", "'The Tabernacle was the dwelling place of the Lord among His people.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew warehouse: Headquarters
            lore_author = pgettext("hebrews_building", "Book of Exodus"),
            -- TRANSLATORS: Purpose helptext for a Hebrew warehouse: Headquarters
            purpose = pgettext("hebrews_building", "Accommodation for your people. Also stores your wares and tools."),
            -- TRANSLATORS: Note helptext for a Hebrew warehouse: Headquarters
            note = pgettext("hebrews_building", "The headquarters is your main building.")
         }
      },
      {
         name = "hebrews_headquarters_tent",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a Hebrew warehouse: Headquarters Tent
            purpose = pgettext("hebrews_building", "Accommodation for your people. Also stores your wares and tools."),
            -- TRANSLATORS: Note helptext for a Hebrew warehouse: Headquarters Tent
            note = pgettext("hebrews_building", "The headquarters tent is your main building.")
         }
      },
      {
         name = "hebrews_warehouse",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew warehouse: Warehouse
            lore = pgettext("hebrews_building", "'Store your grain in times of plenty, for lean years will surely follow.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew warehouse: Warehouse
            lore_author = pgettext("hebrews_building", "Ancient Hebrew proverb"),
            -- TRANSLATORS: Purpose helptext for a Hebrew warehouse: Warehouse
            purpose = pgettext("hebrews_building", "Your workers and soldiers will find shelter here. Also stores your wares and tools.")
         }
      },
      {
         name = "hebrews_port",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew warehouse: Port
            lore = pgettext("hebrews_building", "'King Solomon built a fleet of ships at Ezion-Geber.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew warehouse: Port
            lore_author = pgettext("hebrews_building", "First Book of Kings"),
            -- TRANSLATORS: Purpose helptext for a Hebrew warehouse: Port
            purpose = pgettext("hebrews_building", "Serves as a base for overseas colonization and trade. Also stores your soldiers, wares and tools."),
            -- TRANSLATORS: Note helptext for a Hebrew warehouse: Port
            note = pgettext("hebrews_building", "Similar to the Headquarters a Port can be attacked and destroyed by an enemy. It is recommendable to send soldiers to defend it.")
         }
      },

      -- Small Production Sites
      {
         name = "hebrews_fishers_hut",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew production site: Fisher's Hut
            lore = pgettext("hebrews_building", "'Cast your net on the right side of the boat and you will find some.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew production site: Fisher's Hut
            lore_author = pgettext("hebrews_building", "Gospel of John"),
            -- TRANSLATORS: Purpose helptext for a Hebrew production site: Fisher's Hut
            purpose = pgettext("hebrews_building", "Catches fish in the nearby waters."),
            -- TRANSLATORS: Note helptext for a Hebrew production site: Fisher's Hut
            note = pgettext("hebrews_building", "The fisher needs water nearby to catch fish.")
         }
      },
      {
         name = "hebrews_shepherds",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew production site: Shepherd's Hut
            lore = pgettext("hebrews_building", "'The Lord is my shepherd, I shall not want.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew production site: Shepherd's Hut
            lore_author = pgettext("hebrews_building", "Psalm 23"),
            -- TRANSLATORS: Purpose helptext for a Hebrew production site: Shepherd's Hut
            purpose = pgettext("hebrews_building", "Raises sheep for wool, meat and fur.")
         }
      },
      {
         name = "hebrews_well",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew production site: Well
            lore = pgettext("hebrews_building", "'Then Israel sang this song: Spring up, O well!'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew production site: Well
            lore_author = pgettext("hebrews_building", "Book of Numbers"),
            -- TRANSLATORS: Purpose helptext for a Hebrew production site: Well
            purpose = pgettext("hebrews_building", "Draws water out of the deep.")
         }
      },
      {
         name = "hebrews_quarry",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew production site: Quarry
            lore = pgettext("hebrews_building", "'Solomon's builders and Hiram's builders cut the stone.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew production site: Quarry
            lore_author = pgettext("hebrews_building", "First Book of Kings"),
            -- TRANSLATORS: Purpose helptext for a Hebrew production site: Quarry
            purpose = pgettext("hebrews_building", "Cuts raw pieces of granite out of rocks in the vicinity."),
            -- TRANSLATORS: Note helptext for a Hebrew production site: Quarry
            note = pgettext("hebrews_building", "The quarry needs rocks to cut within the work area.")
         }
      },
      {
         name = "hebrews_clay_pit",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew production site: Clay Pit
            lore = pgettext("hebrews_building", "'Like clay in the hand of the potter, so are you in My hand.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew production site: Clay Pit
            lore_author = pgettext("hebrews_building", "Book of Jeremiah"),
            -- TRANSLATORS: Purpose helptext for a Hebrew production site: Clay Pit
            purpose = pgettext("hebrews_building", "Digs clay out of the ground.")
         }
      },
      {
         name = "hebrews_branch_collectors_hut",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew production site: Branch Collector's Hut
            lore = pgettext("hebrews_building", "'For there is hope for a tree, if it be cut down, that it will sprout again, and that its shoots will not cease.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew production site: Branch Collector's Hut
            lore_author = pgettext("hebrews_building", "Book of Job"),
            -- TRANSLATORS: Purpose helptext for a Hebrew production site: Branch Collector's Hut
            purpose = pgettext("hebrews_building", "Collects fallen branches from trees in the vicinity. The trees are not harmed."),
            -- TRANSLATORS: Note helptext for a Hebrew production site: Branch Collector's Hut
            note = pgettext("hebrews_building", "Branches are used to fuel the clay furnace for smelting copper. The branch collector needs trees nearby.")
         }
      },
      {
         name = "hebrews_spinning_mill",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew production site: Spinning Mill
            lore = pgettext("hebrews_building", "'She puts her hands to the distaff, and her hands hold the spindle.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew production site: Spinning Mill
            lore_author = pgettext("hebrews_building", "Book of Proverbs"),
            -- TRANSLATORS: Purpose helptext for a Hebrew production site: Spinning Mill
            purpose = pgettext("hebrews_building", "Spins wool into yarn.")
         }
      },
      {
         name = "hebrews_zizijot_makers_hut",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew production site: Zizijot Maker's Hut
            lore = pgettext("hebrews_building", "'They shall make fringes on the corners of their garments throughout their generations.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew production site: Zizijot Maker's Hut
            lore_author = pgettext("hebrews_building", "Book of Numbers"),
            -- TRANSLATORS: Purpose helptext for a Hebrew production site: Zizijot Maker's Hut
            purpose = pgettext("hebrews_building", "Produces zizit from yarn.")
         }
      },
      {
         name = "hebrews_scouts_hut",
         helptexts = {
            no_scouting_building_connected = pgettext("hebrews_building", "You need to connect this flag to a scout's hut before you can send a scout here."),
            -- TRANSLATORS: Lore helptext for a Hebrew production site: Scout's Hut
            lore = pgettext("hebrews_building", "'Send men to spy out the land of Canaan, which I am giving to the people of Israel.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew production site: Scout's Hut
            lore_author = pgettext("hebrews_building", "Book of Numbers"),
            -- TRANSLATORS: Purpose helptext for a Hebrew production site: Scout's Hut
            purpose = pgettext("building", "Explores unknown territory.")
         }
      },

      -- Medium Production Sites
      {
         name = "hebrews_mill",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew production site: Mill
            lore = pgettext("hebrews_building", "'Two women will be grinding at the mill.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew production site: Mill
            lore_author = pgettext("hebrews_building", "Gospel of Matthew"),
            -- TRANSLATORS: Purpose helptext for a Hebrew production site: Mill
            purpose = pgettext("hebrews_building", "Grinds wheat grains into flour and presses olives into olive oil.")
         }
      },
      {
         name = "hebrews_bakery",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew production site: Bakery
            lore = pgettext("hebrews_building", "'Give us this day our daily bread.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew production site: Bakery
            lore_author = pgettext("hebrews_building", "Traditional prayer"),
            -- TRANSLATORS: Purpose helptext for a Hebrew production site: Bakery
            purpose = pgettext("hebrews_building", "Bakes bread from flour and water for soldiers and miners."),
            -- TRANSLATORS: Performance helptext for a Hebrew production site: Bakery
            performance = pgettext("hebrews_building", "If all needed wares are delivered in time, this building can produce a bread in %s on average."):bformat(format_seconds(34))
         }
      },
      {
         name = "hebrews_butchery",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew production site: Butchery
            lore = pgettext("hebrews_building", "'Every moving thing that lives shall be food for you.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew production site: Butchery
            lore_author = pgettext("hebrews_building", "Book of Genesis"),
            -- TRANSLATORS: Purpose helptext for a Hebrew production site: Butchery
            purpose = pgettext("hebrews_building", "Slaughters sheep to produce meat, wool and fur.")
         }
      },
      {
         name = "hebrews_weaving_mill",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew production site: Weaving Mill
            lore = pgettext("hebrews_building", "'She seeks wool and flax, and works with willing hands.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew production site: Weaving Mill
            lore_author = pgettext("hebrews_building", "Book of Proverbs"),
            -- TRANSLATORS: Purpose helptext for a Hebrew production site: Weaving Mill
            purpose = pgettext("hebrews_building", "Weaves yarn into cloth.")
         }
      },
      {
         name = "hebrews_winery",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew production site: Winery
            lore = pgettext("hebrews_building", "'Wine gladdens the heart of man.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew production site: Winery
            lore_author = pgettext("hebrews_building", "Psalm 104"),
            -- TRANSLATORS: Purpose helptext for a Hebrew production site: Winery
            purpose = pgettext("hebrews_building", "Produces wine from grapes and water.")
         }
      },
      {
         name = "hebrews_dressmakery",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew production site: Dressmakery
            lore = pgettext("hebrews_building", "'And they sewed fig leaves together and made themselves loincloths.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew production site: Dressmakery
            lore_author = pgettext("hebrews_building", "Book of Genesis"),
            -- TRANSLATORS: Purpose helptext for a Hebrew production site: Dressmakery
            purpose = pgettext("hebrews_building", "Produces tunics, tallit katan and tallitot from cloth and gold leaf for soldiers.")
         }
      },
      {
         name = "hebrews_clay_furnace",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew production site: Clay Furnace
            lore = pgettext("hebrews_building", "'As silver is melted in the midst of the furnace, so shall ye be melted in the midst thereof.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew production site: Clay Furnace
            lore_author = pgettext("hebrews_building", "Book of Ezekiel"),
            -- TRANSLATORS: Purpose helptext for a Hebrew production site: Clay Furnace
            purpose = pgettext("hebrews_building", "Smelts copper ore into copper using branches as fuel.")
         }
      },
      {
         name = "hebrews_gold_beater",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew production site: Gold Beater's Workshop
            lore = pgettext("hebrews_building", "'And they did beat the gold into thin plates, and cut it into wires.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew production site: Gold Beater's Workshop
            lore_author = pgettext("hebrews_building", "Book of Exodus"),
            -- TRANSLATORS: Purpose helptext for a Hebrew production site: Gold Beater's Workshop
            purpose = pgettext("hebrews_building", "Beats gold ore into gold leaf and crafts sacred menorahs.")
         }
      },
      {
         name = "hebrews_workshop",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew production site: Workshop
            lore = pgettext("hebrews_building", "'The craftsman stretches out his arm, he shapes it with hammers.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew production site: Workshop
            lore_author = pgettext("hebrews_building", "Book of Isaiah"),
            -- TRANSLATORS: Purpose helptext for a Hebrew production site: Workshop
            purpose = pgettext("hebrews_building", "Crafts tools such as picks, hammers and fishing rods. Also produces tefilin from fur and gold leaf.")
         }
      },
      {
         name = "hebrews_weaponsmithy",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew production site: Weaponsmithy
            lore = pgettext("hebrews_building", "'They shall beat their swords into plowshares.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew production site: Weaponsmithy
            lore_author = pgettext("hebrews_building", "Book of Isaiah"),
            -- TRANSLATORS: Purpose helptext for a Hebrew production site: Weaponsmithy
            purpose = pgettext("hebrews_building", "Forges slingshots from fur and granite, and copper daggers from copper and fur for soldiers.")
         }
      },
      {
         name = "hebrews_donkeyfarm",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew production site: Donkey Farm
            lore = pgettext("hebrews_building", "'Issachar is a strong donkey, crouching between the sheepfolds.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew production site: Donkey Farm
            lore_author = pgettext("hebrews_building", "Book of Genesis"),
            -- TRANSLATORS: Purpose helptext for a Hebrew production site: Donkey Farm
            purpose = pgettext("hebrews_building", "Breeds donkeys for adding them to the transportation system."),
            -- TRANSLATORS: Performance helptext for a Hebrew production site: Donkey Farm
            performance = pgettext("hebrews_building", "If all needed wares are delivered in time, this building can produce a donkey in %s on average."):bformat(format_seconds(30))
         }
      },
      {
         name = "hebrews_machane",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew production site: Machane
            lore = pgettext("hebrews_building", "'Be strong and courageous, for the Lord your God is with you wherever you go.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew production site: Machane
            lore_author = pgettext("hebrews_building", "Book of Joshua"),
            -- TRANSLATORS: Purpose helptext for a Hebrew production site: Machane
            purpose = pgettext("hebrews_building", "Turns carriers into soldiers ready for battle.")
         }
      },

      -- Big Production Sites
      {
         name = "hebrews_farm",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew production site: Farm
            lore = pgettext("hebrews_building", "'He who tills his land will have plenty of bread.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew production site: Farm
            lore_author = pgettext("hebrews_building", "Book of Proverbs"),
            -- TRANSLATORS: Purpose helptext for production site: Farm
            purpose = pgettext("building", "Sows and harvests wheat."),
            -- TRANSLATORS: Performance helptext for a Hebrew production site: Farm
            performance = pgettext("hebrews_building", "The farmer needs %1% on average to sow and harvest a sheaf of wheat."):bformat(format_minutes_seconds(1, 40))
         }
      },
      {
         name = "hebrews_oliveplant",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew production site: Olive Plantation
            lore = pgettext("hebrews_building", "'The olive tree said: Should I leave my richness, by which God and men are honoured?'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew production site: Olive Plantation
            lore_author = pgettext("hebrews_building", "Book of Judges"),
            -- TRANSLATORS: Purpose helptext for a Hebrew production site: Olive Plantation
            purpose = pgettext("hebrews_building", "Plants and harvests olives.")
         }
      },
      {
         name = "hebrews_yeshiva",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew production site: Yeshiva
            lore = pgettext("hebrews_building", "'The study of Torah is equal to all other commandments.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew production site: Yeshiva
            lore_author = pgettext("hebrews_building", "Talmud, Shabbat 127a"),
            -- TRANSLATORS: Purpose helptext for a Hebrew production site: Yeshiva
            purpose = pgettext("hebrews_building", "Educates workers to higher levels of learning: Sabalim become Talmidim, Talmidim become Talmidei Chachamim, and Talmidei Chachamim become Rebbes."),
            -- TRANSLATORS: Note helptext for a Hebrew production site: Yeshiva
            note = pgettext("hebrews_building", "Requires a Rebbe to run the Yeshiva. Educated workers are needed to staff specialized buildings."),
            -- TRANSLATORS: Performance helptext for a Hebrew production site: Yeshiva
            performance = pgettext("hebrews_building", "If all needed wares and workers are delivered in time, the Yeshiva needs about %s to educate a worker to the next level."):bformat(format_seconds(30))
         }
      },
      {
         name = "hebrews_market",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew building: Market
            lore = pgettext("hebrews_building", "'King Solomon excelled all the kings of the earth in riches and trade.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew building: Market
            lore_author = pgettext("hebrews_building", "First Book of Kings"),
            -- TRANSLATORS: Purpose helptext for a Hebrew building: Market
            purpose = pgettext("hebrews_building", "Enables trade with other players. The Hebrews rely on the market to trade gold for the wood they cannot produce.")
         }
      },

      -- Mines
      {
         name = "hebrews_coppermine",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew production site: Copper Mine
            lore = pgettext("hebrews_building", "'The land where you will dig copper out of the hills.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew production site: Copper Mine
            lore_author = pgettext("hebrews_building", "Book of Deuteronomy"),
            -- TRANSLATORS: Purpose helptext for a Hebrew production site: Copper Mine
            purpose = pgettext("hebrews_building", "Digs copper ore out of the ground in mountain terrain."),
            -- TRANSLATORS: Performance helptext for a Hebrew production site: Copper Mine
            performance = pgettext("hebrews_building", "If the food supply is steady, this mine can produce copper ore in %s on average."):bformat(format_minutes_seconds(1, 9))
         }
      },
      {
         name = "hebrews_goldmine",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew production site: Gold Mine
            lore = pgettext("hebrews_building", "'The gold of that land is good.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew production site: Gold Mine
            lore_author = pgettext("hebrews_building", "Book of Genesis"),
            -- TRANSLATORS: Purpose helptext for a Hebrew production site: Gold Mine
            purpose = pgettext("hebrews_building", "Digs gold ore out of the ground in mountain terrain."),
            -- TRANSLATORS: Performance helptext for a Hebrew production site: Gold Mine
            performance = pgettext("hebrews_building", "If the food supply is steady, this mine can produce gold ore in %s on average."):bformat(format_minutes_seconds(1, 9))
         }
      },
      {
         name = "hebrews_granitemine",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew production site: Granite Mine
            lore = pgettext("hebrews_building", "'Solomon's builders cut the great stones to lay the foundation of the temple.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew production site: Granite Mine
            lore_author = pgettext("hebrews_building", "First Book of Kings"),
            -- TRANSLATORS: Purpose helptext for a Hebrew production site: Granite Mine
            purpose = pgettext("hebrews_building", "Carves granite out of the rock in mountain terrain."),
            -- TRANSLATORS: Note helptext for a Hebrew production site: Granite Mine
            note = pgettext("hebrews_building", "It cannot be enhanced."),
            -- TRANSLATORS: Performance helptext for a Hebrew production site: Granite Mine
            performance = pgettext("hebrews_building", "If the food supply is steady, this mine can produce granite in %s on average."):bformat(format_seconds(24))
         }
      },
      {
         name = "hebrews_threshing_floor",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew production site: Threshing Floor
            lore = pgettext("hebrews_building", "'Ornan was threshing wheat on the threshing floor.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew production site: Threshing Floor
            lore_author = pgettext("hebrews_building", "First Book of Chronicles"),
            -- TRANSLATORS: Purpose helptext for a Hebrew production site: Threshing Floor
            purpose = pgettext("hebrews_building", "Threshes harvested wheat into wheat grains.")
         }
      },
      {
         name = "hebrews_vineyard",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew production site: Vineyard
            lore = pgettext("hebrews_building", "'Noah began to be a man of the soil, and he planted a vineyard.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew production site: Vineyard
            lore_author = pgettext("hebrews_building", "Book of Genesis"),
            -- TRANSLATORS: Purpose helptext for a Hebrew production site: Vineyard
            purpose = pgettext("hebrews_building", "Cultivates grapevines and harvests grapes.")
         }
      },

      -- Training Sites
      {
         name = "hebrews_trainingcamp",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew training site: Training Camp
            lore = pgettext("hebrews_building", "'Blessed be the Lord my Rock, who trains my hands for war and my fingers for battle.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew training site: Training Camp
            lore_author = pgettext("hebrews_building", "Psalm 144"),
            purpose = {
               -- TRANSLATORS: Purpose helptext for a Hebrew training site: Training Camp, part 1
               pgettext("hebrews_building", "Trains soldiers in 'Attack', 'Health' and 'Evade'."),
               -- TRANSLATORS: Purpose helptext for a Hebrew training site: Training Camp, part 2
               pgettext("hebrews_building", "Equips the soldiers with all necessary weapons and armor parts.")
            },
            -- TRANSLATORS: Note helptext for a Hebrew training site: Training Camp
            note = pgettext("hebrews_building", "Hebrew soldiers cannot be trained in 'Defense' and will remain at their initial level."),
            -- TRANSLATORS: Performance helptext for a Hebrew training site: Training Camp
            performance = pgettext("hebrews_building", "If all needed wares are delivered in time, a training camp can train one new soldier in attack, health and evade to the final level in %s on average."):bformat(format_minutes_seconds(5, 0))
         }
      },

      -- Military Sites
      {
         name = "hebrews_tent_small",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew military site: Small Tent
            lore = pgettext("hebrews_building", "'How good are your tents, O Jacob, your dwelling places, O Israel!'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew military site: Small Tent
            lore_author = pgettext("hebrews_building", "Book of Numbers"),
            -- TRANSLATORS: Purpose helptext for a Hebrew military site: Small Tent
            purpose = pgettext("hebrews_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for a Hebrew military site: Small Tent
            note = pgettext("hebrews_building", "If you're low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "hebrews_massada",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew military site: Massada
            lore = pgettext("hebrews_building", "'Massada shall not fall again!'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew military site: Massada
            lore_author = pgettext("hebrews_building", "Ancient Hebrew oath"),
            -- TRANSLATORS: Purpose helptext for a Hebrew military site: Massada
            purpose = pgettext("hebrews_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for a Hebrew military site: Massada
            note = pgettext("hebrews_building", "If you're low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },

      -- TODO: Seafaring/Ferry Sites (ship, shipyard, ferry_yard) not yet implemented

      -- Partially Finished Buildings - these are the same 2 buildings for all tribes
      {
         name = "constructionsite",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew building: Construction Site
            lore = pgettext("building", "'Don't swear at the builder who is short of building materials.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew building: Construction Site
            lore_author = pgettext("building", "Proverb widely used for impossible tasks of any kind"),
            -- TRANSLATORS: Purpose helptext for a Hebrew building: Construction Site
            purpose = pgettext("building", "A new building is being built at this construction site.")
         }
      },
      {
         name = "dismantlesite",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a Hebrew building: Dismantle Site
            lore = pgettext("building", "'New paths will appear when you are willing to tear down the old.'"),
            -- TRANSLATORS: Lore author helptext for a Hebrew building: Dismantle Site
            lore_author = pgettext("building", "Proverb"),
            -- TRANSLATORS: Purpose helptext for a Hebrew building: Dismantle Site
            purpose = pgettext("building", "A building is being dismantled at this dismantle site, returning some of the resources that were used during this building's construction to your tribe's stores.")
         }
      },
   },

   warehouse_names = {
      -- TRANSLATORS: This Hebrew warehouse is inspired by a biblical town name.
      pgettext("warehousename", "Tel Aviv"),
      -- TRANSLATORS: This Hebrew warehouse is inspired by a biblical town name.
      pgettext("warehousename", "Jerusalem"),
      -- TRANSLATORS: This Hebrew warehouse is inspired by a biblical town name.
      pgettext("warehousename", "Ashdod"),
      -- TRANSLATORS: This Hebrew warehouse is inspired by a biblical town name.
      pgettext("warehousename", "Ashkelon"),
      -- TRANSLATORS: This Hebrew warehouse is inspired by a biblical town name.
      pgettext("warehousename", "Beit El"),
      -- TRANSLATORS: This Hebrew warehouse is inspired by a biblical town name.
      pgettext("warehousename", "Betlehem"),
      -- TRANSLATORS: This Hebrew warehouse is inspired by a biblical town name.
      pgettext("warehousename", "Gaza"),
      -- TRANSLATORS: This Hebrew warehouse is inspired by a biblical town name.
      pgettext("warehousename", "Nob"),
      -- TRANSLATORS: This Hebrew warehouse is inspired by a biblical town name.
      pgettext("warehousename", "Bersheva"),
      -- TRANSLATORS: This Hebrew warehouse is inspired by a biblical town name.
      pgettext("warehousename", "Carmel"),
      -- TRANSLATORS: This Hebrew warehouse is inspired by a biblical town name.
      pgettext("warehousename", "Jaffa"),
      -- TRANSLATORS: This Hebrew warehouse is inspired by a biblical town name.
      pgettext("warehousename", "Jericho"),
      -- TRANSLATORS: This Hebrew warehouse is inspired by a biblical town name.
      pgettext("warehousename", "Timnah"),
      -- TRANSLATORS: This Hebrew warehouse is inspired by a biblical town name.
      pgettext("warehousename", "Ziklag"),
      -- TRANSLATORS: This Hebrew warehouse is inspired by a biblical town name.
      pgettext("warehousename", "Tyre"),
      -- TRANSLATORS: This Hebrew warehouse is inspired by a biblical town name.
      pgettext("warehousename", "Sidon"),
      -- TRANSLATORS: This Hebrew warehouse is inspired by a biblical town name.
      pgettext("warehousename", "Susa"),
      -- TRANSLATORS: This Hebrew warehouse is inspired by a biblical town name.
      pgettext("warehousename", "Hebron"),
      -- TRANSLATORS: This Hebrew warehouse is inspired by a biblical town name.
      pgettext("warehousename", "Ekron"),
      -- TRANSLATORS: This Hebrew warehouse is inspired by a biblical town name.
      pgettext("warehousename", "Babylon"),
      -- TRANSLATORS: This Hebrew warehouse is inspired by a biblical town name.
      pgettext("warehousename", "Nimrod"),
   },

   -- Productionsite status strings

   -- TRANSLATORS: Productivity label on a Hebrew building if there is 1 worker missing
   productionsite_worker_missing = pgettext("hebrews", "Worker missing"),
   -- TRANSLATORS: Productivity label on a Hebrew building if there is 1 worker coming
   productionsite_worker_coming = pgettext("hebrews", "Worker is coming"),
   -- TRANSLATORS: Productivity label on a Hebrew building if there is more than 1 worker missing. If you need plural forms here, please let us know.
   productionsite_workers_missing = pgettext("hebrews", "Workers missing"),
   -- TRANSLATORS: Productivity label on a Hebrew building if there is more than 1 worker coming. If you need plural forms here, please let us know.
   productionsite_workers_coming = pgettext("hebrews", "Workers are coming"),
   -- TRANSLATORS: Productivity label on a Hebrew building if there is 1 experienced worker missing
   productionsite_experienced_worker_missing = pgettext("hebrews", "Expert missing"),
   -- TRANSLATORS: Productivity label on a Hebrew building if there is more than 1 experienced worker missing. If you need plural forms here, please let us know.
   productionsite_experienced_workers_missing = pgettext("hebrews", "Experts missing"),

   -- Soldier strings to be used in Military Status strings

   soldier_context = "hebrews_soldier",
   soldier_0_sg = "%1% soldier (+%2%)",
   soldier_0_pl = "%1% soldiers (+%2%)",
   soldier_1_sg = "%1% soldier",
   soldier_1_pl = "%1% soldiers",
   soldier_2_sg = "%1%(+%2%) soldier (+%3%)",
   soldier_2_pl = "%1%(+%2%) soldiers (+%3%)",
   soldier_3_sg = "%1%(+%2%) soldier",
   soldier_3_pl = "%1%(+%2%) soldiers",
   -- TRANSLATORS: %1% is the number of Hebrew soldiers the plural refers to. %2% is the maximum number of soldier slots in the building.
   UNUSED_soldier_0 = npgettext("hebrews_soldier", "%1% soldier (+%2%)", "%1% soldiers (+%2%)", 0),
   -- TRANSLATORS: Number of Hebrew soldiers stationed at a militarysite.
   UNUSED_soldier_1 = npgettext("hebrews_soldier", "%1% soldier", "%1% soldiers", 0),
   -- TRANSLATORS: %1% is the number of Hebrew soldiers the plural refers to. %2% are currently open soldier slots in the building. %3% is the maximum number of soldier slots in the building
   UNUSED_soldier_2 = npgettext("hebrews_soldier", "%1%(+%2%) soldier (+%3%)", "%1%(+%2%) soldiers (+%3%)", 0),
   -- TRANSLATORS: %1% is the number of Hebrew soldiers the plural refers to. %2% are currently open soldier slots in the building.
   UNUSED_soldier_3 = npgettext("hebrews_soldier", "%1%(+%2%) soldier", "%1%(+%2%) soldiers", 0),

   -- Special types
   builder = "hebrews_builder",
   carriers = {"hebrews_carrier", "hebrews_donkey"},
   geologist = "hebrews_geologist",
   scouts_house = "hebrews_scouts_hut",
   soldier = "hebrews_soldier",
   ship = "barbarians_ship", -- TODO: replace with hebrews_ship once own ship exists
   ferry = "hebrews_ferry",
   port = "hebrews_port",

   fastplace = {
      warehouse = "hebrews_warehouse",
      port = "hebrews_port",
      training_small = "hebrews_trainingcamp",
      training_large = "hebrews_trainingcamp",
      military_small_primary = "hebrews_tent_small",
      military_fortress = "hebrews_massada",
      quarry = "hebrews_quarry",
      building_materials_primary = "hebrews_clay_pit",
      fisher = "hebrews_fishers_hut",
      well = "hebrews_well",
      farm_primary = "hebrews_farm",
      bakery = "hebrews_bakery",
      scout = "hebrews_scouts_hut",
      barracks = "hebrews_machane",
      second_carrier = "hebrews_donkeyfarm",
      mine_stone = "hebrews_granitemine",
      mine_iron = "hebrews_coppermine",
      mine_gold = "hebrews_goldmine",
   },
}

pop_textdomain()
