descriptions = wl.Descriptions() -- TODO(matthiakl): only for savegame compatibility with 1.0, do not use.

image_dirname = path.dirname(__file__) .. "images/"

push_textdomain("tribes_encyclopedia")

-- For formatting time strings
include "tribes/scripting/help/time_strings.lua"

-- For lore texts
-- TRANSLATORS: Format string for a latin quote and its translation
quote_with_translation_format = pgettext("empire_lore", "‘%1%’<br>(%2%)")
function latin_lore(latin, translation)
   return quote_with_translation_format:bformat(latin, translation)
end

wl.Descriptions():new_tribe {
   name = "empire",
   military_capacity_script = path.dirname(__file__) .. "military_capacity.lua",
   animation_directory = image_dirname,
   animations = {
      bridge_normal_e = { hotspot = {-2, 12} },
      bridge_busy_e = { hotspot = {-2, 12} },
      bridge_normal_se = { hotspot = {5, 2} },
      bridge_busy_se = { hotspot = {5, 2} },
      bridge_normal_sw = { hotspot = {36, 3} },
      bridge_busy_sw = { hotspot = {36, 3} }
   },
   spritesheets = {
      frontier = {
         frames = 1,
         columns = 1,
         rows = 1,
         hotspot = { 6, 19 }
      },
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
      { ware = "gold", points = 3},
      { ware = "spear_wooden", points = 1},
      { ware = "spear", points = 3},
      { ware = "spear_advanced", points = 4},
      { ware = "spear_heavy", points = 7},
      { ware = "spear_war", points = 8},
      { ware = "armor_helmet", points = 2},
      { ware = "armor", points = 3},
      { ware = "armor_chain", points = 4},
      { ware = "armor_gilded", points = 8},
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
               purpose = {
                  -- TRANSLATORS: Helptext for an empire ware: Granite, part 1
                  pgettext("ware", "Granite is a basic building material."),
                  -- TRANSLATORS: Helptext for an empire ware: Granite, part 2
                  pgettext("empire_ware", "The Empire produces granite blocks in quarries and marble mines.")
               }
            }
         },
         {
            name = "log",
            preciousness = 14,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an empire ware: Log, part 1
                  pgettext("ware", "Logs are an important basic building material. They are produced by felling trees."),
                  -- TRANSLATORS: Helptext for an empire ware: Log, part 2
                  pgettext("empire_ware", "Imperial lumberjacks fell the trees; foresters take care of the supply of trees. Logs are also used by the charcoal kiln, the toolsmithy and the sawmill.")
               }
            }
         },
         {
            name = "planks",
            default_target_quantity = 20,
            preciousness = 10,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an empire ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an empire ware: Planks, part 2
                  pgettext("empire_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an empire ware: Planks, part 3
                  pgettext("empire_ware", "They are also used by the weapon smithy.")
               }
            }
         },
         {
            name = "wool",
            default_target_quantity = 10,
            preciousness = 2,
            helptexts = {
               -- TRANSLATORS: Helptext for an empire ware: Wool
               purpose = pgettext("empire_ware", "Wool is the hair of sheep. Weaving mills use it to make cloth.")
            }
         },
         {
            name = "cloth",
            default_target_quantity = 15,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for an empire ware: Cloth
               purpose = pgettext("empire_ware", "Cloth is needed to build several buildings. It is also consumed in the armor smithy.")
            }
         }
      },
      {
         -- Food
         {
            name = "fish",
            preciousness = 3,
            helptexts = {
               -- TRANSLATORS: Helptext for an empire ware: Fish
               purpose = pgettext("empire_ware", "Besides bread and meat, fish is also a foodstuff for the Empire. Fish are used in taverns, inns and training sites (arena, colosseum, training camp).")
            }
         },
         {
            name = "meat",
            default_target_quantity = 20,
            preciousness = 2,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an empire ware: Meat, part 1
                  pgettext("ware", "Meat contains a lot of energy, and it is obtained from wild game taken by hunters."),
                  -- TRANSLATORS: Helptext for an empire ware: Meat, part 2
                  pgettext("empire_ware", "Meat can also be obtained as pork from piggeries. It is used in the inns and taverns to prepare lunch for the miners and is consumed at the training sites (arena, colosseum, training camp).")
               }
            }
         },
         {
            name = "water",
            preciousness = 7,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an empire ware: Water, part 1
                  pgettext("ware", "Water is the essence of life!"),
                  -- TRANSLATORS: Helptext for an empire ware: Water, part 2
                  pgettext("empire_ware", "Water is used in the bakery and the brewery. The donkey farm, the sheep farm and the piggery also need to be supplied with water.")
               }
            }
         },
         {
            name = "wheat",
            preciousness = 12,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an empire ware: Wheat, part 1
                  pgettext("ware", "Wheat is essential for survival."),
                  -- TRANSLATORS: Helptext for an empire ware: Wheat, part 2
                  pgettext("empire_ware", "Wheat is produced by farms and used by mills and breweries. Donkey farms, sheep farms and piggeries also need to be supplied with wheat.")
               }
            }
         },
         {
            name = "flour",
            default_target_quantity = 20,
            preciousness = 9,
            helptexts = {
               -- TRANSLATORS: Helptext for an empire ware: Flour
               purpose = pgettext("empire_ware", "Flour is produced by the mill out of wheat and is needed in the bakery to produce the tasty Empire bread.")
            }
         },
         {
            name = "empire_bread",
            default_target_quantity = 20,
            preciousness = 7,
            helptexts = {
               -- TRANSLATORS: Helptext for an empire ware: Bread
               purpose = pgettext("empire_ware", "The bakers of the Empire make really tasty bread out of flour and water. It is used in taverns and inns to prepare rations and meals. Bread is also consumed at the training sites (arena, colosseum, training camp).")
            }
         },
         {
            name = "beer",
            default_target_quantity = 15,
            preciousness = 5,
            helptexts = {
               -- TRANSLATORS: Helptext for an empire ware: Beer
               purpose = pgettext("empire_ware", "This beer is produced in a brewery out of wheat and water. It is consumed by miners in coal and iron mines.")
            }
         },
         {
            name = "grape",
            preciousness = 10,
            helptexts = {
               -- TRANSLATORS: Helptext for an empire ware: Grape
               purpose = pgettext("empire_ware", "These grapes are the base for a tasty wine. They are harvested in a vineyard and processed in a winery.")
            }
         },
         {
            name = "wine",
            preciousness = 8,
            helptexts = {
               -- TRANSLATORS: Helptext for an empire ware: Wine
               purpose = pgettext("empire_ware", "This tasty wine is drunk by the miners working the marble and gold mines. It is produced in a winery.")
            }
         },
         {
            name = "ration",
            default_target_quantity = 20,
            preciousness = 4,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an empire ware: Ration, part 1
                  pgettext("ware", "A small bite to keep miners strong and working. The scout also consumes rations on his scouting trips."),
                  -- TRANSLATORS: Helptext for an empire ware: Ration, part 2
                  pgettext("empire_ware", "Rations are produced in a tavern out of fish or meat or bread.")
               }
            }
         },
         {
            name = "meal",
            default_target_quantity = 20,
            preciousness = 4,
            helptexts = {
               -- TRANSLATORS: Helptext for an empire ware: Meal
               purpose = pgettext("empire_ware", "A real meal is made in inns out of bread and fish/meat. It satisfies the needs of miners in deep mines.")
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
               -- TRANSLATORS: Helptext for an empire ware: Marble
               purpose = pgettext("empire_ware", "Marble is the beautiful stone which is cut out of the mountains or produced in a quarry. Marble is used as a building material and gets chiseled into marble columns in the stonemason’s house.")
            }
         },
         {
            name = "marble_column",
            default_target_quantity = 10,
            preciousness = 5,
            helptexts = {
               -- TRANSLATORS: Helptext for an empire ware: Marble Column
               purpose = pgettext("empire_ware", "Marble columns represent the high culture of the Empire, so they are needed for nearly every larger building. They are produced out of marble at a stonemason’s house.")
            }
         },
         {
            name = "coal",
            default_target_quantity = 20,
            preciousness = 10,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an empire ware: Coal, part 1
                  pgettext("ware", "Coal is mined in coal mines or produced out of logs by a charcoal kiln."),
                  -- TRANSLATORS: Helptext for an empire ware: Coal, part 2
                  pgettext("empire_ware", "The fires of the Empire smelting works, armor smithies and weapon smithies are usually fed with coal.")
               }
            }
         },
         {
            name = "iron_ore",
            default_target_quantity = 15,
            preciousness = 4,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an empire ware: Iron Ore, part 1
                  pgettext("default_ware", "Iron ore is mined in iron mines."),
                  -- TRANSLATORS: Helptext for an empire ware: Iron Ore, part 2
                  pgettext("empire_ware", "It is smelted in a smelting works to retrieve the iron.")
               }
            }
         },
         {
            name = "iron",
            default_target_quantity = 20,
            preciousness = 4,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an empire ware: Iron, part 1
                  pgettext("ware", "Iron is smelted out of iron ores."),
                  -- TRANSLATORS: Helptext for an empire ware: Iron, part 2
                  pgettext("empire_ware", "It is produced by the smelting works. Weapons, armor and tools are made of iron.")
               }
            }
         },
         {
            name = "gold_ore",
            default_target_quantity = 15,
            preciousness = 2,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an empire ware: Gold Ore, part 1
                  pgettext("ware", "Gold ore is mined in a gold mine."),
                  -- TRANSLATORS: Helptext for an empire ware: Gold Ore, part 2
                  pgettext("empire_ware", "Smelted in a smelting works, it turns into gold which is used as a precious building material and to produce weapons and armor.")
               }
            }
         },
         {
            name = "gold",
            default_target_quantity = 20,
            preciousness = 2,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an empire ware: Gold, part 1
                  pgettext("ware", "Gold is the most valuable of all metals, and it is smelted out of gold ore."),
                  -- TRANSLATORS: Helptext for an empire ware: Gold, part 2
                  pgettext("empire_ware", "It is produced by the smelting works. Armor and weapons are embellished with gold in the armor smithy and the weapon smithy.")
               }
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
               -- TRANSLATORS: Helptext for an empire ware: Pick
               purpose = pgettext("empire_ware", "Picks are used by stonemasons and miners. They are produced by the toolsmith.")
            }
         },
         {
            name = "felling_ax",
            default_target_quantity = 3,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an empire ware: Felling Ax, part 1
                  pgettext("ware", "The felling ax is the tool to chop down trees."),
                  -- TRANSLATORS: Helptext for an empire ware: Felling Ax, part 2
                  pgettext("empire_ware", "Felling axes are used by lumberjacks and produced by the toolsmithy.")
               }
            }
         },
         {
            name = "saw",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for an empire ware: Saw
               purpose = pgettext("empire_ware", "This saw is needed by the carpenter. It is produced by the toolsmith.")
            }
         },
         {
            name = "shovel",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an empire ware: Shovel, part 1
                  pgettext("ware", "Shovels are needed for the proper handling of plants."),
                  -- TRANSLATORS: Helptext for an empire ware: Shovel, part 2
                  pgettext("empire_ware", "Therefore the forester and the vine farmer use them. They are produced by the toolsmith.")
               }
            }
         },
         {
            name = "hammer",
            default_target_quantity = 2,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an empire ware: Hammer, part 1
                  pgettext("ware", "The hammer is an essential tool."),
                  -- TRANSLATORS: Helptext for an empire ware: Hammer, part 2
                  pgettext("empire_ware", "Geologists, builders, weaponsmiths and armorsmiths all need a hammer. Make sure you’ve always got some in reserve! They are produced by the toolsmith.")
               }
            }
         },
         {
            name = "fishing_rod",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an empire ware: Fishing Rod, part 1
                  pgettext("ware", "Fishing rods are needed by fishers to catch fish."),
                  -- TRANSLATORS: Helptext for an empire ware: Fishing Rod, part 2
                  pgettext("empire_ware", "They are produced by the toolsmith.")
               }
            }
         },
         {
            name = "hunting_spear",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an empire ware: Hunting Spear, part 1
                  pgettext("ware", "This spear is light enough to be thrown, but heavy enough to kill any animal in one blow. It is only used by hunters."),
                  -- TRANSLATORS: Helptext for an empire ware: Hunting Spear, part 2
                  pgettext("empire_ware", "They are produced by the toolsmith.")
               }
            }
         },
         {
            name = "scythe",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an empire ware: Scythe, part 1
                  pgettext("ware", "The scythe is the tool of the farmers."),
                  -- TRANSLATORS: Helptext for an empire ware: Scythe, part 2
                  pgettext("empire_ware", "Scythes are produced by the toolsmith.")
               }
            }
         },
         {
            name = "bread_paddle",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an empire ware: Bread Paddle, part 1
                  pgettext("ware", "The bread paddle is the tool of the baker, each baker needs one."),
                  -- TRANSLATORS: Helptext for an empire ware: Bread Paddle, part 2
                  pgettext("empire_ware", "Bread paddles are produced by the toolsmith.")
               }
            }
         },
         {
            name = "basket",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for an empire ware: Basket
               purpose = pgettext("empire_ware", "This basket is needed by the vine farmer for harvesting the grapes. It is produced by the toolsmith.")
            }
         },
         {
            name = "kitchen_tools",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for an empire ware: Kitchen Tools
               purpose = pgettext("empire_ware", "Kitchen tools are needed for preparing rations and meals. They are produced in a toolsmithy and used in taverns and inns.")
            }
         },
         {
            name = "fire_tongs",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an empire ware: Fire Tongs, part 1
                  pgettext("ware", "Fire tongs are the tools for smelting ores."),
                  -- TRANSLATORS: Helptext for an empire ware: Fire Tongs, part 2
                  pgettext("empire_ware", "They are used in the smelting works and produced by the toolsmith.")
               }
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
               -- TRANSLATORS: Helptext for an empire ware: Wooden Spear
               purpose = pgettext("empire_ware", "This wooden spear is the basic weapon in the Empire military system. It is produced in the weapon smithy. In combination with a helmet, it is the equipment to fit out young soldiers.")
            }
         },
         {
            name = "spear",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for an empire ware: Spear
               purpose = pgettext("empire_ware", "This spear has an iron spike. It is produced in a weapon smithy and used in a training camp – together with food – to train soldiers from attack level 0 to attack level 1.")
            }
         },
         {
            name = "spear_advanced",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for an empire ware: Advanced Spear
               purpose = pgettext("empire_ware", "This is an advanced spear with a steel tip. It is produced in a weapon smithy and used in a training camp – together with food – to train soldiers from attack level 1 to level 2.")
            }
         },
         {
            name = "spear_heavy",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for an empire ware: Heavy Spear
               purpose = pgettext("empire_ware", "This is a strong spear with a steel-tip and a little blade. It is produced in the weapon smithy and used in the training camp – together with food – train soldiers from attack level 2 to level 3.")
            }
         },
         {
            name = "spear_war",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for an empire ware: War Spear
               purpose = pgettext("empire_ware", "This is the best and sharpest weapon the Empire weaponsmith creates for the soldiers. It is used in the training camp – together with food – to train soldiers from attack level 3 to level 4.")
            }
         },
         {
            name = "armor_helmet",
            default_target_quantity = 30,
            preciousness = 2,
            helptexts = {
               -- TRANSLATORS: Helptext for an empire ware: Helmet
               purpose = pgettext("empire_ware", "The helmet is the basic defense of a warrior. It is produced in an armor smithy. In combination with a wooden spear, it is the equipment to fit out young soldiers. Helmets are also used in training camps – together with food – to train soldiers from health level 0 to level 1.")
            }
         },
         {
            name = "armor",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for an empire ware: Armor
               purpose = pgettext("empire_ware", "Basic armor for Empire soldiers. It is produced in the armor smithy and used in the training camp – together with food – to train soldiers from health level 1 to level 2.")
            }
         },
         {
            name = "armor_chain",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for an empire ware: Chain Armor
               purpose = pgettext("empire_ware", "The chain armor is a medium armor for Empire soldiers. It is produced in an armor smithy and used in a training camp – together with food – to train soldiers from health level 2 to level 3.")
            }
         },
         {
            name = "armor_gilded",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for an empire ware: Gilded Armor
               purpose = pgettext("empire_ware", "The gilded armor is the strongest armor an Empire soldier can have. It is produced in the armor smithy and used in the training camp – together with food – to train soldiers from health level 3 to level 4.")
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
               -- TRANSLATORS: Helptext for an empire worker: Carrier
               purpose = pgettext("empire_worker", "Carries items along your roads.")
            }
         },
         {
            name = "empire_ferry",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Ferry
               purpose = pgettext("empire_worker", "Ships wares across narrow rivers.")
            }
         },
         {
            name = "empire_donkey",
            default_target_quantity = 10,
            preciousness = 2,
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Donkey
               purpose = pgettext("empire_worker", "Donkeys help to carry items along busy roads. They are reared in a donkey farm.")
            }
         },
         {
            name = "empire_donkeybreeder",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Donkey Breeder
               purpose = pgettext("empire_worker", "Breeds cute and helpful donkeys for adding them to the transportation system.")
            }
         }
      },
      {
         -- Building Materials
         {
            name = "empire_stonemason",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Stonemason
               purpose = pgettext("empire_worker", "Cuts blocks of granite and marble out of rocks in the vicinity.")
            }
         },
         {
            name = "empire_carpenter",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Carpenter
               purpose = pgettext("empire_worker", "Works in the sawmill.")
            }
         },
         {
            name = "empire_lumberjack",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Lumberjack
               purpose = pgettext("empire_worker", "Fells trees.")
            }
         },
         {
            name = "empire_forester",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Forester
               purpose = pgettext("empire_worker", "Plants trees.")
            }
         },
         {
            name = "empire_builder",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Builder
               purpose = pgettext("empire_worker", "Works at construction sites to raise new buildings.")
            }
         },
         {
            name = "empire_shepherd",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Shepherd
               purpose = pgettext("empire_worker", "Keeping sheep.")
            }
         },
         {
            name = "empire_weaver",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Weaver
               purpose = pgettext("empire_worker", "Produces cloth for ships and soldiers.")
            }
         },
         {
            name = "empire_shipwright",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Shipwright
               purpose = pgettext("empire_worker", "Works at the shipyard and constructs new ships.")
            }
         }
      },
      {
         -- Food
         {
            name = "empire_fisher",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Fisher
               purpose = pgettext("empire_worker", "Catches fish in the sea.")
            }
         },
         {
            name = "empire_hunter",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Hunter
               purpose = pgettext("empire_worker", "The hunter brings fresh, raw meat to the colonists.")
            }
         },
         {
            name = "empire_farmer",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Farmer
               purpose = pgettext("empire_worker", "Plants fields.")
            }
         },
         {
            name = "empire_miller",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Miller
               purpose = pgettext("empire_worker", "Grinds wheat to produce flour.")
            }
         },
         {
            name = "empire_baker",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Baker
               purpose = pgettext("empire_worker", "Bakes bread for workers.")
            }
         },
         {
            name = "empire_brewer",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Brewer
               purpose = pgettext("empire_worker", "Produces beer.")
            }
         },
         {
            name = "empire_vinefarmer",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Vine Farmer
               purpose = pgettext("empire_worker", "Grows grapevines.")
            }
         },
         {
            name = "empire_vintner",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Vintner
               purpose = pgettext("empire_worker", "Produces wine.")
            }
         },
         {
            name = "empire_pigbreeder",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Pig Breeder
               purpose = pgettext("empire_worker", "Breeds pigs.")
            }
         },
         {
            name = "empire_innkeeper",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Innkeeper
               purpose = pgettext("empire_worker", "Produces food for miners.")
            }
         }
      },
      {
         -- Mining
         {
            name = "empire_geologist",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Geologist
               purpose = pgettext("empire_worker", "Discovers resources for mining.")
            }
         },
         {
            name = "empire_miner",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Miner
               purpose = pgettext("empire_worker", "Works deep in the mines to obtain coal, iron, gold or marble.")
            }
         },
         {
            name = "empire_miner_master",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Master Miner
               purpose = pgettext("empire_worker", "Works deep in the mines to obtain coal, iron, gold or marble.")
            }
         },
         {
            name = "empire_charcoal_burner",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Charcoal Burner
               purpose = pgettext("empire_worker", "Burns coal.")
            }
         },
         {
            name = "empire_smelter",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Smelter
               purpose = pgettext("empire_worker", "Smelts ores into metal.")
            }
         }
      },
      {
         -- Tools
         {
            name = "empire_toolsmith",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Toolsmith
               purpose = pgettext("empire_worker", "Produces tools for the workers.")
            }
         }
      },
      {
         -- Military
         {
            name = "empire_recruit",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Recruit
               purpose = pgettext("empire_worker", "Eager to become a soldier and defend his tribe!")
            }
         },
         {
            name = "empire_soldier",
            default_target_quantity = 10,
            preciousness = 5,
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Soldier
               purpose = pgettext("empire_worker", "Defend and Conquer!")
            }
         },
         {
            name = "empire_trainer",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Trainer
               purpose = pgettext("empire_worker", "Trains the soldiers.")
            }
         },
         {
            name = "empire_weaponsmith",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Weaponsmith
               purpose = pgettext("empire_worker", "Produces weapons for the soldiers.")
            }
         },
         {
            name = "empire_armorsmith",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Armorsmith
               purpose = pgettext("empire_worker", "Produces armor for the soldiers.")
            }
         },
         {
            name = "empire_scout",
            helptexts = {
               -- TRANSLATORS: Helptext for an empire worker: Scout
               purpose = pgettext("empire_worker", "Scouts like Scotty the scout scouting unscouted areas in a scouty fashion.")
               -- (c) WiHack Team 02.01.2010
            }
         }
      }
   },

   immovables = {
      {
         name = "ashes",
         helptexts = {
            -- TRANSLATORS: Helptext for an empire immovable: Ashes
            purpose = _("The remains of a destroyed building.")
         }
      },
      {
         name = "destroyed_building",
         helptexts = {
            -- TRANSLATORS: Helptext for an empire immovable: Destroyed Building
            purpose = _("The remains of a destroyed building.")
         }
      },
      {
         name = "wheatfield_tiny",
         helptexts = {
            -- TRANSLATORS: Helptext for an empire immovable: Wheat field
            purpose = _("This field has just been planted.")
         }
      },
      {
         name = "wheatfield_small",
         helptexts = {
            -- TRANSLATORS: Helptext for an empire immovable: Wheat field
            purpose = _("This field is growing.")
         }
      },
      {
         name = "wheatfield_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for an empire immovable: Wheat field
            purpose = _("This field is growing.")
         }
      },
      {
         name = "wheatfield_ripe",
         helptexts = {
            -- TRANSLATORS: Helptext for an empire immovable: Wheat field
            purpose = _("This field is ready for harvesting.")
         }
      },
      {
         name = "wheatfield_harvested",
         helptexts = {
            -- TRANSLATORS: Helptext for an empire immovable: Wheat field
            purpose = _("This field has been harvested.")
         }
      },
      {
         name = "grapevine_tiny",
         helptexts = {
            -- TRANSLATORS: Helptext for an empire immovable: Grapevine
            purpose = _("This grapevine has just been planted.")
         }
      },
      {
         name = "grapevine_small",
         helptexts = {
            -- TRANSLATORS: Helptext for an empire immovable: Grapevine
            purpose = _("This grapevine is growing.")
         }
      },
      {
         name = "grapevine_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for an empire immovable: Grapevine
            purpose = _("This grapevine is growing.")
         }
      },
      {
         name = "grapevine_ripe",
         helptexts = {
            -- TRANSLATORS: Helptext for an empire immovable: Grapevine
            purpose = _("This grapevine is ready for harvesting.")
         }
      },
      {
         name = "empire_resi_none",
         helptexts = {
            -- TRANSLATORS: Helptext for an empire resource indicator: No resources
            purpose = _("There are no resources in the ground here.")
         }
      },
      {
         name = "empire_resi_water",
         helptexts = {
            -- TRANSLATORS: Helptext for an empire resource indicator: Water
            purpose = _("There is water in the ground here that can be pulled up by a well.")
         }
      },
      {
         name = "empire_resi_coal_1",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an empire resource indicator: Coal, part 1
               _("Coal veins contain coal that can be dug up by coal mines."),
               -- TRANSLATORS: Helptext for an empire resource indicator: Coal, part 2
               _("There is only a little bit of coal here.")
            }
         }
      },
      {
         name = "empire_resi_iron_1",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an empire resource indicator: Iron, part 1
               _("Iron veins contain iron ore that can be dug up by iron mines."),
               -- TRANSLATORS: Helptext for an empire resource indicator: Iron, part 2
               _("There is only a little bit of iron here.")
            }
         }
      },
      {
         name = "empire_resi_gold_1",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an empire resource indicator: Gold, part 1
               _("Gold veins contain gold ore that can be dug up by gold mines."),
               -- TRANSLATORS: Helptext for an empire resource indicator: Gold, part 2
               _("There is only a little bit of gold here.")
            }
         }
      },
      {
         name = "empire_resi_stones_1",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an Empire resource indicator: Stones, part 1
               _("Marble is a basic building material and can be dug up by a marble mine. You will also get granite from the mine."),
               -- TRANSLATORS: Helptext for an Empire resource indicator: Stones, part 2
               _("There is only a little bit of marble here.")
            }
         }
      },
      {
         name = "empire_resi_coal_2",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an empire resource indicator: Coal, part 1
               _("Coal veins contain coal that can be dug up by coal mines."),
               -- TRANSLATORS: Helptext for an empire resource indicator: Coal, part 2
               _("There is a lot of coal here.")
            }
         }
      },
      {
         name = "empire_resi_iron_2",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an empire resource indicator: Iron, part 1
               _("Iron veins contain iron ore that can be dug up by iron mines."),
               -- TRANSLATORS: Helptext for an empire resource indicator: Iron, part 2
               _("There is a lot of iron here.")
            }
         }
      },
      {
         name = "empire_resi_gold_2",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an empire resource indicator: Gold, part 1
               _("Gold veins contain gold ore that can be dug up by gold mines."),
               -- TRANSLATORS: Helptext for an empire resource indicator: Gold, part 2
               _("There is a lot of gold here.")
            }
         }
      },
      {
         name = "empire_resi_stones_2",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an Empire resource indicator: Stones, part 1
               _("Marble is a basic building material and can be dug up by a marble mine. You will also get granite from the mine."),
               -- TRANSLATORS: Helptext for an Empire resource indicator: Stones, part 2
               _("There is a lot of marble here.")
            }
         }
      },
      {
         name = "empire_shipconstruction",
         helptexts = {
            -- TRANSLATORS: Helptext for an empire immovable: Ship Under Construction
            purpose = _("A ship is being constructed at this site.")
         }
      },
      -- non imperial Immovables used by the woodcutter
      {
         name = "deadtree7",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by empire: Dead Tree
            purpose = _("The remains of an old tree.")
         }
      },
      {
         name = "balsa_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by empire: Balsa Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_black_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by empire: Balsa Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_desert_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by empire: Balsa Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_winter_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by empire: Balsa Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by empire: Ironwood Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_black_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by empire: Ironwood Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_desert_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by empire: Ironwood Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_winter_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by empire: Ironwood Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by empire: Rubber Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_black_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by empire: Rubber Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_desert_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by empire: Corn Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_winter_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by empire: Rubber Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
   },

   -- The order here also determines the order in lists on screen.
   buildings = {
      -- Warehouses
      {
         name = "empire_headquarters",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Adaptation of a classical Latin quote meaning:
               --    "All roads lead to Fremil."
               _("Omnes viæ ducunt Fremilem."),
               -- TRANSLATORS: Lore helptext for an empire warehouse: Headquarters
               --    Translation for the adapted classical Latin quote:
               --    "Omnes viæ ducunt Fremilem."
               _("All roads lead to Fremil.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire warehouse: Headquarters
            lore_author = pgettext("empire_building",
               "Proverb signifying the glory of the Empire and its capital"
            ),
            -- TRANSLATORS: Purpose helptext for an empire warehouse: Headquarters
            purpose = pgettext("empire_building", "Accommodation for your people. Also stores your wares and tools."),
            -- TRANSLATORS: Note helptext for an empire warehouse: Headquarters
            note = pgettext("empire_building", "The headquarters is your main building.")
         }
      },
      {
         name = "empire_headquarters_shipwreck",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "A shipwreck that is common to all is a consolation."
               _("Commune naufragium omnibus est consolatio."),
               -- TRANSLATORS: Lore helptext for an empire warehouse: Headquarters Shipwreck
               --    Translation for the classical Latin quote:
               --    "Commune naufragium omnibus est consolatio."
               _("A shipwreck that is common to all is a consolation.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire warehouse: Headquarters Shipwreck
            lore_author = pgettext("empire_building", "Lutius after stranding on an uncharted island"),
            -- TRANSLATORS: Purpose helptext for an empire warehouse: Headquarters Shipwreck
            purpose = pgettext("empire_building", "Although this ship ran aground, it still serves as accommodation for your people. It also stores your wares and tools."),
            -- TRANSLATORS: Note helptext for an empire warehouse: Headquarters Shipwreck
            note = pgettext("empire_building", "The headquarters shipwreck is your main building.")
         }
      },
      {
         name = "empire_warehouse",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "Who gives promptly, gives twice, who gives late, gives nothing."
               _("Bis dat, qui cito dat, nil dat, qui munera tardat."),
               -- TRANSLATORS: Lore helptext for an empire warehouse: Warehouse
               --    Translation for the classical Latin quote:
               --    "Bis dat, qui cito dat, nil dat, qui munera tardat."
               _("Who gives promptly, gives twice, who gives late, gives nothing.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire warehouse: Warehouse
            lore_author = pgettext("empire_building", "Proverb written over the doors of a warehouse"),
            -- TRANSLATORS: Purpose helptext for an empire warehouse: Warehouse
            purpose = pgettext("empire_building", "Your workers and soldiers will find shelter here. Also stores your wares and tools.")
         }
      },
      {
         name = "empire_port",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "To sail is necessary."
               _("Navigare necesse est."),
               -- TRANSLATORS: Lore helptext for an empire warehouse: Port
               --    Translation for the classical Latin quote:
               --    "Navigare necesse est."
               _("To sail is necessary.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire warehouse: Port
            lore_author = pgettext("empire_building",
               "Admiral Litus Marius Exsiccatus, sending out the fleet to break the naval blockade "..
               "during the First Atlantean War"
            ),
            -- TRANSLATORS: Purpose helptext for an empire warehouse: Port
            purpose = pgettext("empire_building", "Serves as a base for overseas colonization and trade. Also stores your soldiers, wares and tools."),
            -- TRANSLATORS: Note helptext for an empire warehouse: Port
            note = pgettext("empire_building", "Similar to the Headquarters a Port can be attacked and destroyed by an enemy. It is recommendable to send soldiers to defend it.")
         }
      },

      -- Small
      {
         name = "empire_quarry",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "There shall not be left here a stone upon a stone that shall not be destroyed."
               _("Non relinquetur hic lapis super lapidem qui non destruatur."),
               -- TRANSLATORS: Lore helptext for an empire production site: Quarry
               --    Translation for the classical Latin quote:
               --    "Non relinquetur hic lapis super lapidem qui non destruatur."
               _("There shall not be left here a stone upon a stone that shall not be destroyed.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Quarry
            lore_author = pgettext("empire_building", "An enthusiastic stonemason arriving to a new quarry"),
            -- TRANSLATORS: Purpose helptext for an empire production site: Quarry
            purpose = pgettext("empire_building", "Cuts blocks of granite and marble out of rocks in the vicinity."),
            -- TRANSLATORS: Note helptext for an empire production site: Quarry
            note = pgettext("empire_building", "The quarry needs rocks to cut within the work area.")
         }
      },
      {
         name = "empire_lumberjacks_house",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "Don’t carry logs to the forest."
               _("In silvam non ligna feras."),
               -- TRANSLATORS: Lore helptext for an empire production site: Lumberjack's House
               --    Translation for the classical Latin quote:
               --    "In silvam non ligna feras."
               _("Don’t carry logs to the forest.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Lumberjack's House
            lore_author = pgettext("empire_building", "A carrier to a lumberjack who lost his way home"),
            -- TRANSLATORS: Purpose helptext for an empire production site: Lumberjack's House
            purpose = pgettext("building", "Fells trees in the surrounding area and processes them into logs."),
            -- TRANSLATORS: Note helptext for an empire production site: Lumberjack's House
            note = pgettext("empire_building", "The lumberjack’s house needs trees to fell within the work area.")
         }
      },
      {
         name = "empire_foresters_house",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "Live, grow, and bloom!"
               _("Vivat, crescat, floreat!"),
               -- TRANSLATORS: Lore helptext for an empire production site: Forester's House
               --    Translation for the classical Latin quote:
               --    "Vivat, crescat, floreat!"
               _("Live, grow, and bloom!")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Forester's House
            lore_author = pgettext("empire_building", "Blessing from the god Silvanus"),
            -- TRANSLATORS: Purpose helptext for an empire production site: Forester's House
            purpose = pgettext("building", "Plants trees in the surrounding area."),
            -- TRANSLATORS: Note helptext for an empire production site: Forester's House
            note = pgettext("empire_building", "The forester’s house needs free space within the work area to plant the trees.")
         }
      },
      {
         name = "empire_fishers_house",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "The fish stinks first at the head."
               _("Piscis primum a capite foetet."),
               -- TRANSLATORS: Lore helptext for an empire production site:  Fisher's House
               --    Translation for the classical Latin quote:
               --    "Piscis primum a capite foetet."
               _("The fish stinks first at the head.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Fisher's House
            lore_author = pgettext("empire_building", "A fisherman criticising the Grand Master of the Fishermans’ Guild"),
            -- TRANSLATORS: Purpose helptext for an empire production site: Fisher's House
            purpose = pgettext("empire_building", "Fishes on the coast near the fisher’s house."),
            -- TRANSLATORS: Note helptext for an empire production site: Fisher's House
            note = pgettext("empire_building", "The fisher’s house needs water full of fish within the work area.")
         }
      },
      {
         name = "empire_hunters_house",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Latin quote meaning:
               --    "The hunter lies in ambush for the game."
               _("Venator venatus insidiatur."),
               -- TRANSLATORS: Lore helptext for an empire production site: Hunter's House
               --    Translation for the Latin quote:
               --    "Venator venatus insidiatur."
               _("The hunter lies in ambush for the game.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Hunter's House
            lore_author = pgettext("empire_building", "Caption on an imperial relief showing a hunter"),
            -- TRANSLATORS: Purpose helptext for an empire production site: Hunter's House
            purpose = pgettext("building", "Hunts animals to produce meat."),
            -- TRANSLATORS: Note helptext for an empire production site: Hunter's House
            note = pgettext("empire_building", "The hunter’s house needs animals to hunt within the work area.")
         }
      },
      {
         name = "empire_well",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "It is more pleasing to drink water from the spring itself."
               _("Gratius ex ipso fonte bibuntur aquæ."),
               -- TRANSLATORS: Lore helptext for an empire production site: Well
               --    Translation for the classical Latin quote:
               --    "Gratius ex ipso fonte bibuntur aquæ."
               _("It is more pleasing to drink water from the spring itself.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Well
            lore_author = pgettext("empire_building", "Imperial proverb"),
            -- TRANSLATORS: Purpose helptext for an empire production site: Well
            purpose = pgettext("building", "Draws water out of the deep.")
         }
      },
      {
         name = "empire_scouts_house",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Adaptation of a classical Latin quote meaning:
               --    "Chat’Karuth at the gates!"
               _("Ciatkaruthus ante portas!"),
               -- TRANSLATORS: Lore helptext for an empire production site: Scout's House
               --    Translation for the adapted classical Latin quote:
               --    "Ciatkaruthus ante portas!"
               _("Chat’Karuth at the gates!")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Scout's House
            lore_author = pgettext("empire_building", "A belated scout’s report"),
            no_scouting_building_connected = pgettext("empire_building", "You need to connect this flag to a scout’s house before you can send a scout here."),
            -- TRANSLATORS: Purpose helptext for an empire production site: Scout's House
            purpose = pgettext("building", "Explores unknown territory.")
         }
      },

      -- Medium
      {
         name = "empire_stonemasons_house",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "The mind moves boulders."
               _("Mens agitat molem."),
               -- TRANSLATORS: Lore helptext for an empire production site: Stonemason's House
               --    Translation for the classical Latin quote:
               --    "Mens agitat molem."
               _("The mind moves boulders.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Stonemason's House
            lore_author = pgettext("empire_building", "Motto of the Stonemasons’ Guild"),
            -- TRANSLATORS: Purpose helptext for an empire production site: Stonemason's House
            purpose = pgettext("empire_building", "Carves marble columns out of marble.")
         }
      },
      {
         name = "empire_sawmill",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "The saw does nothing except when moved by art."
               _("Nihil agit serra nisi mota ab arte."),
               -- TRANSLATORS: Lore helptext for an empire production site: Sawmill
               --    Translation for the classical Latin quote:
               --    "Nihil agit serra nisi mota ab arte."
               _("The saw does nothing except when moved by art.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Sawmill
            lore_author = pgettext("empire_building", "Motto of the Carpenters’ Guild"),
            -- TRANSLATORS: Purpose helptext for an empire production site: Sawmill
            purpose = pgettext("building", "Saws logs to produce planks.")
         }
      },
      {
         name = "empire_mill",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "The mills of the gods grind slowly."
               _("Sero molunt deorum molæ."),
               -- TRANSLATORS: Lore helptext for an empire production site: Mill
               --    Translation for the classical Latin quote:
               --    "Sero molunt deorum molæ."
               _("The mills of the gods grind slowly.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Mill
            lore_author = pgettext("empire_building", "Proverb often cited by millers to impatient customers"),
            -- TRANSLATORS: Purpose helptext for an empire production site: Mill
            purpose = pgettext("building", "Grinds wheat to produce flour."),
            -- TRANSLATORS: Performance helptext for an empire production site: Mill
            performance = pgettext("empire_building", "The miller needs %1% on average to grind wheat into a sack of flour."):bformat(format_seconds(19))
         }
      },
      {
         name = "empire_bakery",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "There’s not this much flour in your sack."
               _("Non est de sacco tanta farina tuo."),
               -- TRANSLATORS: Lore helptext for an empire production site: Bakery
               --    Translation for the classical Latin quote:
               --    "Non est de sacco tanta farina tuo."
               _("There’s not this much flour in your sack.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Bakery
            lore_author = pgettext("empire_building", "A baker accusing another of theft"),
            -- TRANSLATORS: Purpose helptext for an empire production site: Bakery
            purpose = pgettext("empire_building", "Bakes bread for soldiers and miners alike."),
            -- TRANSLATORS: Performance helptext for an empire production site: Bakery
            performance = pgettext("empire_building", "The baker needs %1% on average to bake a loaf of bread."):bformat(format_seconds(34))
         }
      },
      {
         name = "empire_brewery",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "Now it’s time to drink! Now it’s time to dance freely!"
               _("Nunc est bibendum, nunc pede libero pulsanda tellus!"),
               -- TRANSLATORS: Lore helptext for an empire production site: Empire Brewery.
               --    Translation for the classical Latin quote:
               --    "Nunc est bibendum, nunc pede libero pulsanda tellus!"
               --    Literal translation of the second half is "stomp the earth with free feet"
               _("Now it’s time to drink! Now it’s time to dance freely!")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Brewery
            lore_author = pgettext("empire_building", "Imperial drinking song"),
            -- TRANSLATORS: Purpose helptext for an empire production site: Brewery
            purpose = pgettext("building", "Produces beer to keep the miners strong and happy."),
            -- TRANSLATORS: Performance helptext for an empire production site: Brewery
            performance = pgettext("empire_building", "The brewer needs %1% on average to brew a vat of beer."):bformat(format_minutes_seconds(1, 5))
         }
      },
      {
         name = "empire_vineyard",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "Good wine needs no bush." (meaning no advertising)
               _("Vino vendibili hedera non opus est."),
               -- TRANSLATORS: Lore helptext for an empire production site: Vineyard
               --    Translation for the classical Latin quote:
               --    "Vino vendibili hedera non opus est."
               --    Bush (ivy in the original) refers to the ivy leaves used for advertising
               --    by places selling wine
               _("Good wine needs no bush.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Vineyard
            lore_author = pgettext("empire_building", "A proud vine farmer boasting"),
            -- TRANSLATORS: Purpose helptext for an empire production site: Vineyard
            purpose = pgettext("empire_building", "Plants grapevines and harvests grapes.")
         }
      },
      {
         name = "empire_winery",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "In wine there is truth."
               _("In vino veritas."),
               -- TRANSLATORS: Lore helptext for an empire production site: Winery
               --    Translation for the classical Latin quote:
               --    "In vino veritas."
               _("In wine there is truth.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Winery
            lore_author = pgettext("empire_building", "Excerpt from the Encyclopædia Imperica"),
            -- TRANSLATORS: Purpose helptext for an empire production site: Winery
            purpose = pgettext("empire_building", "Produces wine.")
         }
      },
      {
         name = "empire_tavern",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "We eat to live, not live to eat."
               _("Esse oportet ut vivas, non vivere ut edas."),
               -- TRANSLATORS: Lore helptext for an empire production site: Tavern
               --    Translation for the classical Latin quote:
               --    "Esse oportet ut vivas, non vivere ut edas."
               _("We eat to live, not live to eat.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Tavern
            lore_author = pgettext("empire_building", "Warning to a gluttonous miner"),
            -- TRANSLATORS: Purpose helptext for an empire production site: Tavern
            purpose = pgettext("building", "Prepares rations to feed the scouts and miners."),
            -- TRANSLATORS: Performance helptext for an empire production site: Tavern
            performance = pgettext("empire_building", "The innkeeper needs %1% on average to prepare a ration."):bformat(format_seconds(36))
         }
      },
      {
         name = "empire_inn",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "A meal full of dishes restrains your mind."
               _("In cena dapibus plena te mente refrena."),
               -- TRANSLATORS: Lore helptext for an empire production site: Inn
               --    Translation for the classical Latin quote:
               --    "In cena dapibus plena te mente refrena."
               _("A meal full of dishes restrains your mind.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Inn
            lore_author = pgettext("empire_building", "Warning from a master miner to an apprentice"),
            -- TRANSLATORS: Purpose helptext for an empire production site: Inn
            purpose = pgettext("empire_building", "Prepares rations for scouts and rations and meals to feed the miners in all mines."),
            -- TRANSLATORS: Performance helptext for an empire production site: Inn
            performance = pgettext("empire_building", "The innkeeper needs %1% on average to prepare a ration and a meal. If the economy doesn’t need both, the innkeeper needs %2% on average to prepare one ration or one meal."):bformat(format_minutes_seconds(1, 25), format_seconds(48))
         }
      },
      {
         name = "empire_charcoal_kiln",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "Are they to be marked with chalk or charcoal?"
               _("Creta an carbone notandi?"),
               -- TRANSLATORS: Lore helptext for an empire production site: Charcoal Kiln
               --    Translation for the classical Latin quote:
               --    "Creta an carbone notandi?"
               _("Are they to be marked with chalk or charcoal?")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Charcoal Kiln
            lore_author = pgettext("empire_building",
               "Imperial proverb meaning to judge whether they are good or bad"
            ),
            -- TRANSLATORS: Purpose helptext for an empire production site: Charcoal Kiln
            purpose = pgettext("building", "Burns logs into charcoal.")
         }
      },
      {
         name = "empire_smelting_works",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "I see that iron was produced from the same darkness as gold."
               _("Video ferrum ex isdem tenebris esse prolatum, quibus aurum."),
               -- TRANSLATORS: Lore helptext for an empire production site: Smelting Works
               --    Translation for the classical Latin quote:
               --    "Video ferrum ex isdem tenebris esse prolatum, quibus aurum."
               _("I see that iron was produced from the same darkness as gold.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Smelting Works
            lore_author = pgettext("empire_building", "Amalea after inspecting a smelting works"),
            -- TRANSLATORS: Purpose helptext for an empire production site: Smelting Works
            purpose = pgettext("building", "Smelts iron ore into iron and gold ore into gold.")
         }
      },
      {
         name = "empire_toolsmithy",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "Each man is the maker of his own fortune."
               _("Faber est suæ quisque fortunæ."),
               -- TRANSLATORS: Lore helptext for an empire production site: Toolsmithy
               --    Translation for the classical Latin quote:
               --    "Faber est suæ quisque fortunæ."
               _("Each man is the maker of his own fortune.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Toolsmithy
            lore_author = pgettext("empire_building", "Imperial proverb quoted in a toolsmith's advertising"),
            -- TRANSLATORS: Purpose helptext for an empire production site: Toolsmithy
            purpose = pgettext("building", "Forges all the tools that your workers need.")
         }
      },
      {
         name = "empire_armorsmithy",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "They enjoy their great armors under their capes, tightly woven from rings, rattling
               --     through the cloth."
               _("Hi loricarum vasto sub tegmine gaudent, intexit creber sibimet quas circulus hærens,"..
                 " atque catenosi crepitant per corpora panni."),
               -- TRANSLATORS: Lore helptext for an empire production site: Armorsmithy
               --    Translation for the classical Latin quote:
               --    "Hi loricarum vasto sub tegmine gaudent, intexit creber sibimet quas circulus hærens,
               --     atque catenosi crepitant per corpora panni."
               _("They enjoy their great armors under their capes, tightly woven from rings, rattling"..
                 " through the cloth.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Armorsmithy
            lore_author = pgettext("empire_building", "An armorsmith teaching an apprentice"),
            -- TRANSLATORS: Purpose helptext for an empire production site: Armor Smithy
            purpose = pgettext("empire_building", "Forges armor and helmets that are used for training soldiers’ health in the training camp.")
         }
      },
      {
         name = "empire_barracks",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "Not walls but men protect the towns."
               _("Non muri, sed viri sunt præsidium oppidorum."),
               -- TRANSLATORS: Lore helptext for an empire production site: Barracks
               --    Translation for the classical Latin quote:
               --    "Non muri, sed viri sunt præsidium oppidorum."
               _("Not walls but men protect the towns.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Barracks
            lore_author = pgettext("empire_building", "Argument in the Senate for increasing recruitment"),
            -- TRANSLATORS: Purpose helptext for an empire production site: Barracks
            purpose = pgettext("empire_building", "Equips recruits and trains them as soldiers.")
         }
      },

      -- Big
      {
         name = "empire_donkeyfarm",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "A donkey prefers straw to gold."
               _("Asinus stramen mavult quam aurum."),
               -- TRANSLATORS: Lore helptext for an empire production site: Donkey Farm
               --    Translation for the classical Latin quote:
               --    "Asinus stramen mavult quam aurum."
               _("A donkey prefers straw to gold.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Donkey Farm
            lore_author = pgettext("empire_building", "A donkey breeder during a shortage of wheat"),
            -- TRANSLATORS: Purpose helptext for an empire production site: Donkey Farm
            purpose = pgettext("empire_building", "Breeds cute and helpful donkeys for adding them to the transportation system.")
         }
      },
      {
         name = "empire_farm",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Adaptation of a classical Latin quote meaning:
               --    "Lutius had already removed himself from public affairs, he was living in the countryside,
               --     and the ambassadors of the senate found him while he was plowing his land."
               _("Lutius iam a publicis negotiis se removerat, ruri vivebat et senatus legati eum invenerunt"..
                 " dum agellum suum arat."),
               -- TRANSLATORS: Lore helptext for an empire production site: Farm
               --    Translation for the adapted classical Latin quote:
               --    "Lutius iam a publicis negotiis se removerat, ruri vivebat et senatus legati eum invenerunt
               --     dum agellum suum arat."
               _("Lutius had already removed himself from public affairs, he was living in the countryside,"..
                 " and the ambassadors of the senate found him while he was plowing his land.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Farm
            lore_author = pgettext("empire_building", "History of the Empire, Part VII"),
            -- TRANSLATORS: Purpose helptext for an empire production site: Farm
            purpose = pgettext("building", "Sows and harvests wheat."),
            -- TRANSLATORS: Performance helptext for an empire production site: Farm
            performance = pgettext("empire_building", "The farmer needs %1% on average to sow and harvest a sheaf of wheat."):bformat(format_minutes_seconds(1, 20))
         }
      },
      {
         name = "empire_piggery",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "What does the pig really have other than meat?"
               _("Sus vero quid habet præter escam?"),
               -- TRANSLATORS: Lore helptext for an empire production site: Piggery
               --    Translation for the classical Latin quote:
               --    "Sus vero quid habet præter escam?"
               _("What does the pig really have other than meat?")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Piggery
            lore_author = pgettext("empire_building", "Excerpt from the Encyclopædia Imperica"),
            -- TRANSLATORS: Purpose helptext for an empire production site: Piggery
            purpose = pgettext("empire_building", "Breeds pigs for their meat."),
            -- TRANSLATORS: Performance helptext for an empire production site: Piggery
            performance = pgettext("empire_building", "The pig breeder needs %1% on average to raise and slaughter a pig."):bformat(format_minutes(1))
         }
      },
      {
         name = "empire_sheepfarm",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "Try to rescue a lamb from a wolf."
               _("Conari agnum eripere lupo."),
               -- TRANSLATORS: Lore helptext for an empire production site: Sheep Farm
               --    Translation for the classical Latin quote:
               --    "Conari agnum eripere lupo."
               _("Try to rescue a lamb from a wolf.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Sheep Farm
            lore_author = pgettext("empire_building", "Imperial proverb for trying the impossible"),
            -- TRANSLATORS: Purpose helptext for an empire production site: Sheep Farm
            purpose = pgettext("empire_building", "Keeps sheep for their wool.")
         }
      },
      {
         name = "empire_weaving_mill",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "Clothes make a man."
               _("Vestis virum reddit."),
               -- TRANSLATORS: Lore helptext for an empire production site: Weaving Mill
               --    Translation for the classical Latin quote:
               --    "Vestis virum reddit."
               _("Clothes make a man.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Weaving Mill
            lore_author = pgettext("empire_building", "Motto of the Weavers’ Guild"),
            -- TRANSLATORS: Purpose helptext for an empire production site: Weaving Mill
            purpose = pgettext("empire_building", "Weaves cloth out of wool.")
         }
      },
      {
         name = "empire_weaponsmithy",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "The fire keeps its heat even in the iron."
               _("Ignis calorem suum etiam in ferro tenet."),
               -- TRANSLATORS: Lore helptext for an empire production site: Weapon Smithy
               --    Translation for the classical Latin quote:
               --    "Ignis calorem suum etiam in ferro tenet."
               _("The fire keeps its heat even in the iron.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Weapon Smithy
            lore_author = pgettext("empire_building", "Motto of the Weaponsmiths’ Guild"),
            -- TRANSLATORS: Purpose helptext for an empire production site: Weapon Smithy
            purpose = pgettext("empire_building", "Forges spears to equip the soldiers and to train their attack in the training camp.")
         }
      },

      -- Mines
      {
         name = "empire_marblemine",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "He who hurts writes in the wind, he who is hurt in marble."
               _("In vento scribit lædens, in marmore læsus."),
               -- TRANSLATORS: Lore helptext for an empire production site: Marble Mine
               --    Translation for the classical Latin quote:
               --    "In vento scribit lædens, in marmore læsus."
               _("He who hurts writes in the wind, he who is hurt in marble.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Marble Mine
            lore_author = pgettext("empire_building", "Imperial proverb"),
            purpose = {
               -- TRANSLATORS: Purpose helptext for an empire production site: Marble Mine
               pgettext("empire_building", "Carves marble and granite out of the rock in mountain terrain.")
            }
         }
      },
      {
         name = "empire_marblemine_deep",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "He found a city of brick, and left it one of marble."
               _("Urbem lateritiam invenit, marmoream reliquit."),
               -- TRANSLATORS: Lore helptext for an empire production site: Deep Marble Mine
               --    Translation for the classical Latin quote:
               --    "Urbem lateritiam invenit, marmoream reliquit."
               _("He found a city of brick, and left it one of marble.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Deep Marble Mine
            lore_author = pgettext("empire_building",
               "History of the Empire, Part VI on the return of Lutius to Fremil"
            ),
            -- TRANSLATORS: Purpose helptext for an empire production site: Deep Marble Mine
            purpose = pgettext("empire_building", "Carves marble and granite out of the rock in mountain terrain.")
         }
      },
      {
         name = "empire_coalmine",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "Coal is revived from the dead, it turns from black to red, creates embers,
               --     and as it falls apart, it becomes ashes."
               _("Carbo de mortuo reviviscit, de nigro in ruborem vertitur, favillas generat,"..
                 " et dum resolvitur, cinerea."),
               -- TRANSLATORS: Lore helptext for an empire production site: Coal Mine
               --    Translation for the classical Latin quote:
               --    "Carbo de mortuo reviviscit, de nigro in ruborem vertitur, favillas generat,
               --     et dum resolvitur, cinerea."
               _("Coal is revived from the dead, it turns from black to red, creates embers,"..
                 " and as it falls apart, it becomes ashes.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Coal Mine
            lore_author = pgettext("empire_building", "Excerpt from the Encyclopædia Imperica"),
            -- TRANSLATORS: Purpose helptext for an empire production site: Coal Mine
            purpose = pgettext("building", "Digs coal out of the ground in mountain terrain.")
         }
      },
      {
         name = "empire_coalmine_deep",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "For there are more types of fire: for coal and flame are different, and the type
               --     that gives light, all of them are fire."
               _("Non enim est una species ignis: nam diversum est carbo et flamma, et lux specie,"..
                 " cum unumquodque horum sit ignis."),
               -- TRANSLATORS: Lore helptext for an empire production site: Deep Coal Mine
               --    Translation for the classical Latin quote:
               --    "Non enim est una species ignis: nam diversum est carbo et flamma, et lux specie,
               --     cum unumquodque horum sit ignis."
               _("For there are more types of fire: for coal and flame are different, and the type"..
                 " that gives light, all of them are fire.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Deep Coal Mine
            lore_author = pgettext("empire_building", "Excerpt from the Encyclopædia Imperica"),
            -- TRANSLATORS: Purpose helptext for an empire production site: Deep Coal Mine
            purpose = pgettext("building", "Digs coal out of the ground in mountain terrain."),
         }
      },
      {
         name = "empire_ironmine",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "Iron is sharpened by iron."
               _("Ferrum ferro acuitur."),
               -- TRANSLATORS: Lore helptext for an empire production site: Iron Mine
               --    Translation for the classical Latin quote:
               --    "Ferrum ferro acuitur."
               _("Iron is sharpened by iron.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Iron Mine
            lore_author = pgettext("empire_building", "A proverb that is vehemently disputed by iron miners"),
            -- TRANSLATORS: Purpose helptext for an empire production site: Iron Mine
            purpose = pgettext("building", "Digs iron ore out of the ground in mountain terrain.")
         }
      },
      {
         name = "empire_ironmine_deep",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "By constant use an iron ring is worn away."
               _("Ferreus assiduo consumitur annulus usu."),
               -- TRANSLATORS: Lore helptext for an empire production site: Deep Iron Mine
               --    Translation for the classical Latin quote:
               --    "Ferreus assiduo consumitur annulus usu."
               _("By constant use an iron ring is worn away.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Deep Iron Mine
            lore_author = pgettext("empire_building",
               "Amalea on the importance of always looking for more resources"
            ),
            -- TRANSLATORS: Purpose helptext for an empire production site: Deep Iron Mine
            purpose = pgettext("building", "Digs iron ore out of the ground in mountain terrain.")
         }
      },
      {
         name = "empire_goldmine",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "Gold discovered and all the better for being so."
               _("Aurum irrepertum et sic melius situm."),
               -- TRANSLATORS: Lore helptext for an empire production site: Gold Mine
               --    Translation for the classical Latin quote:
               --    "Aurum irrepertum et sic melius situm."
               _("Gold discovered and all the better for being so.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Gold Mine
            lore_author = pgettext("empire_building",
               "Amalea receiving the first geologist report of gold on the small island"
            ),
            -- TRANSLATORS: Purpose helptext for an empire production site: Gold Mine
            purpose = pgettext("building", "Digs gold ore out of the ground in mountain terrain.")
         }
      },
      {
         name = "empire_goldmine_deep",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "Gold has conquered the fear of death."
               _("Formidinem mortis vicit aurum."),
               -- TRANSLATORS: Lore helptext for an empire production site: Deep Gold Mine
               --    Translation for the classical Latin quote:
               --    "Formidinem mortis vicit aurum."
               _("Gold has conquered the fear of death.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Deep Gold Mine
            lore_author = pgettext("empire_building",
               "General Trepidus Egenus Attenuatus on motivating soldiers"
            ),
            -- TRANSLATORS: Purpose helptext for an empire production site: Deep Gold Mine
            purpose = pgettext("building", "Digs gold ore out of the ground in mountain terrain.")
         }
      },

      -- Training Sites
      {
         name = "empire_arena",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "[The People of the Empire] who once handed out all [power], now restrains itself
               --     and anxiously hopes for just two things: bread and games."
               _("Qui dabat olim […] omnia, nunc se continet atque duas tantum res anxius optat,"..
                 " panem et circenses."),
               -- TRANSLATORS: Lore helptext for an empire training site: Arena
               --    Translation for the classical Latin quote:
               --    "Qui dabat olim […] omnia, nunc se continet atque duas tantum res anxius optat,
               --     panem et circenses."
               _("[The People of the Empire] who once handed out all [power], now restrains itself"..
                 " and anxiously hopes for just two things: bread and games.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire training site: Arena
            lore_author = pgettext("empire_building", "History of the Empire, Part X – the decline"),
            purpose = {
               -- TRANSLATORS: Purpose helptext for an empire training site: Arena, part 1
               pgettext("empire_building", "Trains soldiers in ‘Evade’."),
               -- TRANSLATORS: Purpose helptext for an empire training site: Arena, part 2
               pgettext("empire_building", "‘Evade’ increases the soldier’s chance not to be hit by the enemy and so to remain totally unaffected.")
            },
            -- TRANSLATORS: Note helptext for an empire training site: Arena
            note = pgettext("empire_building", "Imperial soldiers cannot be trained in ‘Defense’ and will remain at the level with which they came.")
         }
      },
      {
         name = "empire_colosseum",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "Hail Cæsar! Those who are about to die greet you!"
               _("Ave Cæsar! Morituri te salutant!"),
               -- TRANSLATORS: Lore helptext for an empire training site: Colosseum
               --    Translation for the classical Latin quote:
               --    "Ave Cæsar! Morituri te salutant!"
               _("Hail Cæsar! Those who are about to die greet you!")
            ),
            -- TRANSLATORS: Lore author helptext for an empire training site: Colosseum
            lore_author = pgettext("empire_building", "Greeting from the gladiators to the emperor"),
            purpose = {
               -- TRANSLATORS: Purpose helptext for an empire training site: Colosseum, part 1
               pgettext("empire_building", "Trains soldiers in ‘Evade’."),
               -- TRANSLATORS: Purpose helptext for an empire training site: Colosseum, part 2
               pgettext("empire_building", "‘Evade’ increases the soldier’s chance not to be hit by the enemy and so to remain totally unaffected.")
            },
            -- TRANSLATORS: Note helptext for an empire training site: Colosseum
            note = pgettext("empire_building", "Imperial soldiers cannot be trained in ‘Defense’ and will remain at the level with which they came.")
         }
      },
      {
         name = "empire_trainingcamp",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "Fortune helps the strong."
               _("Fortes fortuna adiuvat."),
               -- TRANSLATORS: Lore helptext for an empire training site: Training Camp
               --    Translation for the classical Latin quote:
               --    "Fortes fortuna adiuvat."
               _("Fortune helps the strong.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire training site: Training Camp
            lore_author = pgettext("empire_building",
               "Trainer to soldiers who questioned the need for higher level training"
            ),
            purpose = {
               -- TRANSLATORS: Purpose helptext for an empire training site: Training Camp, part 1
               pgettext("empire_building", "Trains soldiers in ‘Attack’ and in ‘Health’."),
               -- TRANSLATORS: Purpose helptext for an empire training site: Training Camp, part 2
               pgettext("empire_building", "Equips the soldiers with all necessary weapons and armor parts.")
            },
            -- TRANSLATORS: Note helptext for an empire training site: Training Camp
            note = pgettext("empire_building", "Imperial soldiers cannot be trained in ‘Defense’ and will remain at the level with which they came.")
         }
      },

      -- Military Sites
      {
         name = "empire_blockhouse",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "An eagle doesn’t catch flies."
               _("Aquila non captat muscas."),
               -- TRANSLATORS: Lore helptext for an empire military site: Blockhouse
               --    Translation for the classical Latin quote:
               --    "Aquila non captat muscas."
               _("An eagle doesn’t catch flies.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire military site: Blockhouse
            lore_author = pgettext("empire_building",
               "General Passerus Claudius Pedestrus on why he didn’t order the destruction of a"..
               " rebel Blockhouse"
            ),
            -- TRANSLATORS: Purpose helptext for an empire military site: Blockhouse
            purpose = pgettext("empire_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an empire military site: Blockhouse
            note = pgettext("empire_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "empire_sentry",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "Who watches the watchmen?"
               _("Quis custodiet ipsos custodes?"),
               -- TRANSLATORS: Lore helptext for an empire production site: Sentry
               --    Translation for the classical Latin quote:
               --    "Quis custodiet ipsos custodes?"
               _("Who watches the watchmen?")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Sentry
            --              (the Limes was the border defence system of fortifications of the Roman Empire)
            lore_author = pgettext("empire_building",
               "Saledus warning Lutius that sentries are not adequate protection for the Limes"
            ),
            -- TRANSLATORS: Purpose helptext for an empire military site: Sentry
            purpose = pgettext("empire_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an empire military site: Sentry
            note = pgettext("empire_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "empire_outpost",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "Two were stationed on the other side of the city than the camp: one over the river, the other by
               --     the river above the camp."
               _("Binæ contra urbem a castris station erant, una ultra flumen, altera eis flumen supra castra."),
               -- TRANSLATORS: Lore helptext for an empire military site: Outpost
               --    Translation for the classical Latin quote:
               --    "Binæ contra urbem a castris station erant, una ultra flumen, altera eis flumen supra castra."
               _("Two were stationed on the other side of the city than the camp: one over the river, the other by"..
                 " the river above the camp.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire military site: Outpost
            lore_author = pgettext("empire_building",
               "History of the Empire, Part III, preparation for the battle of Uliginosium"
            ),
            -- TRANSLATORS: Purpose helptext for an empire military site: Outpost
            purpose = pgettext("empire_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an empire military site: Outpost
            note = pgettext("empire_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "empire_barrier",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "The die is cast!"
               _("Alea iacta est!"),
               -- TRANSLATORS: Lore helptext for an empire military site: Barrier
               --    Translation for the classical Latin quote:
               --    "Alea iacta est!"
               _("The die is cast!")
            ),
            -- TRANSLATORS: Lore author helptext for an empire military site: Barrier
            lore_author = pgettext("empire_building",
               "Rebel general Certus Ignavus Fidelius upon entering the limits of Fremil to"..
               " overthrow the government"
            ),
            -- TRANSLATORS: Purpose helptext for an empire military site: Barrier
            purpose = pgettext("empire_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an empire military site: Barrier
            note = pgettext("empire_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "empire_tower",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "I came, I saw, I conquered!"
               _("Veni, Vidi, Vici!"),
               -- TRANSLATORS: Lore helptext for an empire military site: Tower
               --    Translation for the classical Latin quote:
               --    "Veni, Vidi, Vici!"
               _("I came, I saw, I conquered!")
            ),
            -- TRANSLATORS: Lore author helptext for an empire military site: Tower
            lore_author = pgettext("empire_building",
               "General Claudius Cæcus Cessus reporting a quick victory over the Barbarians thanks to "..
               "the superior vision of a tower"
            ),
            -- TRANSLATORS: Purpose helptext for an empire military site: Tower
            purpose = pgettext("empire_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an empire military site: Tower
            note = pgettext("empire_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "empire_fortress",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "If you want peace, prepare for war."
               _("Si vis pacem, para bellum."),
               -- TRANSLATORS: Lore helptext for an empire production site: Fortress
               --    Translation for the classical Latin quote:
               --    "Si vis pacem, para bellum."
               _("If you want peace, prepare for war.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire military site: Fortress
            lore_author = pgettext("empire_building", "Saledus arguing with Amalea"),
            -- TRANSLATORS: Purpose helptext for an empire military site: Fortress
            purpose = pgettext("empire_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an empire military site: Fortress
            note = pgettext("empire_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "empire_castle",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Adaptation of a classical Latin quote meaning:
               --    "Furthermore, I think that Al’thunran must be destroyed!"
               _("Ceterum autem censeo Althunranam esse delendam!"),
               -- TRANSLATORS: Lore helptext for an empire military site: Castle
               --    Translation for the adapted classical Latin quote:
               --    "Ceterum autem censeo Althunranam esse delendam!"
               _("Furthermore, I think that Al’thunran must be destroyed!")
            ),
            -- TRANSLATORS: Lore author helptext for an empire military site: Castle
            lore_author = pgettext("empire_building",
               "Saledus in the Senate, urging final victory over the Barbarians"
            ),
            -- TRANSLATORS: Purpose helptext for an empire military site: Castle
            purpose = pgettext("empire_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an empire military site: Castle
            note = pgettext("empire_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },

      -- Seafaring/Ferry Sites - these are only displayed on seafaring/ferry maps
      {
         name = "empire_ferry_yard",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "Who crossed seas does not fear rivers."
               _("Non terretur fluminibus, qui maria transiit."),
               -- TRANSLATORS: Lore helptext for an empire production site: Ferry Yard
               --    Translation for the classical Latin quote:
               --    "Non terretur fluminibus, qui maria transiit."
               _("Who crossed seas does not fear rivers.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Ferry Yard
            lore_author = pgettext("empire_building",
               "Amalea on the need for ferries after the reconstruction of Fremil"
            ),
            -- TRANSLATORS: Purpose helptext for an empire production site: Ferry Yard
            purpose = pgettext("building", "Builds ferries."),
            -- TRANSLATORS: Note helptext for an empire production site: Ferry Yard
            note = pgettext("building", "Needs water nearby.")
         }
      },
      {
         name = "empire_shipyard",
         helptexts = {
            lore = latin_lore(
               -- TRANSLATORS: DO NOT TRANSLATE, but you may transliterate into non-latin fonts.
               --    Classical Latin quote meaning:
               --    "Don’t put everything on the same ship."
               _("Uni navi ne committas omnia."),
               -- TRANSLATORS: Lore helptext for an empire production site: Shipyard
               --    Translation for the classical Latin quote:
               --    "Uni navi ne committas omnia."
               _("Don’t put everything on the same ship.")
            ),
            -- TRANSLATORS: Lore author helptext for an empire production site: Shipyard
            lore_author = pgettext("empire_building",
               "A shipwright lamenting when he was instructed to stop working"
            ),
            -- TRANSLATORS: Purpose helptext for an empire production site: Shipyard
            purpose = pgettext("building", "Constructs ships that are used for overseas colonization and for trading between ports.")
         }
      },

      -- Partially Finished Buildings - these are the same 2 buildings for all tribes
      {
         name = "constructionsite",
         helptexts = {
            -- TRANSLATORS: Lore helptext for an empire building: Construction Site
            lore = pgettext("building", "‘Don’t swear at the builder who is short of building materials.’"),
            -- TRANSLATORS: Lore author helptext for an empire building: Construction Site
            lore_author = pgettext("building", "Proverb widely used for impossible tasks of any kind"),
            -- TRANSLATORS: Purpose helptext for an empire building: Construction Site
            purpose = pgettext("building", "A new building is being built at this construction site.")
         }
      },
      {
         name = "dismantlesite",
         helptexts = {
            -- TRANSLATORS: Lore helptext for an empire building: Dismantle Site
            lore = pgettext("building", "‘New paths will appear when you are willing to tear down the old.’"),
            -- TRANSLATORS: Lore author helptext for an empire building: Dismantle Site
            lore_author = pgettext("building", "Proverb"),
            -- TRANSLATORS: Purpose helptext for an empire building: Dismantle Site
            purpose = pgettext("building", "A building is being dismantled at this dismantle site, returning some of the resources that were used during this building’s construction to your tribe’s stores.")
         }
      }
   },

   -- Productionsite status strings

   -- TRANSLATORS: Productivity label on an empire building if there is 1 worker missing
   productionsite_worker_missing = pgettext("empire", "Worker missing"),
   -- TRANSLATORS: Productivity label on an empire building if there is 1 worker coming
   productionsite_worker_coming = pgettext("empire", "Worker is coming"),
   -- TRANSLATORS: Productivity label on an empire building if there is more than 1 worker missing. If you need plural forms here, please let us know.
   productionsite_workers_missing = pgettext("empire", "Workers missing"),
   -- TRANSLATORS: Productivity label on an empire building if there is more than 1 worker coming. If you need plural forms here, please let us know.
   productionsite_workers_coming = pgettext("empire", "Workers are coming"),

   -- Special types
   builder = "empire_builder",
   carriers = {"empire_carrier", "empire_donkey"},
   geologist = "empire_geologist",
   scouts_house = "empire_scouts_house",
   soldier = "empire_soldier",
   ship = "empire_ship",
   ferry = "empire_ferry",
   port = "empire_port",

   fastplace = {
      warehouse = "empire_warehouse",
      port = "empire_port",
      training_small = "empire_arena",
      training_large = "empire_trainingcamp",
      military_small_primary = "empire_sentry",
      military_small_secondary = "empire_blockhouse",
      military_medium_primary = "empire_outpost",
      military_medium_secondary = "empire_barrier",
      military_tower = "empire_tower",
      military_fortress = "empire_fortress",
      woodcutter = "empire_lumberjacks_house",
      forester = "empire_foresters_house",
      quarry = "empire_quarry",
      building_materials_primary = "empire_sawmill",
      building_materials_secondary = "empire_stonemasons_house",
      fisher = "empire_fishers_house",
      hunter = "empire_hunters_house",
      fish_meat_replenisher = "empire_piggery",
      well = "empire_well",
      farm_primary = "empire_farm",
      mill = "empire_mill",
      bakery = "empire_bakery",
      brewery = "empire_brewery",
      tavern = "empire_tavern",
      smelting = "empire_smelting_works",
      tool_smithy = "empire_toolsmithy",
      weapon_smithy = "empire_weaponsmithy",
      armor_smithy = "empire_armorsmithy",
      weaving_mill = "empire_weaving_mill",
      shipyard = "empire_shipyard",
      ferry_yard = "empire_ferry_yard",
      scout = "empire_scouts_house",
      barracks = "empire_barracks",
      second_carrier = "empire_donkeyfarm",
      charcoal = "empire_charcoal_kiln",
      mine_stone = "empire_marblemine",
      mine_coal = "empire_coalmine",
      mine_iron = "empire_ironmine",
      mine_gold = "empire_goldmine",
      agriculture_producer = "empire_vineyard",
      agriculture_consumer_primary = "empire_winery",
      industry_supporter = "empire_sheepfarm",
   },
}

pop_textdomain()
