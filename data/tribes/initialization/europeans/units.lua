descriptions = wl.Descriptions()

image_dirname = path.dirname(__file__) .. "images/"

push_textdomain("tribes_encyclopedia")

-- For formatting time strings
include "tribes/scripting/help/time_strings.lua"

descriptions:new_tribe {
   name = "europeans",
   military_capacity_script = path.dirname(__file__) .. "military_capacity.lua",
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
         [0] = "europeans_resi_none",
      },
      resource_coal = {
         [10] = "europeans_resi_coal_1",
         [20] = "europeans_resi_coal_2",
      },
      resource_iron = {
         [10] = "europeans_resi_iron_1",
         [20] = "europeans_resi_iron_2",
      },
      resource_gold = {
         [10] = "europeans_resi_gold_1",
         [20] = "europeans_resi_gold_2",
      },
      resource_stones = {
         [10] = "europeans_resi_stones_1",
         [20] = "europeans_resi_stones_2",
      },
      resource_water = {
         [100] = "europeans_resi_water",
      },
   },

   -- Wares positions in wares windows.
   -- This also gives us the information which wares the tribe uses.
   -- Each subtable is a column in the wares windows.
   wares_order = {
      {
         -- Building Materials 1
         {
            name = "log",
            default_target_quantity = 32,
            preciousness = 32,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an europeans ware: Log, part 1
                  pgettext("ware", "Logs are an important basic building material. They are produced by felling trees."),
                  -- TRANSLATORS: Helptext for an europeans ware: Log, part 2
                  pgettext("europeans_ware", "Europeans lumberjacks fell the trees; foresters take care of the supply of trees. Logs are also used by the charcoal kiln, the shipyard and the sawmill.")
               }
            }
         },
         {
            name = "planks",
            default_target_quantity = 16,
            preciousness = 28,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an europeans ware: Planks, part 1
                  pgettext("ware", "Planks are an important building material."),
                  -- TRANSLATORS: Helptext for an europeans ware: Planks, part 2
                  pgettext("europeans_ware", "They are produced out of logs by the sawmill."),
                  -- TRANSLATORS: Helptext for an europeans ware: Planks, part 3
                  pgettext("europeans_ware", "They are also used by the tool smithy and weapon smithy.")
               }
            }
         },
         {
            name = "reed",
            default_target_quantity = 32,
            preciousness = 24,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an europeans ware: Reed, part 1
                  pgettext("ware", "Reed are an important building material."),
                  -- TRANSLATORS: Helptext for an europeans ware: Reed, part 2
                  pgettext("europeans_ware", "They are produced by reed yard."),
               }
            }
         },
         {
            name = "wool",
            default_target_quantity = 16,
            preciousness = 8,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Wool
               purpose = pgettext("europeans_ware", "Wool is the hair of sheep. Weaving mills use it to make armor.")
            }
         },
         {
            name = "spider_silk",
            default_target_quantity = 32,
            preciousness = 24,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Wool
               purpose = pgettext("europeans_ware", "Spidersilk is produced by spiderfarm. Weaving mills use it to make spidercloth and tabard.")
            }
         },
         {
            name = "spidercloth",
            default_target_quantity = 32,
            preciousness = 24,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Cloth
               purpose = pgettext("europeans_ware", "Spidercloth is needed to build several buildings. It is also consumed in the tool smithy.")
            }
         }
         
      },
      {
         -- Building Materials 1
         {
            name = "granite",
            default_target_quantity = 16,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an europeans ware: Granite, part 1
                  pgettext("ware", "Granite is a basic building material."),
                  -- TRANSLATORS: Helptext for an europeans ware: Granite, part 2
                  pgettext("europeans_ware", "The europeans produces granite blocks in quarries.")
               }
            }
         },
         {
            name = "grout",
            default_target_quantity = 16,
            preciousness = 4,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an europeans ware: Granite, part 1
                  pgettext("ware", "Grout is a basic building material."),
                  -- TRANSLATORS: Helptext for an europeans ware: Granite, part 2
                  pgettext("europeans_ware", "The europeans produces grout in stonemason huts out of granite and water.")
               }
            }
         },
         {
            name = "clay",
            default_target_quantity = 4,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an europeans ware: Granite, part 1
                  pgettext("ware", "Clay is needed to make bricks."),
                  -- TRANSLATORS: Helptext for an europeans ware: Granite, part 2
                  pgettext("europeans_ware", "The europeans gets clay from claydiggers house.")
               }
            }
         },
         {
            name = "brick",
            default_target_quantity = 16,
            preciousness = 4,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an europeans ware: Granite, part 1
                  pgettext("ware", "Brick is a basic building material."),
                  -- TRANSLATORS: Helptext for an europeans ware: Granite, part 2
                  pgettext("europeans_ware", "The europeans produces bricks in stonemason huts out of clay, granite and water.")
               }
            }
         },
         {
            name = "marble",
            default_target_quantity = 8,
            preciousness = 4,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Marble
               purpose = pgettext("europeans_ware", "Marble is the beautiful stone which is cut out of the mountains or produced in a quarry. Marble is used as a building material and gets chiseled into marble columns in the stonemason’s house.")
            }
         },
         {
            name = "marble_column",
            default_target_quantity = 32,
            preciousness = 8,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Marble Column
               purpose = pgettext("europeans_ware", "Marble columns represent the high culture of the europeans, so they are needed for nearly every larger building. They are produced out of marble at a stonemason’s house.")
            }
         },
         {
            name = "quartz",
            default_target_quantity = 16,
            preciousness = 8,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Quartz
               purpose = pgettext("europeans_ware", "These transparent quartz gems are used to build some exclusive buildings. They are produced in a crystal quarry.")
            }
         },
         {
            name = "diamond",
            default_target_quantity = 16,
            preciousness = 8,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Diamond
               purpose = pgettext("europeans_ware", "These wonderful diamonds are used to build some exclusive buildings. They are mined in a deep coal mine.")
            }
         },
      },
      {
         -- Food 1
         {
            name = "fish",
            default_target_quantity = 16,
            preciousness = 16,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Fish
               purpose = pgettext("europeans_ware", "Fish is one of the biggest food resources of the Europeans. It has to be smoked in a smokery before being delivered to mines, training sites and scouts.")
            }
         },
         {
            name = "meat",
            default_target_quantity = 16,
            preciousness = 16,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an europeans ware: Meat, part 1
                  pgettext("ware", "Meat contains a lot of energy, and it is obtained from wild game taken by hunters."),
                  -- TRANSLATORS: Helptext for an europeans ware: Meat, part 2
                  pgettext("europeans_ware", "Meat has to be smoked in a smokery before being delivered to mines and training sites.")
               }
            }
         },
         {
            name = "corn",
            default_target_quantity = 16,
            preciousness = 24,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Corn
               purpose = pgettext("europeans_ware", "This corn is processed in the mill into fine cornmeal that every baker needs for a good bread. Also spider farms need to be provided with corn.")
            }
         },
         {
            name = "blackroot",
            default_target_quantity = 16,
            preciousness = 24,
            helptexts = {
               -- TRANSLATORS: Helptext for an atlantean ware: Blackroot
               purpose = pgettext("europeans_ware", "Blackroots are a special kind of root. Also sheep farms need to be provided with corn.")
            }
         },
         {
            name = "rye",
            default_target_quantity = 16,
            preciousness = 16,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an europeans ware: Wheat, part 1
                  pgettext("ware", "Rye is essential for survival."),
                  -- TRANSLATORS: Helptext for an europeans ware: Wheat, part 2
                  pgettext("europeans_ware", "Wheat is produced by farms and used by mills.")
               }
            }
         },
         {
            name = "wheat",
            default_target_quantity = 16,
            preciousness = 16,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an europeans ware: Wheat, part 1
                  pgettext("ware", "Wheat is essential for survival."),
                  -- TRANSLATORS: Helptext for an europeans ware: Wheat, part 2
                  pgettext("europeans_ware", "Wheat is produced by farms and used by mills. Sheep farms also need to be supplied with wheat.")
               }
            }
         },
         {
            name = "flour",
            default_target_quantity = 16,
            preciousness = 16,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Flour
               purpose = pgettext("europeans_ware", "Flour is produced by the mill out of rye and wheat and is needed in the bakery to produce the tasty europeans bread.")
            }
         },
         {
            name = "ration",
            default_target_quantity = 32,
            preciousness = 32,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a barbarian ware: Ration, part 1
                  pgettext("ware", "A small bite to keep miners strong and working. The scout also consumes rations on his scouting trips."),
                  -- TRANSLATORS: Helptext for a barbarian ware: Ration, part 2
                  pgettext("europeans_ware", "Rations are produced in a tavern, an inn or a big inn out of fish or meat or pitta bread.")
               }
            }
         },
         {
            name = "snack",
            default_target_quantity = 32,
            preciousness = 32,
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian ware: Snack
               purpose = pgettext("europeans_ware", "A bigger morsel than the ration to provide miners in deep mines.")
            }
         },
         {
            name = "meal",
            default_target_quantity = 32,
            preciousness = 32,
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian ware: Meal
               purpose = pgettext("europeans_ware", "A meal is made out of pitta bread, strong beer and fish/meat in a big inn. This substantial food is exactly what workers in a deeper mine need.")
            }
         }
      },
      {
         -- Food 2
        {
            name = "water",
            default_target_quantity = 32,
            preciousness = 16,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an europeans ware: Water, part 1
                  pgettext("ware", "Water is the essence of life!"),
                  -- TRANSLATORS: Helptext for an europeans ware: Water, part 2
                  pgettext("europeans_ware", "Water is used by fishbreeder, gamekeeper, by all farms, and in the bakery and the brewery. The spider farm and the sheep farm need to be supplied with water.")
               }
            }
         },
         {
            name = "barley",
            default_target_quantity = 16,
            preciousness = 24,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Barley
               purpose = pgettext("europeans_ware", "Barley is a slow-growing grain that is used for brewing beer.")
            }
         },
         {
            name = "beer",
            default_target_quantity = 32,
            preciousness = 16,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Beer
               purpose = pgettext("europeans_ware", "This beer is produced in a brewery out of barley and water. It is consumed in trainingssites.")
            }
         },
         {
            name = "beer_strong",
            default_target_quantity = 32,
            preciousness = 16,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Beer
               purpose = pgettext("europeans_ware", "This strong beer is produced in a brewery out of barley and water. It is consumed in trainingssites.")
            }
         },
         {
            name = "honey",
            default_target_quantity = 16,
            preciousness = 4,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Honey
               purpose = pgettext("europeans_ware", "Honey is produced by bees belonging to a beekeeper. It is used to brew mead.")
            }
         },
         {
            name = "mead",
            default_target_quantity = 32,
            preciousness = 8,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Mead
               purpose = pgettext("europeans_ware", "Mead is produced by breweries. Soldiers drink mead during advanced training.")
            }
         },
         {
            name = "fruit",
            default_target_quantity = 16,
            preciousness = 4,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Fruit
               purpose = pgettext("europeans_ware", "Fruit are berries gathered from berry bushes by a fruit collector. They are used to brew fruit wine.")
            }
         },
         {
            name = "grape",
            default_target_quantity = 16,
            preciousness = 4,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Grape
               purpose = pgettext("europeans_ware", "These grapes are the base for a tasty wine. They are harvested in a vineyard and processed in a winery.")
            }
         },
         {
            name = "wine",
            default_target_quantity = 32,
            preciousness = 8,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Wine
               purpose = pgettext("europeans_ware", "This tasty wine is drunk by soldiers to train in colosseum. It is produced in a winery.")
            }
         }
      },
      {
         -- Mining
         {
            name = "coal",
            default_target_quantity = 32,
            preciousness = 32,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an europeans ware: Coal, part 1
                  pgettext("ware", "Coal is mined in coal mines or produced out of logs by a charcoal kiln."),
                  -- TRANSLATORS: Helptext for an europeans ware: Coal, part 2
                  pgettext("europeans_ware", "The fires of the europeans smelting works, armor smithies and weapon smithies are usually fed with coal.")
               }
            }
         },
         {
            name = "ore",
            default_target_quantity = 32,
            preciousness = 16,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an europeans ware: Iron Ore, part 1
                  pgettext("default_ware", "Ore is mined in ore mines."),
                  -- TRANSLATORS: Helptext for an europeans ware: Iron Ore, part 2
                  pgettext("europeans_ware", "It is smelted in a smelting works to retrieve all the metals.")
               }
            }
         },
         {
            name = "scrap_iron",
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Scrap Iron
               purpose = pgettext("europeans_ware", "Discarded weapons and armor can be recycled in a recycling center to produce new tools, weapon and armor.")
            }
         },
         {
            name = "iron",
            default_target_quantity = 32,
            preciousness = 8,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an europeans ware: Iron, part 1
                  pgettext("ware", "Iron is smelted out of ores."),
                  -- TRANSLATORS: Helptext for an europeans ware: Iron, part 2
                  pgettext("europeans_ware", "It is produced by the smelting works. Weapons, armor and tools are made of iron.")
               }
            }
         },
         {
            name = "scrap_metal_mixed",
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Scrap metal (mixed)
               purpose = pgettext("europeans_ware", "Discarded weapons and armor can be recycled in a recycling center to produce new tools, weapon and armor.")
            }
         },
         {
            name = "gold",
            default_target_quantity = 32,
            preciousness = 8,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an europeans ware: Gold, part 1
                  pgettext("ware", "Gold is the most valuable of all metals, and it is smelted out of ore."),
                  -- TRANSLATORS: Helptext for an europeans ware: Gold, part 2
                  pgettext("europeans_ware", "It is produced by the smelting works. Armor and weapons are embellished with gold in the armor smithy and the weapon smithy.")
               }
            }
         }
      },
      {
         -- Tools 1
         {
            name = "hammer",
            default_target_quantity = 2,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an europeans ware: Hammer, part 1
                  pgettext("ware", "The hammer is an essential tool."),
                  -- TRANSLATORS: Helptext for an europeans ware: Hammer, part 2
                  pgettext("europeans_ware", "Geologists, builders, weaponsmiths and armorsmiths all need a hammer. Make sure you’ve always got some in reserve! They are produced by the toolsmith.")
               }
            }
         },
         {
            name = "saw",
            default_target_quantity = 2,
            preciousness = 1,
            helptexts = {
               purpose = {
                   -- TRANSLATORS: Helptext for an europeans ware: Saw, part 1
                   pgettext("ware", "The saw is an essential tool."),
                   -- TRANSLATORS: Helptext for an europeans ware: Saw, part 2
                   pgettext("europeans_ware", "This saw is needed by the woodcutter and the carpenter. It is produced by the toolsmith.")
                }
            }
         },
         {
            name = "felling_ax",
            default_target_quantity = 2,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an europeans ware: Felling Ax, part 1
                  pgettext("ware", "The felling ax is the tool to chop down trees."),
                  -- TRANSLATORS: Helptext for an europeans ware: Felling Ax, part 2
                  pgettext("europeans_ware", "Felling axes are used by lumberjacks and produced by the toolsmithy.")
               }
            }
         },
         {
            name = "pick",
            default_target_quantity = 2,
            preciousness = 1,
            helptexts = {
               purpose = {
                   -- TRANSLATORS: Helptext for an europeans ware: Pick, part 1
                   pgettext("ware", "The pick is an essential tool."),
                   -- TRANSLATORS: Helptext for an europeans ware: Pick, part 2
                   pgettext("europeans_ware", "Picks are used by stonemasons and miners. They are produced by the toolsmith.")
                }
            }
         },
         {
            name = "shovel",
            default_target_quantity = 2,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an europeans ware: Shovel, part 1
                  pgettext("ware", "The shovel is an essential tool."),
                  -- TRANSLATORS: Helptext for an europeans ware: Shovel, part 2
                  pgettext("europeans_ware", "Therefore the forester, the claydigger and the farmer use them. They are produced by the toolsmith.")
               }
            }
         },
         {
            name = "buckets",
            default_target_quantity = 2,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Buckets
               purpose = pgettext("europeans_ware", "Big buckets for the forester, the claydigger, and the fish breeder – produced by the toolsmith.")
            }
         },
         {
            name = "fire_tongs",
            default_target_quantity = 2,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an europeans ware: Fire Tongs, part 1
                  pgettext("ware", "Fire tongs are the tools for smelting ores."),
                  -- TRANSLATORS: Helptext for an europeans ware: Fire Tongs, part 2
                  pgettext("europeans_ware", "They are used in the smelting works and smithies and produced by the toolsmith.")
               }
            }
         },
         {
            name = "milking_tongs",
            default_target_quantity = 2,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Milking Tongs
               purpose = pgettext("europeans_ware", "Milking tongs are used by the spider breeder to milk the spiders. They are produced by the toolsmith.")
            }
         },
         {
            name = "needles",
            default_target_quantity = 2,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Milking Tongs
               purpose = pgettext("europeans_ware", "Needöes are used by weaver.")
            }
         }
      },
      {
         -- Tools 2
         {
            name = "scythe",
            default_target_quantity = 2,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an europeans ware: Scythe, part 1
                  pgettext("ware", "The scythe is the tool of the farmers."),
                  -- TRANSLATORS: Helptext for an europeans ware: Scythe, part 2
                  pgettext("europeans_ware", "Scythes are produced by the toolsmith.")
               }
            }
         },
         {
            name = "basket",
            default_target_quantity = 2,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Basket
               purpose = pgettext("europeans_ware", "This basket is needed by the farmer for harvesting the fruits and grapes and needed by miller and by gamekeeper. It is produced by the toolsmith.")
            }
         },
         {
            name = "bread_paddle",
            default_target_quantity = 2,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an europeans ware: Bread Paddle, part 1
                  pgettext("ware", "The bread paddle is the tool of the baker, each baker needs one."),
                  -- TRANSLATORS: Helptext for an europeans ware: Bread Paddle, part 2
                  pgettext("europeans_ware", "Bread paddles are produced by the toolsmith.")
               }
            }
         },
         {
            name = "fishing_net",
            default_target_quantity = 2,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Fishing Net
               purpose = pgettext("europeans_ware", "The fishing net is used by the fisher and produced by the toolsmith.")
            }
         },
         {
            name = "fishing_rod",
            default_target_quantity = 2,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an europeans ware: Fishing Rod, part 1
                  pgettext("ware", "Fishing rods are needed by fishers to catch fish."),
                  -- TRANSLATORS: Helptext for an europeans ware: Fishing Rod, part 2
                  pgettext("europeans_ware", "They are produced by the toolsmith.")
               }
            }
         },
         {
            name = "hunting_bow",
            default_target_quantity = 2,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Hunting Bow
               purpose = pgettext("europeans_ware", "This bow is used by the hunter. It is produced by the toolsmith.")
            }
         },
         {
            name = "hunting_spear",
            default_target_quantity = 2,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an europeans ware: Hunting Spear, part 1
                  pgettext("ware", "This spear is light enough to be thrown, but heavy enough to kill any animal in one blow. It is only used by hunters."),
                  -- TRANSLATORS: Helptext for an europeans ware: Hunting Spear, part 2
                  pgettext("europeans_ware", "They are produced by the toolsmith.")
               }
            }
         },
         {
            name = "hook_pole",
            default_target_quantity = 2,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Hook Pole
               purpose = pgettext("europeans_ware", "This hook pole is used by the smoker to suspend all the meat and fish from the top of the smokery. It is created by the toolsmith.")
            }
         }
      },
      {
         -- Armor
         {
            name = "armor",
            default_target_quantity = 16,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Armor
               purpose = pgettext("europeans_ware", "Basic armor for europeans soldiers. It is produced in the weaving mill. In combination with wooden speer, it is the equipment to fit out young soldiers.")
            }
         },
         {
            name = "tabard",
            default_target_quantity = 16,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Tabard
               purpose = pgettext("europeans_ware", "Basic tabard for europeans soldiers. It is produced in the weaving mill. In combination with wooden speer, it is the equipment to fit out young soldiers.")
            }
         },
         {
            name = "armor_chain",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Chain Armor
               purpose = pgettext("europeans_ware", "The chain armor is a medium armor for europeans soldiers. It is produced in an armor smithy and used in a training camp – together with food – to train soldiers from health level 0 to level 1.")
            }
         },
         {
            name = "armor_helmet",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Helmet
               purpose = pgettext("europeans_ware", "The helmet is the medium armor of a soldier. It is produced in an armor smithy. It is produced in an armor smithy and used in a training camp – together with food – to train soldiers from health level 1 to level 2.")
            }
         },
         {
            name = "helmet_mask",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Helmet Mask
               purpose = pgettext("europeans_ware", "The mask is the medium armor of a soldier. It is produced in an armor smithy. It is produced in an armor smithy and used in a training camp – together with food – to train soldiers from health level 2 to level 3.")
            }
         },
         {
            name = "armor_gilded",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Gilded Armor
               purpose = pgettext("europeans_ware", "The gilded armor is an advanced armor of a soldier. It is produced in the armor smithy and used in the training camp – together with food – to train soldiers from health level 3 to level 4.")
            }
         },
         {
            name = "tabard_golden",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Golden Tabard
               purpose = pgettext("europeans_ware", "The golden tabard is an advanced armor of a soldier. It is produced in an armor smithy. It is produced in an armor smithy and used in a training camp – together with food – to train soldiers from health level 3 to level 4.")
            }
         },
         {
            name = "helmet_golden",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Golden Helmet
               purpose = pgettext("europeans_ware", "The golden helmet is an advanced armor of a soldier. It is produced in an armor smithy. It is produced in an armor smithy and used in a training camp – together with food – to train soldiers from health level 4 to level 5.")
            }
         },
         {
            name = "helmet_warhelm",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Warhelm
               purpose = pgettext("europeans_ware", "The warhelm is the strongest armor an europeans soldier can have. It is produced in an armor smithy. It is produced in an armor smithy and used in a training camp – together with food – to train soldiers from health level 5 to level 6.")
            }
         }
      },
      {
         -- Weapons and Shields
         {
            name = "spear_wooden",
            default_target_quantity = 32,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Wooden Spear
               purpose = pgettext("europeans_ware", "This wooden spear is the basic weapon in the europeans military system. It is produced in the tool smithy. In combination with armor or tabard, it is the equipment to fit out young soldiers.")
            }
         },
         {
            name = "spear_advanced",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Advanced Spear
               purpose = pgettext("europeans_ware", "This is a advanced spear with a steel tip. It is produced in a weapon smithy and used in a training camp – together with food – to train soldiers from attack level 0 to level 1.")
            }
         },
         {
            name = "ax_broad",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Broad Ax
               purpose = pgettext("europeans_ware", "This is an broad ax. It is produced in a weapon smithy and used in a training camp – together with food – to train soldiers from attack level 1 to level 2.")
            }
         },
         {
            name = "trident_steel",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Steel Trident
               purpose = pgettext("europeans_ware", "This is a advanced trident with a steel tip. It is produced in a weapon smithy and used in a training camp – together with food – to train soldiers from attack level 2 to level 3.")
            }
         },
         {
            name = "sword_broad",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Broad Sword
               purpose = pgettext("europeans_ware", "This is a broad sword. It is produced in a weapon smithy and used in a training camp – together with food – to train soldiers from attack level 3 to level 4.")
            }
         },
         {
            name = "ax_warriors",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Warrior Ax
               purpose = pgettext("europeans_ware", "This is a an warriors ax. It is produced in a weapon smithy and used in a training camp – together with food – to train soldiers from attack level 4 to level 5.")
            }
         },
         {
            name = "trident_heavy_double",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Heavy Double Trident
               purpose = pgettext("europeans_ware", "This is a heavy double trident. It is produced in a weapon smithy and used in a training camp – together with food – to train soldiers from attack level 5 to level 6.")
            }
         },
         {
            name = "shield_steel",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Steel Shield
               purpose = pgettext("europeans_ware", "This steel shield is produced in the armor smithy and used in the dungeon – together with food – to train soldiers from defense level 0 to level 3.")
            }
         },
         {
            name = "shield_advanced",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans ware: Advanced Shield
               purpose = pgettext("europeans_ware", "These advanced shields are used by the best soldiers of the Europeans. They are produced in the armor smithy and used in the big trainingscamp – together with food – to train soldiers from defense level 3 to level 6.")
            }
         }
       }
   },
   -- Workers positions in workers windows.
   -- This also gives us the information which workers the tribe uses.
   -- Each subtable is a column in the workers windows.
   workers_order = {
      {
         -- Basic Workers
         {
            name = "europeans_carrier",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Carrier
               purpose = pgettext("europeans_worker", "Carries items along your roads.")
            }
         },
         {
            name = "europeans_ferry",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Ferry
               purpose = pgettext("europeans_worker", "Ships wares across narrow rivers.")
            }
         },
         {
            name = "europeans_shipwright",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Shipwright
               purpose = pgettext("europeans_worker", "Works at the shipyard and constructs new ships.")
            }
         },
         {
            name = "europeans_builder",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Builder
               purpose = pgettext("europeans_worker", "Works at construction sites to raise new buildings.")
            }
         },
         {
            name = "europeans_geologist",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Geologist
               purpose = pgettext("europeans_worker", "Discovers resources for mining.")
            }
         },
         {
            name = "europeans_scout",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Scout
               purpose = pgettext("europeans_worker", "Scouts like Scotty the scout scouting unscouted areas in a scouty fashion.")
               -- (c) WiHack Team 02.01.2010
            }
         },
         {
            name = "europeans_recruit",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Recruit
               purpose = pgettext("europeans_worker", "Eager to become a soldier and defend his tribe!")
            }
         },
         {
            name = "europeans_trainer",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Trainer
               purpose = pgettext("europeans_worker", "Trains the soldiers.")
            }
         },
         {
            name = "europeans_soldier",
            default_target_quantity = 16,
            preciousness = 4,
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Soldier
               purpose = pgettext("europeans_worker", "Defend and Conquer!")
            }
         }
      },
      {
         -- Collectors, Supporters and Producer (Wood)
         {
            name = "europeans_lumberjack_basic",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Lumberjack / Woodcutter
               purpose = pgettext("europeans_worker", "Fells trees.")
            }
         },
         {
            name = "europeans_lumberjack_normal",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Lumberjack / Woodcutter
               purpose = pgettext("europeans_worker", "Fells trees.")
            }
         },
         {
            name = "europeans_lumberjack_advanced",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Lumberjack / Woodcutter
               purpose = pgettext("europeans_worker", "Fells trees.")
            }
         },
         {
            name = "europeans_forester_basic",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Ranger / Forester
               purpose = pgettext("europeans_worker", "Plant trees.")
            }
         },
         {
            name = "europeans_forester_normal",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Ranger / Forester
               purpose = pgettext("europeans_worker", "Plant trees.")
            }
         },
         {
            name = "europeans_forester_advanced",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Ranger / Forester
               purpose = pgettext("europeans_worker", "Plant trees.")
            }
         },
         {
            name = "europeans_carpenter_basic",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Carpenter / Sawyer
               purpose = pgettext("europeans_worker", "Produces planks out of log.")
            }
         },
         {
            name = "europeans_carpenter_normal",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Carpenter / Sawyer
               purpose = pgettext("europeans_worker", "Produces planks out of log.")
            }
         },
         {
            name = "europeans_carpenter_advanced",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Carpenter / Sawyer
               purpose = pgettext("europeans_worker", "Produces planks out of log.")
            }
         }
      },
      {
         -- Collectors and Producer (Clay and Stone)
         {
            name = "europeans_charcoal_burner_basic",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Charcoal Burner
               purpose = pgettext("europeans_worker", "Produces charcoal out of log.")
            }
         },
         {
            name = "europeans_charcoal_burner_normal",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Charcoal Burner
               purpose = pgettext("europeans_worker", "Produces charcoal out of log.")
            }
         },
         {
            name = "europeans_charcoal_burner_advanced",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Charcoal Burner
               purpose = pgettext("europeans_worker", "Produces charcoal out of log or old planks.")
            }
         },
         {
            name = "europeans_stonecutter_basic",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Stonecutter
               purpose = pgettext("europeans_worker", "Cuts blocks of granite and marble out of rocks in the vicinity.")
            }
         },
         {
            name = "europeans_stonecutter_normal",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Stonecutter
               purpose = pgettext("europeans_worker", "Cuts blocks of granite and marble out of rocks in the vicinity.")
            }
         },
         {
            name = "europeans_stonecutter_advanced",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Stonecutter
               purpose = pgettext("europeans_worker", "Cuts blocks of granite and marble out of rocks in the vicinity.")
            }
         },
         {
            name = "europeans_stonemason_basic",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Stonemason
               purpose = pgettext("europeans_worker", "Burns bricks and grout out of clay, water and granite.")
            }
         },
         {
            name = "europeans_stonemason_normal",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Stonemason
               purpose = pgettext("europeans_worker", "Burns bricks and grout out of clay, water and granite.")
            }
         },
         {
            name = "europeans_stonemason_advanced",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Stonemason
               purpose = pgettext("europeans_worker", "Burns bricks and grout out of clay, water and granite. Makes marble column out of marble.")
            }
         }
      },
      {
         -- Collectors, Supporters and Producer (Cloth)
         {
            name = "europeans_farmer_basic",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Farmer
               purpose = pgettext("europeans_worker", "Plants and harvests fields.")
            }
         },
         {
            name = "europeans_farmer_normal",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Farmer
               purpose = pgettext("europeans_worker", "Plants and harvests fields.")
            }
         },
         {
            name = "europeans_farmer_advanced",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Farmer
               purpose = pgettext("europeans_worker", "Plants and harvests fields.")
            }
         },
         {
            name = "europeans_breeder_basic",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Breeder
               purpose = pgettext("europeans_worker", "Breeds spiders and sheeps and produce silk and wool.")
            }
         },
         {
            name = "europeans_breeder_normal",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Breeder
               purpose = pgettext("europeans_worker", "Breeds spiders and sheeps and produce silk and wool.")
            }
         },
         {
            name = "europeans_breeder_advanced",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Breeder
               purpose = pgettext("europeans_worker", "Breeds spiders and sheeps and produce silk and wool.")
            }
         },
         {
            name = "europeans_weaver_basic",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Weaver
               purpose = pgettext("europeans_worker", "Produces cloth for ships, tools and armor.")
            }
         },
         {
            name = "europeans_weaver_normal",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Weaver
               purpose = pgettext("europeans_worker", "Produces cloth for ships, tools and armor.")
            }
         },
         {
            name = "europeans_weaver_advanced",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Weaver
               purpose = pgettext("europeans_worker", "Produces cloth for ships, tools and armor.")
            }
         }
      },
      {
         -- Collectors and Supporters (Food)
         {
            name = "europeans_fisher_basic",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Carrier
               purpose = pgettext("europeans_worker", "Catches fish in the sea or in pond.")
            }
         },
         {
            name = "europeans_fisher_normal",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Carrier
               purpose = pgettext("europeans_worker", "Catches fish in the sea or in pond.")
            }
         },
         {
            name = "europeans_fisher_advanced",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Carrier
               purpose = pgettext("europeans_worker", "Catches fish in the sea or in pond.")
            }
         },
         {
            name = "europeans_hunter_basic",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Carrier
               purpose = pgettext("europeans_worker", "The hunter brings fresh, raw meat to the colonists.")
            }
         },
         {
            name = "europeans_hunter_normal",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Carrier
               purpose = pgettext("europeans_worker", "The hunter brings fresh, raw meat to the colonists.")
            }
         },
         {
            name = "europeans_hunter_advanced",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Carrier
               purpose = pgettext("europeans_worker", "The hunter brings fresh, raw meat to the colonists.")
            }
         },
         {
            name = "europeans_fishbreeder",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Carrier
               purpose = pgettext("europeans_worker", "Breeds fish in pond and in the sea.")
            }
         },
         {
            name = "europeans_gamekeeper",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Carrier
               purpose = pgettext("europeans_worker", "The gamekeeper makes sure that the natural population of animals never drops too low.")
            }
         },
         {
            name = "europeans_beekeeper",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Carrier
               purpose = pgettext("europeans_worker", "Lets bees swarm over flowers, then gathers the honey.")
            }
         }
      },
      {
         -- Producer (Food)
         {
            name = "europeans_miller_basic",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Miller
               purpose = pgettext("europeans_worker", "Grind rye and wheat to mixed flour and corn to cornmeal")
            }
         },
         {
            name = "europeans_miller_normal",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Miller
               purpose = pgettext("europeans_worker", "Grind rye and wheat to mixed flour and corn to cornmeal")
            }
         },
         {
            name = "europeans_miller_advanced",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Miller
               purpose = pgettext("europeans_worker", "Grind rye and wheat to mixed flour and corn to cornmeal" )
            }
         },
         {
            name = "europeans_baker_basic",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Baker
               purpose = pgettext("europeans_worker", "Produce bread out of water, flour and cornmeal.")
            }
         },
         {
            name = "europeans_baker_normal",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Baker
               purpose = pgettext("europeans_worker", "Produce bread out of water, flour and cornmeal.")
            }
         },
         {
            name = "europeans_baker_advanced",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Baker
               purpose = pgettext("europeans_worker", "Produce bread out of water, flour and cornmeal.")
            }
         },
         {
            name = "europeans_brewer_basic",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Brewer
               purpose = pgettext("europeans_worker", "Brewing beer and mead.")
            }
         },
         {
            name = "europeans_brewer_normal",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Brewer
               purpose = pgettext("europeans_worker", "Brewing beer and mead.")
            }
         },
         {
            name = "europeans_brewer_advanced",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Brewer
               purpose = pgettext("europeans_worker", "Brewing beer and mead. Produces wine.")
            }
         },
         {
            name = "europeans_smoker_basic",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Smoker
               purpose = pgettext("europeans_worker", "Smoking fish and meat.")
            }
         },
         {
            name = "europeans_smoker_normal",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Smoker
               purpose = pgettext("europeans_worker", "Smoking fish and meat.")
            }
         },
         {
            name = "europeans_smoker_advanced",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Smoker
               purpose = pgettext("europeans_worker", "Smoking fish and meat.")
            }
         }
      },
      {
         -- Miners, Smelters and Smiths
         {
            name = "europeans_miner_basic",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Miner
               purpose = pgettext("europeans_worker", "Mines ore and minerals.")
            }
         },
         {
            name = "europeans_miner_normal",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Miner
               purpose = pgettext("europeans_worker", "Mines ore and minerals.")
            }
         },
         {
            name = "europeans_miner_advanced",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Miner
               purpose = pgettext("europeans_worker", "Mines ore and minerals.")
            }
         },
         {
            name = "europeans_smelter_basic",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Smelter
               purpose = pgettext("europeans_worker", "Smelts ore to metals.")
            }
         },
         {
            name = "europeans_smelter_normal",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Smelter
               purpose = pgettext("europeans_worker", "Smelts ore to metals.")
            }
         },
         {
            name = "europeans_smelter_advanced",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Smelter
               purpose = pgettext("europeans_worker", "Smelts ore to metals.")
            }
         },
         {
            name = "europeans_smith_basic",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Smith
               purpose = pgettext("europeans_worker", "Produce tools, armor and weapons.")
            }
         },
         {
            name = "europeans_smith_normal",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Smith
               purpose = pgettext("europeans_worker", "Produce tools, armor and weapons.")
            }
         },
         {
            name = "europeans_smith_advanced",
            helptexts = {
               -- TRANSLATORS: Helptext for an europeans worker: Smith
               purpose = pgettext("europeans_worker", "Produce tools, armor and weapons.")
            }
         }
      }
   },

   immovables = {
      {
         name = "ashes",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: Ashes
            purpose = _("The remains of a destroyed building.")
         }
      },
      {
         name = "destroyed_building",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: Destroyed Building
            purpose = _("The remains of a destroyed building.")
         }
      },
      {
         name = "barleyfield_tiny",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: barley field
            purpose = _("This field has just been planted.")
         }
      },
      {
         name = "barleyfield_small",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: barley field
            purpose = _("This field is growing.")
         }
      },
      {
         name = "barleyfield_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: barley field
            purpose = _("This field is growing.")
         }
      },
      {
         name = "barleyfield_ripe",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: barley field
            purpose = _("This field is ready for harvesting.")
         }
      },
      {
         name = "barleyfield_harvested",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: barley field
            purpose = _("This field has been harvested.")
         }
      },
      {
         name = "berry_bush_blueberry_tiny",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush has just been planted.")
         }
      },
      {
         name = "berry_bush_blueberry_small",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush is growing.")
         }
      },
      {
         name = "berry_bush_blueberry_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush is flowering. Honey can be produced from it by a beekeeper.")
         }
      },
      {
         name = "berry_bush_blueberry_ripe",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush is ready for harvesting.")
         }
      },
      {
         name = "berry_bush_currant_red_tiny",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush has just been planted.")
         }
      },
      {
         name = "berry_bush_currant_red_small",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush is growing.")
         }
      },
      {
         name = "berry_bush_currant_red_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush is flowering. Honey can be produced from it by a beekeeper.")
         }
      },
      {
         name = "berry_bush_currant_red_ripe",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush is ready for harvesting.")
         }
      },
      {
         name = "berry_bush_juniper_tiny",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush has just been planted.")
         }
      },
      {
         name = "berry_bush_juniper_small",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush is growing.")
         }
      },
      {
         name = "berry_bush_juniper_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush is flowering. Honey can be produced from it by a beekeeper.")
         }
      },
      {
         name = "berry_bush_juniper_ripe",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush is ready for harvesting.")
         }
      },
      {
         name = "berry_bush_raspberry_tiny",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush has just been planted.")
         }
      },
      {
         name = "berry_bush_raspberry_small",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush is growing.")
         }
      },
      {
         name = "berry_bush_raspberry_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush is flowering. Honey can be produced from it by a beekeeper.")
         }
      },
      {
         name = "berry_bush_raspberry_ripe",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush is ready for harvesting.")
         }
      },
      {
         name = "berry_bush_currant_black_tiny",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush has just been planted.")
         }
      },
      {
         name = "berry_bush_currant_black_small",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush is growing.")
         }
      },
      {
         name = "berry_bush_currant_black_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush is flowering. Honey can be produced from it by a beekeeper.")
         }
      },
      {
         name = "berry_bush_currant_black_ripe",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush is ready for harvesting.")
         }
      },
      {
         name = "berry_bush_strawberry_tiny",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush has just been planted.")
         }
      },
      {
         name = "berry_bush_strawberry_small",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush is growing.")
         }
      },
      {
         name = "berry_bush_strawberry_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush is flowering. Honey can be produced from it by a beekeeper.")
         }
      },
      {
         name = "berry_bush_strawberry_ripe",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush is ready for harvesting.")
         }
      },
      {
         name = "berry_bush_stink_tree_tiny",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush has just been planted.")
         }
      },
      {
         name = "berry_bush_stink_tree_small",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush is growing.")
         }
      },
      {
         name = "berry_bush_stink_tree_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush is flowering. Honey can be produced from it by a beekeeper.")
         }
      },
      {
         name = "berry_bush_stink_tree_ripe",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush is ready for harvesting.")
         }
      },
      {
         name = "berry_bush_desert_hackberry_tiny",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush has just been planted.")
         }
      },
      {
         name = "berry_bush_desert_hackberry_small",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush is growing.")
         }
      },
      {
         name = "berry_bush_desert_hackberry_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush is flowering. Honey can be produced from it by a beekeeper.")
         }
      },
      {
         name = "berry_bush_desert_hackberry_ripe",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush is ready for harvesting.")
         }
      },
      {
         name = "berry_bush_sea_buckthorn_tiny",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush has just been planted.")
         }
      },
      {
         name = "berry_bush_sea_buckthorn_small",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush is growing.")
         }
      },
      {
         name = "berry_bush_sea_buckthorn_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush is flowering. Honey can be produced from it by a beekeeper.")
         }
      },
      {
         name = "berry_bush_sea_buckthorn_ripe",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: One of the berry bushes
            purpose = _("This berry bush is ready for harvesting.")
         }
      },
      {
         name = "blackrootfield_tiny",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: blackroot field
            purpose = _("This field has just been planted.")
         }
      },
      {
         name = "blackrootfield_small",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: blackroot field
            purpose = _("This field is growing.")
         }
      },
      {
         name = "blackrootfield_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: blackroot field
            purpose = _("This field is growing.")
         }
      },
      {
         name = "blackrootfield_ripe",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: blackroot field
            purpose = _("This field is ready for harvesting.")
         }
      },
      {
         name = "blackrootfield_harvested",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: blackroot field
            purpose = _("This field has been harvested.")
         }
      },
      {
         name = "cornfield_tiny",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: corn field
            purpose = _("This field has just been planted.")
         }
      },
      {
         name = "cornfield_small",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: corn field
            purpose = _("This field is growing.")
         }
      },
      {
         name = "cornfield_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: corn field
            purpose = _("This field is growing.")
         }
      },
      {
         name = "cornfield_ripe",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: corn field
            purpose = _("This field is ready for harvesting.")
         }
      },
      {
         name = "cornfield_harvested",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: corn field
            purpose = _("This field has been harvested.")
         }
      },
      {
         name = "pond_dry",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: Pond
            purpose = _("When claydiggers dig up earth, they leave holes in the ground. These holes vanish after a while. Aqua farms can use them as ponds to grow fish in them, whereas charcoal burners erect their charcoal stacks in them.")
         }
      },
      {
         name = "pond_growing",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: Pond
            purpose = _("Fish are maturing in this pond. A fisher working from an aqua farm will be able to catch them when they are bigger.")
         }
      },
      {
         name = "pond_mature",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: Pond
            purpose = _("Fish are living in this pond. A fisher working from an aqua farm can catch them as food.")
         }
      },
      {
         name = "pond_burning",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: Pond
            purpose = _("A charcoal stack is burning in this earthen hole. When it has burnt down, a charcoal burner will be able to gather coal from it.")
         }
      },
      {
         name = "pond_coal",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: Pond
            purpose = _("A charcoal stack, which had been erected in this earthen hole, is ready for a charcoal burner to gather coal from it.")
         }
      },
      {
         name = "reedfield_tiny",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: Reed Field
            purpose = _("This reed field has just been planted.")
         }
      },
      {
         name = "reedfield_small",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: Reed Field
            purpose = _("This reed field is growing.")
         }
      },
      {
         name = "reedfield_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: Reed Field
            purpose = _("This reed field is flowering. Honey can be produced from it by a beekeeper.")
         }
      },
      {
         name = "reedfield_ripe",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: Reed Field
            purpose = _("This reed field is ready for harvesting.")
         }
      },
      {
         name = "ryefield_tiny",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: rye field
            purpose = _("This field has just been planted.")
         }
      },
      {
         name = "ryefield_small",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: rye field
            purpose = _("This field is growing.")
         }
      },
      {
         name = "ryefield_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: rye field
            purpose = _("This field is growing.")
         }
      },
      {
         name = "ryefield_ripe",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: rye field
            purpose = _("This field is ready for harvesting.")
         }
      },
      {
         name = "ryefield_harvested",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: rye field
            purpose = _("This field has been harvested.")
         }
      },
      {
         name = "wheatfield_tiny",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: Wheat field
            purpose = _("This field has just been planted.")
         }
      },
      {
         name = "wheatfield_small",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: Wheat field
            purpose = _("This field is growing.")
         }
      },
      {
         name = "wheatfield_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: Wheat field
            purpose = _("This field is growing.")
         }
      },
      {
         name = "wheatfield_ripe",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: Wheat field
            purpose = _("This field is ready for harvesting.")
         }
      },
      {
         name = "wheatfield_harvested",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: Wheat field
            purpose = _("This field has been harvested.")
         }
      },
      {
         name = "grapevine_tiny",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: Grapevine
            purpose = _("This grapevine has just been planted.")
         }
      },
      {
         name = "grapevine_small",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: Grapevine
            purpose = _("This grapevine is growing.")
         }
      },
      {
         name = "grapevine_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: Grapevine
            purpose = _("This grapevine is growing.")
         }
      },
      {
         name = "grapevine_ripe",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: Grapevine
            purpose = _("This grapevine is ready for harvesting.")
         }
      },
      {
         name = "europeans_resi_none",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans resource indicator: No resources
            purpose = _("There are no resources in the ground here.")
         }
      },
      {
         name = "europeans_resi_water",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans resource indicator: Water
            purpose = _("There is water in the ground here that can be pulled up by a well.")
         }
      },
      {
         name = "europeans_resi_coal_1",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an europeans resource indicator: Coal, part 1
               _("Coal veins contain coal that can be dug up by coal mines."),
               -- TRANSLATORS: Helptext for an europeans resource indicator: Coal, part 2
               _("There is only a little bit of coal here.")
            }
         }
      },
      {
         name = "europeans_resi_iron_1",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an europeans resource indicator: Iron, part 1
               _("Iron veins contain iron ore that can be dug up by iron mines."),
               -- TRANSLATORS: Helptext for an europeans resource indicator: Iron, part 2
               _("There is only a little bit of iron here.")
            }
         }
      },
      {
         name = "europeans_resi_gold_1",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an europeans resource indicator: Gold, part 1
               _("Gold veins contain gold ore that can be dug up by gold mines."),
               -- TRANSLATORS: Helptext for an europeans resource indicator: Gold, part 2
               _("There is only a little bit of gold here.")
            }
         }
      },
      {
         name = "europeans_resi_stones_1",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an europeans resource indicator: Stones, part 1
               _("Marble is a basic building material and can be dug up by a marble mine. You will also get granite from the mine."),
               -- TRANSLATORS: Helptext for an europeans resource indicator: Stones, part 2
               _("There is only a little bit of marble here.")
            }
         }
      },
      {
         name = "europeans_resi_coal_2",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an europeans resource indicator: Coal, part 1
               _("Coal veins contain coal that can be dug up by coal mines."),
               -- TRANSLATORS: Helptext for an europeans resource indicator: Coal, part 2
               _("There is a lot of coal here.")
            }
         }
      },
      {
         name = "europeans_resi_iron_2",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an europeans resource indicator: Iron, part 1
               _("Iron veins contain iron ore that can be dug up by iron mines."),
               -- TRANSLATORS: Helptext for an europeans resource indicator: Iron, part 2
               _("There is a lot of iron here.")
            }
         }
      },
      {
         name = "europeans_resi_gold_2",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an europeans resource indicator: Gold, part 1
               _("Gold veins contain gold ore that can be dug up by gold mines."),
               -- TRANSLATORS: Helptext for an europeans resource indicator: Gold, part 2
               _("There is a lot of gold here.")
            }
         }
      },
      {
         name = "europeans_resi_stones_2",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an europeans resource indicator: Stones, part 1
               _("Marble is a basic building material and can be dug up by a marble mine. You will also get granite from the mine."),
               -- TRANSLATORS: Helptext for an europeans resource indicator: Stones, part 2
               _("There is a lot of marble here.")
            }
         }
      },
      {
         name = "europeans_shipconstruction",
         helptexts = {
            -- TRANSLATORS: Helptext for an europeans immovable: Ship Under Construction
            purpose = _("A ship is being constructed at this site.")
         }
      },
      -- non imperial Immovables used by the woodcutter
      {
         name = "deadtree7",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by europeans: Dead Tree
            purpose = _("The remains of an old tree.")
         }
      },
      {
         name = "balsa_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by europeans: Balsa Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_black_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by europeans: Balsa Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_desert_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by europeans: Balsa Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_winter_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by europeans: Balsa Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by europeans: Ironwood Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_black_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by europeans: Ironwood Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_desert_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by europeans: Ironwood Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_winter_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by europeans: Ironwood Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by europeans: Rubber Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_black_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by europeans: Rubber Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_desert_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by europeans: Corn Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_winter_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by europeans: Rubber Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
   },

   -- The order here also determines the order in lists on screen.
   buildings = {
      -- Warehouses
      {
         name = "europeans_headquarters",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans warehouse: Headquarters
            purpose = pgettext("europeans_building", "Accommodation for your people. Also stores your wares and tools."),
            -- TRANSLATORS: Note helptext for an europeans warehouse: Headquarters
            note = pgettext("europeans_building", "The headquarters is your main building.")
         }
      },
      {
         name = "europeans_warehouse",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans warehouse: Warehouse
            purpose = pgettext("europeans_building", "Your workers and soldiers will find shelter here. Also stores your wares and tools.")
         }
      },
      {
         name = "europeans_port",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans warehouse: Port
            purpose = pgettext("europeans_building", "Serves as a base for overseas colonization and trade. Also stores your soldiers, wares and tools.")
         }
      },

      -- Small
      {
         name = "europeans_well_basic",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Well
            purpose = pgettext("europeans_building", "Draws water out of the deep.")
         }
      },
      {
         name = "europeans_well_normal",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Well
            purpose = pgettext("europeans_building", "Draws water out of the deep.")
         }
      },
      {
         name = "europeans_well_advanced",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Well
            purpose = pgettext("europeans_building", "Draws water out of the deep.")
         }
      },
      {
         name = "europeans_lumberjacks_house_basic",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a europeans production site: Lumberjack_s Hut
            purpose = pgettext("europeans_building", "Fells trees in the surrounding area and processes them into logs."),
            -- TRANSLATORS: Note helptext for a europeans production site: Lumberjack_s Hut
            note = pgettext("europeans_building", "The lumberjack’s hut needs trees to fell within the work area."),
         }
      },
      {
         name = "europeans_lumberjacks_house_normal",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Woodcutter's House
            purpose = pgettext("europeans_building", "Fells trees in the surrounding area and processes them into logs."),
            -- TRANSLATORS: Note helptext for an europeans production site: Woodcutter's House
            note = pgettext("europeans_building", "The woodcutter’s house needs trees to fell within the work area.")
         }
      },
      {
         name = "europeans_lumberjacks_house_advanced",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Lumberjack's House
            purpose = pgettext("europeans_building", "Fells trees in the surrounding area and processes them into logs."),
            -- TRANSLATORS: Note helptext for an europeans production site: Lumberjack's House
            note = pgettext("europeans_building", "The lumberjack’s house needs trees to fell within the work area.")
         }
      },
      {
         name = "europeans_foresters_house_basic",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a europeans production site: Ranger's Hut
            purpose = pgettext("europeans_building", "Plants trees in the surrounding area."),
            -- TRANSLATORS: Note helptext for a europeans production site: Ranger's Hut
            note = pgettext("europeans_building", "The ranger’s hut needs free space within the work area to plant the trees."),
         }
      },
      {
         name = "europeans_foresters_house_normal",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Forester's House
            purpose = pgettext("europeans_building", "Plants trees in the surrounding area."),
            -- TRANSLATORS: Note helptext for an europeans production site: Forester's House
            note = pgettext("europeans_building", "The forester’s house needs free space within the work area to plant the trees.")
         }
      },
      {
         name = "europeans_foresters_house_advanced",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Forester's House
            purpose = pgettext("europeans_building", "Plants trees in the surrounding area."),
            -- TRANSLATORS: Note helptext for an europeans production site: Forester's House
            note = pgettext("europeans_building", "The forester’s house needs free space within the work area to plant the trees.")
         }
      },
      {
         name = "europeans_quarry_basic",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a europeans production site: Quarry
            purpose = pgettext("europeans_building", "Cuts raw pieces of granite out of rocks in the vicinity."),
            -- TRANSLATORS: Note helptext for a europeans production site: Quarry
            note = pgettext("europeans_building", "The quarry needs rocks to cut within the work area."),
         }
      },
      {
         name = "europeans_quarry_normal",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Quarry
            purpose = pgettext("europeans_building", "Cuts blocks of granite out of rocks in the vicinity."),
            -- TRANSLATORS: Note helptext for an europeans production site: Quarry
            note = pgettext("europeans_building", "The quarry needs rocks to cut within the work area.")
         }
      },
      {
         name = "europeans_quarry_advanced",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Quarry
            purpose = pgettext("europeans_building", "Cuts blocks of granite and marble out of rocks in the vicinity."),
            -- TRANSLATORS: Note helptext for an europeans production site: Quarry
            note = pgettext("europeans_building", "The quarry needs rocks to cut within the work area.")
         }
      },
      {
         name = "europeans_clay_pit",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a europeans production site: Clay Pit
            purpose = pgettext("europeans_building", "Digs up mud from the ground and uses water to turn it into clay. Clay is used to make bricks.")
         }
      },
      {
         name = "europeans_fishers_house_basic",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for production site: Fisher's Hut
            purpose = pgettext("europeans_building", "Fishes on the coast near the fisher’s hut."),
            -- TRANSLATORS: Note helptext for production site: Fisher's Hut
            note = pgettext("europeans_building", "The fisher’s hut needs water full of fish within the work area."),
         }
      },
      {
         name = "europeans_fishers_house_normal",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Fisher's House
            purpose = pgettext("europeans_building", "Fishes on the coast near the fisher’s house."),
            note = {
               -- TRANSLATORS: Note helptext for an europeans production site: Fisher's House, part 1
               pgettext("europeans_building", "The fisher’s house needs water full of fish within the work area."),
               -- TRANSLATORS: Note helptext for an europeans production site: Fisher's House, part 2
               pgettext("europeans_building", "Build a fish breeder’s house close to the fisher’s house to make sure that you don’t run out of fish.")
            }
         }
      },
      {
         name = "europeans_fishers_house_advanced",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Fisher's House
            purpose = pgettext("europeans_building", "Fishes on the coast near the fisher’s house."),
            -- TRANSLATORS: Note helptext for an europeans production site: Fisher's House
            note = pgettext("europeans_building", "The fisher’s house needs water full of fish within the work area.")
         }
      },
      {
         name = "europeans_hunters_house_basic",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a europeans production site: Hunter's Hut
            purpose = pgettext("europeans_building", "Hunts animals to produce meat."),
            -- TRANSLATORS: Note helptext for a europeans production site: Hunter's Hut
            note = pgettext("europeans_building", "The hunter’s hut needs animals to hunt within the work area."),
         }
      },
      {
         name = "europeans_hunters_house_normal",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Hunter's House
            purpose = pgettext("europeans_building", "Hunts animals to produce meat."),
            -- TRANSLATORS: Note helptext for an europeans production site: Hunter's House
            note = pgettext("europeans_building", "The hunter’s house needs animals to hunt within the work area.")
         }
      },
      {
         name = "europeans_hunters_house_advanced",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Hunter's House
            purpose = pgettext("europeans_building", "Hunts animals to produce meat."),
            -- TRANSLATORS: Note helptext for an europeans production site: Hunter's House
            note = pgettext("europeans_building", "The hunter’s house needs animals to hunt within the work area.")
         }
      },
      {
         name = "europeans_gamekeepers_hut",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for production site: Gamekeeper's Hut
            purpose = pgettext("europeans_building", "Releases animals into the wild to steady the meat production."),
         }
      },
      {
         name = "europeans_beekeepers_house",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Beekeeper's House
            purpose = pgettext("europeans_building", "Keeps bees and lets them swarm over flowering fields to produce honey."),
            -- TRANSLATORS: Note helptext for a frisian production site: Beekeeper's House
            note = pgettext("europeans_building", "Needs medium-sized fields (barley, rye, wheat, reed, corn or blackroot) or bushes (berry bushes or grapevines) nearby."),
         }
      },

      -- Medium
      {
         name = "europeans_reed_yard",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Reed Yard
            purpose = pgettext("europeans_building", "Cultivates reed that serves two different purposes for the Europeans tribe."),
            -- TRANSLATORS: Note helptext for a barbarian production site: Reed Yard
            note = pgettext("europeans_building", "Reed is the traditional material for roofing, and it is woven into the extremely durable rope that they use for their ships’ sails."),
         }
      },
      {
         name = "europeans_sawmill_basic",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Sawmill
            purpose = pgettext("europeans_building", "Saws logs to produce planks.")
         }
      },
      {
         name = "europeans_sawmill_normal",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Sawmill
            purpose = pgettext("europeans_building", "Saws logs to produce planks.")
         }
      },
      {
         name = "europeans_sawmill_advanced",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Sawmill
            purpose = pgettext("europeans_building", "Saws logs to produce planks.")
         }
      },
      {
         name = "europeans_charcoal_kiln_basic",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a europeans production site: Charcoal Kiln
            purpose = pgettext("europeans_building", "Burns logs into charcoal.")
         }
      },
      {
         name = "europeans_charcoal_kiln_normal",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Charcoal Kiln
            purpose = pgettext("europeans_building", "Burns logs into charcoal.")
         }
      },
      {
         name = "europeans_charcoal_kiln_advanced",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Charcoal Kiln
            purpose = pgettext("europeans_building", "Burns logs into charcoal.")
         }
      },
      {
         name = "europeans_lime_kiln",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a europeans production site: Lime Kiln
            purpose = pgettext("europeans_building", "Processes clay and granite to make bricks and ‘grout’, a substance that solidifies and so reinforces masonry."),
            -- TRANSLATORS: Note helptext for a europeans production site: Lime Kiln
            note = pgettext("europeans_building", "The lime kiln’s output will only go to construction sites that need it. Those are predominantly houses that work with fire, and some military sites."),
         }
      },
      {
         name = "europeans_stonemasons_house",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Stonemason's House
            purpose = pgettext("europeans_building", "Carves marble columns out of marble.")
         }
      },
      {
         name = "europeans_mill_basic",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Mill
            purpose = pgettext("europeans_building", "Grinds corn to produce cornmeal.")
         }
      },
      {
         name = "europeans_mill_normal",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Mill
            purpose = pgettext("europeans_building", "Grinds corn to produce cornmeal. Grinds wheat and rye to produce mixed flour.")
         }
      },
      {
         name = "europeans_mill_advanced",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Mill
            purpose = pgettext("europeans_building", "Grinds corn to produce cornmeal. Grinds wheat and rye to produce mixed flour.")
         }
      },
      {
         name = "europeans_tavern_basic",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Tavern
            purpose = pgettext("building", "Prepares rations to feed the scouts and miners.")
         }
      },
      {
         name = "europeans_tavern_level_1",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Tavern
            purpose = pgettext("building", "Prepares rations to feed the scouts and miners.")
         }
      },
      {
         name = "europeans_tavern_level_2",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Tavern
            purpose = pgettext("building", "Prepares rations to feed the scouts and miners.")
         }
      },
      {
         name = "europeans_tavern_level_3",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Inn
            purpose = pgettext("building", "Prepares rations and snacks to feed the miners in the basic and deep mines.")
         }
      },
      {
         name = "europeans_tavern_level_4",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Inn
            purpose = pgettext("building", "Prepares rations and snacks to feed the miners in the basic and deep mines.")
         }
      },
      {
         name = "europeans_tavern_level_5",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Inn
            purpose = pgettext("building", "Prepares rations and snacks to feed the miners in the basic and deep mines.")
         }
      },
      {
         name = "europeans_inn_basic",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Big Inn
            purpose = pgettext("europeans_building", "Prepares rations, snacks and meals to feed all miners.")
         }
      },
      {
         name = "europeans_inn_level_1",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Big Inn
            purpose = pgettext("europeans_building", "Prepares rations, snacks and meals to feed all miners.")
         }
      },
      {
         name = "europeans_inn_level_2",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Big Inn
            purpose = pgettext("europeans_building", "Prepares rations, snacks and meals to feed all miners.")
         }
      },
      {
         name = "europeans_brewery_basic",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a europeans production site: Brewery
            purpose = pgettext("europeans_building", "Produces beer to keep the miners strong and happy.")
         }
      },
      {
         name = "europeans_brewery_normal",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a europeans production site: Brewery
            purpose = pgettext("europeans_building", "Produces beer and mead to keep the miners strong and happy.")
         }
      },
      {
         name = "europeans_brewery_advanced",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Brewery
            purpose = pgettext("europeans_building", "Produces beer, mead and strong_beer for trainingssites."),
         }
      },
      {
         name = "europeans_brewery_winery",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Winery
            purpose = pgettext("europeans_building", "Produces mead and wine.")
         }
      },
      {
         name = "europeans_smelting_works_basic",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a europeans production site: Smelting Works
            purpose = pgettext("europeans_building", "Smelts ore into iron.")
         }
      },
      {
         name = "europeans_smelting_works_normal",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Smelting Works
            purpose = pgettext("europeans_building", "Smelts ore into iron and gold.")
         }
      },
      {
         name = "europeans_smelting_works_advanced",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Smelting Works
            purpose = pgettext("europeans_building", "Smelts ore into iron and gold. Recycles scrap metal.")
         }
      },
      {
         name = "europeans_smithy_basic",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Toolsmithy
            purpose = pgettext("europeans_building", "Forges all the tools that your workers need.")
         }
      },
      {
         name = "europeans_smithy_level_1",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Toolsmithy
            purpose = pgettext("europeans_building", "Forges all the tools that your workers need.")
         }
      },
      {
         name = "europeans_smithy_level_2",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Toolsmithy
            purpose = pgettext("europeans_building", "Forges all the tools that your workers need.")
         }
      },
      {
         name = "europeans_smithy_level_3",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Toolsmithy
            purpose = pgettext("europeans_building", "Forges all the tools that your workers need.")
         }
      },
      {
         name = "europeans_smithy_level_4",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Toolsmithy
            purpose = pgettext("europeans_building", "Forges all the tools that your workers need. Produces armor and weapons, too.")
         }
      },
      {
         name = "europeans_smithy_level_5",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Toolsmithy
            purpose = pgettext("europeans_building", "Forges all the tools that your workers need. Produces armor and weapons, too.")
         }
      },
      {
         name = "europeans_smithy_level_6",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Toolsmithy
            purpose = pgettext("europeans_building", "Forges all the tools that your workers need. Produces armor and weapons, too.")
         }
      },
      {
         name = "europeans_smithy_level_7",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Toolsmithy
            purpose = pgettext("europeans_building", "Forges all the tools that your workers need. Produces armor and weapons, too.")
         }
      },
      {
         name = "europeans_smithy_level_8",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Toolsmithy
            purpose = pgettext("europeans_building", "Forges all the tools that your workers need. Produces armor and weapons, too.")
         }
      },
      {
         name = "europeans_smithy_level_9",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Toolsmithy
            purpose = pgettext("europeans_building", "Forges all the tools that your workers need. Produces armor and weapons, too.")
         }
      },
      {
         name = "europeans_barracks",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Barracks
            purpose = pgettext("europeans_building", "Equips recruits and trains them as soldiers.")
         }
      },

      -- Big
      {
         name = "europeans_farm_basic",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for production site: Farm
            purpose = pgettext("europeans_building", "Sows and harvests corn, rye and wheat."),
         }
      },
      {
         name = "europeans_farm_level_1",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Farm
            purpose = pgettext("europeans_building", "Sows and harvests barley, corn, rye and wheat."),
         }
      },
      {
         name = "europeans_farm_level_2",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Blackroot Farm
            purpose = pgettext("europeans_building", "Sows and harvests barley, blackroot, corn, rye and wheat."),
         }
      },
      {
         name = "europeans_farm_level_3",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Blackroot Farm
            purpose = pgettext("europeans_building", "Sows and harvests barley, blackroot, corn, fruit, grape, rye and wheat."),
         }
      },
      {
         name = "europeans_spiderfarm",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Spider Farm
            purpose = pgettext("europeans_building", "Breeds spiders for silk.")
         }
      },
      {
         name = "europeans_sheepfarm",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Sheep Farm
            purpose = pgettext("europeans_building", "Keeps sheep for their wool.")
         }
      },
      {
         name = "europeans_weaving_mill_basic",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a europeans production site: Weaving Mill
            purpose = pgettext("europeans_building", "Weaves spider cloth out of spider silk.")
         }
      },
      {
         name = "europeans_weaving_mill_normal",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Weaving Mill
            purpose = pgettext("europeans_building", "Weaves spidercloth for buildings and ships’ sails, and tabards to equip and train the soldiers.")
         }
      },
      {
         name = "europeans_weaving_mill_advanced",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Weaving Mill
            purpose = pgettext("europeans_building", "Weaves tabard and armor out of wool and silk.")
         }
      },

      -- Mines
      {
         name = "europeans_coalmine_basic",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for production site: Coal Mine
            purpose = pgettext("europeans_building", "Digs coal out of the ground in mountain terrain.")
         }
      },
      {
         name = "europeans_coalmine_level_1",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for production site: Deep Coal Mine
            purpose = pgettext("europeans_building", "Digs coal out of the ground in mountain terrain.")
         }
      },
      {
         name = "europeans_coalmine_level_2",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for production site: Deeper Coal Mine
            purpose = pgettext("europeans_building", "Digs coal out of the ground in mountain terrain.")
         }
      },
      {
         name = "europeans_coalmine_level_3",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Coal Mine
            purpose = pgettext("europeans_building", "Digs coal out of the ground in mountain terrain.")
         }
      },
      {
         name = "europeans_coalmine_level_4",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Deep Coal Mine
            purpose = pgettext("europeans_building", "Digs coal out of the ground in mountain terrain.")
         }
      },
      {
         name = "europeans_coalmine_level_5",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Coal Mine
            purpose = pgettext("europeans_building", "Digs coal out of the ground in mountain terrain.")
         }
      },
      {
         name = "europeans_ironmine_basic",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a europeans production site: Iron Mine
            purpose = pgettext("europeans_building", "Digs iron ore out of the ground in mountain terrain.")
         }
      },
      {
         name = "europeans_ironmine_level_1",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a europeans production site: Deep Iron Mine
            purpose = pgettext("europeans_building", "Digs iron ore out of the ground in mountain terrain.")
         }
      },
      {
         name = "europeans_ironmine_level_2",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a europeans production site: Deeper Iron Mine
            purpose = pgettext("europeans_building", "Digs iron ore out of the ground in mountain terrain.")
         }
      },
      {
         name = "europeans_ironmine_level_3",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Iron Mine
            purpose = pgettext("europeans_building", "Digs iron ore out of the ground in mountain terrain.")
         }
      },
      {
         name = "europeans_ironmine_level_4",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Deep Iron Mine
            purpose = pgettext("europeans_building", "Digs iron ore out of the ground in mountain terrain.")
         }
      },
      {
         name = "europeans_ironmine_level_5",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Iron Mine
            purpose = pgettext("europeans_building", "Digs iron ore out of the ground in mountain terrain.")
         }
      },
      {
         name = "europeans_goldmine_basic",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for production site: Gold Mine
            purpose = pgettext("europeans_building", "Digs gold ore out of the ground in mountain terrain.")
         }
      },
      {
         name = "europeans_goldmine_level_1",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for production site: Deep Gold Mine
            purpose = pgettext("europeans_building", "Digs gold ore out of the ground in mountain terrain.")
         }
      },
      {
         name = "europeans_goldmine_level_2",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for production site: Deeper Gold Mine
            purpose = pgettext("europeans_building", "Digs gold ore out of the ground in mountain terrain.")
         }
      },
      {
         name = "europeans_goldmine_level_3",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Gold Mine
            purpose = pgettext("europeans_building", "Digs gold ore out of the ground in mountain terrain.")
         }
      },
      {
         name = "europeans_goldmine_level_4",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Deep Gold Mine
            purpose = pgettext("europeans_building", "Digs gold ore out of the ground in mountain terrain.")
         }
      },
      {
         name = "europeans_goldmine_level_5",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Gold Mine
            purpose = pgettext("europeans_building", "Digs gold ore out of the ground in mountain terrain.")
         }
      },

      -- Training Sites
      {
         name = "europeans_guardhall",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Purpose helptext for an europeans training site: Guardhall, part 1
               pgettext("europeans_building", "Trains soldiers in ‘Health’."),
               -- TRANSLATORS: Purpose helptext for an europeans training site: Guardhall, part 2
               pgettext("europeans_building", "Equips the soldiers with all necessary armor parts.")
            },
         }
      },
      {
         name = "europeans_dungeon",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Purpose helptext for an europeans training site: Dungeon, part 1
               pgettext("europeans_building", "Trains soldiers in ‘Defense’."),
               -- TRANSLATORS: Purpose helptext for an europeans training site: Dungeon, part 2
               pgettext("europeans_building", "Equips the soldiers with all necessary parts.")
            },
         }
      },
      {
         name = "europeans_trainingscamp_basic",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Purpose helptext for an europeans training site: Basic Training Camp, part 1
               pgettext("europeans_building", "Trains soldiers in ‘Attack’."),
               -- TRANSLATORS: Purpose helptext for an europeans training site: Basic Training Camp, part 2
               pgettext("europeans_building", "Equips the soldiers with all necessary weapons.")
            },
         }
      },
      {
         name = "europeans_trainingscamp_advanced",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Purpose helptext for an europeans training site: Advanced Training Camp, part 1
               pgettext("europeans_building", "Trains soldiers in ‘Attack’."),
               -- TRANSLATORS: Purpose helptext for an europeans training site: Advanced Training Camp, part 2
               pgettext("europeans_building", "Equips the soldiers with all necessary weapons.")
            },
         }
      },
      {
         name = "europeans_labyrinth",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Purpose helptext for an europeans training site: Labyrinth, part 1
               pgettext("europeans_building", "Trains soldiers in ‘Evade’."),
               -- TRANSLATORS: Purpose helptext for an europeans training site: Labyrinth, part 2
               pgettext("europeans_building", "‘Evade’ increases the soldier’s chance not to be hit by the enemy and so to remain totally unaffected.")
            },
         }
      },
      {
         name = "europeans_arena",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Purpose helptext for an europeans training site: Arena, part 1
               pgettext("europeans_building", "Trains soldiers in ‘Evade’, in ‘Health’ and in ‘Defense’."),
               -- TRANSLATORS: Purpose helptext for an europeans training site: Arena, part 2
               pgettext("europeans_building", "‘Evade’ increases the soldier’s chance not to be hit by the enemy and so to remain totally unaffected.")
            },
         }
      },
      {
         name = "europeans_colosseum",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Purpose helptext for an europeans training site: Colosseum, part 1
               pgettext("europeans_building", "Trains soldiers in ‘Evade’, in ‘Health’, in ‘Defense’ and in ‘Attack’."),
               -- TRANSLATORS: Purpose helptext for an europeans training site: Colosseum, part 2
               pgettext("europeans_building", "‘Evade’ increases the soldier’s chance not to be hit by the enemy and so to remain totally unaffected.")
            },
         }
      },
      {
         name = "europeans_trainingscamp_big",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Purpose helptext for an europeans training site: Training Camp, part 1
               pgettext("europeans_building", "Trains soldiers in ‘Attack’ and in ‘Defense’."),
               -- TRANSLATORS: Purpose helptext for an europeans training site: Training Camp, part 2
               pgettext("europeans_building", "Equips the soldiers with all necessary weapons and armor parts.")
            },
         }
      },

      -- Military Sites
      {
         name = "europeans_guardhouse",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans military site: Blockhouse
            purpose = pgettext("europeans_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an europeans military site: Blockhouse
            note = pgettext("europeans_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "europeans_tower_small",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans military site: Blockhouse
            purpose = pgettext("europeans_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an europeans military site: Blockhouse
            note = pgettext("europeans_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "europeans_blockhouse",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans military site: Blockhouse
            purpose = pgettext("europeans_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an europeans military site: Blockhouse
            note = pgettext("europeans_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "europeans_sentry",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans military site: Sentry
            purpose = pgettext("europeans_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an europeans military site: Sentry
            note = pgettext("europeans_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "europeans_barrier",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans military site: Barrier
            purpose = pgettext("europeans_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an europeans military site: Barrier
            note = pgettext("europeans_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "europeans_outpost",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans military site: Outpost
            purpose = pgettext("europeans_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an europeans military site: Outpost
            note = pgettext("europeans_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "europeans_advanced_barrier",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans military site: Barrier
            purpose = pgettext("europeans_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an europeans military site: Barrier
            note = pgettext("europeans_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "europeans_tower",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans military site: Tower
            purpose = pgettext("europeans_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an europeans military site: Tower
            note = pgettext("europeans_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "europeans_tower_high",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans military site: Tower
            purpose = pgettext("europeans_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an europeans military site: Tower
            note = pgettext("europeans_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "europeans_advanced_tower",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans military site: Tower
            purpose = pgettext("europeans_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an europeans military site: Tower
            note = pgettext("europeans_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "europeans_castle",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans military site: Castle
            purpose = pgettext("europeans_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an europeans military site: Castle
            note = pgettext("europeans_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "europeans_fortress",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans military site: Fortress
            purpose = pgettext("europeans_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an europeans military site: Fortress
            note = pgettext("europeans_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "europeans_advanced_castle",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans military site: Castle
            purpose = pgettext("europeans_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an europeans military site: Castle
            note = pgettext("europeans_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },

      -- Seafaring/Ferry Sites - these are only displayed on seafaring/ferry maps
      {
         name = "europeans_ferry_yard",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Ferry Yard
            purpose = pgettext("europeans_building", "Builds ferries."),
            -- TRANSLATORS: Note helptext for an europeans production site: Ferry Yard
            note = pgettext("europeans_building", "Needs water nearby.")
         }
      },
      {
         name = "europeans_shipyard",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an europeans production site: Shipyard
            purpose = pgettext("europeans_building", "Constructs ships that are used for overseas colonization and for trading between ports.")
         }
      },

      -- Partially Finished Buildings - these are the same 2 buildings for all tribes
      {
         name = "constructionsite",
         helptexts = {
            -- TRANSLATORS: Lore helptext for an europeans building: Construction Site
            lore = pgettext("europeans_building", "‘Don’t swear at the builder who is short of building materials.’"),
            -- TRANSLATORS: Lore author helptext for an europeans building: Construction Site
            lore_author = pgettext("europeans_building", "Proverb widely used for impossible tasks of any kind"),
            -- TRANSLATORS: Purpose helptext for an europeans building: Construction Site
            purpose = pgettext("europeans_building", "A new building is being built at this construction site.")
         }
      },
      {
         name = "dismantlesite",
         helptexts = {
            -- TRANSLATORS: Lore helptext for an europeans building: Dismantle Site
            lore = pgettext("europeans_building", "‘New paths will appear when you are willing to tear down the old.’"),
            -- TRANSLATORS: Lore author helptext for an europeans building: Dismantle Site
            lore_author = pgettext("europeans_building", "Proverb"),
            -- TRANSLATORS: Purpose helptext for an europeans building: Dismantle Site
            purpose = pgettext("europeans_building", "A building is being dismantled at this dismantle site, returning some of the resources that were used during this building’s construction to your tribe’s stores.")
         }
      }
   },

   -- Special types
   builder = "europeans_builder",
   carrier = "europeans_carrier",
   carrier2 = "europeans_carrier",
   geologist = "europeans_geologist",
   soldier = "europeans_soldier",
   ship = "europeans_ship",
   ferry = "europeans_ferry",
   port = "europeans_port",
}

pop_textdomain()
