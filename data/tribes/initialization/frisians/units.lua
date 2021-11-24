descriptions = wl.Descriptions()

image_dirname = path.dirname(__file__) .. "images/"

push_textdomain("tribes_encyclopedia")

-- For formatting time strings
include "tribes/scripting/help/time_strings.lua"

descriptions:new_tribe {
   name = "frisians",
   military_capacity_script = path.dirname(__file__) .. "military_capacity.lua",
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

   collectors_points_table = {
      { ware = "gold", points = 3},
      { ware = "sword_short", points = 2},
      { ware = "sword_long", points = 3},
      { ware = "sword_broad", points = 6},
      { ware = "sword_double", points = 7},
      { ware = "helmet", points = 2},
      { ware = "helmet_golden", points = 7},
      { ware = "fur_garment", points = 2},
      { ware = "fur_garment_studded", points = 3},
      { ware = "fur_garment_golden", points = 6},
   },

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
               purpose = {
                  -- TRANSLATORS: Helptext for a frisian ware: Log, part 1
                  pgettext("ware", "Logs are an important basic building material. They are produced by felling trees."),
                  -- TRANSLATORS: Helptext for a frisian ware: Log, part 2
                  pgettext("frisians_ware", "Woodcutters fell the trees; foresters take care of the supply of trees. Logs are also used in the blacksmithy to build basic tools, and in the charcoal kiln for the production of coal. Smokeries use logs as fuel for smoking meat and fish.")
               }
            }
         },
         {
            name = "granite",
            default_target_quantity = 30,
            preciousness = 3,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a frisian ware: Granite, part 1
                  pgettext("ware", "Granite is a basic building material."),
                  -- TRANSLATORS: Helptext for a frisian ware: Granite, part 2
                  pgettext("frisians_ware", "The Frisians produce granite blocks in quarries and rock mines. They can be refined in a brick kiln.")
               }
            }
         },
         {
            name = "clay",
            default_target_quantity = 30,
            preciousness = 9,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Clay
               purpose = pgettext("frisians_ware", "Clay is made out of water and mud to be turned into bricks, used in ship construction and to improve the charcoal kiln.")
            }
         },
         {
            name = "brick",
            default_target_quantity = 40,
            preciousness = 3,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Brick
               purpose = pgettext("frisians_ware", "Bricks are the best and most important building material. They are made out of a mix of clay and granite dried in a coal fire.")
            }
         },
         {
            name = "reed",
            preciousness = 8,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Reed
               purpose = pgettext("frisians_ware", "Reed is grown in a reed farm. Nothing is better suited to make roofs waterproof. It is also used to make baskets and fishing nets as well as cloth." )
            }
         },
         {
            name = "fur",
            default_target_quantity = 10,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Fur
               purpose = pgettext("frisians_ware", "Fur is won from reindeer in a reindeer farm. It is woven into cloth or turned into fur garments for soldiers.")
            }
         },
         {
            name = "cloth",
            default_target_quantity = 10,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Cloth
               purpose = pgettext("frisians_ware", "Cloth is needed for ships. It is produced out of reindeer fur and reed.")
            }
         },
      },
      {
         -- Food
         {
            name = "fruit",
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Fruit
               purpose = pgettext("frisians_ware", "Fruit are berries gathered from berry bushes by a fruit collector. They are used for rations and for feeding the fish at the aqua farms.")
            }
         },
         {
            name = "water",
            preciousness = 2,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a frisian ware: Water, part 1
                  pgettext("ware", "Water is the essence of life!"),
                  -- TRANSLATORS: Helptext for a frisian ware: Water, part 2
                  pgettext("frisians_ware", "Water is used to bake bread and brew beer. Reindeer farms and aqua farms also consume it.")
               }
            }
         },
         {
            name = "barley",
            preciousness = 25,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Barley
               purpose = pgettext("frisians_ware", "Barley is a slow-growing grain that is used for baking bread and brewing beer. It is also eaten by reindeer.")
            }
         },
         {
            name = "honey",
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Honey
               purpose = pgettext("frisians_ware", "Honey is produced by bees belonging to a beekeeper. It is used to bake honey bread and brew mead.")
            }
         },
         {
            name = "bread_frisians",
            default_target_quantity = 20,
            preciousness = 3,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Bread
               purpose = pgettext("frisians_ware", "Bread is made out of barley and water and is used in the taverns to prepare rations. It is also consumed by training soldiers.")
            }
         },
         {
            name = "honey_bread",
            default_target_quantity = 20,
            preciousness = 5,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Honey Bread
               purpose = pgettext("frisians_ware", "This bread is sweetened with honey. It is consumed by the most experienced miners and in advanced soldier training.")
            }
         },
         {
            name = "beer",
            default_target_quantity = 15,
            preciousness = 3,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Beer
               purpose = pgettext("frisians_ware", "Beer is produced in breweries and used in drinking halls to produce meals. Soldiers drink beer while receiving basic training.")
            }
         },
         {
            name = "mead",
            default_target_quantity = 15,
            preciousness = 5,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Mead
               purpose = pgettext("frisians_ware", "Mead is produced by mead breweries. Soldiers drink mead during advanced training.")
            }
         },
         {
            name = "fish",
            default_target_quantity = 20,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Fish
               purpose = pgettext("frisians_ware", "Fish is a very important food resource for the Frisians. It is fished from the shore or reared in aqua farms." )
            }
         },
         {
            name = "meat",
            preciousness = 2,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a frisian ware: Meat, part 1
                  pgettext("ware", "Meat contains a lot of energy, and it is obtained from wild game taken by hunters."),
                  -- TRANSLATORS: Helptext for a frisian ware: Meat, part 2
                  pgettext("frisians_ware", "Meat has to be smoked in a smokery before being delivered to taverns, drinking halls and training sites.")
               }
            }
         },
         {
            name = "smoked_fish",
            default_target_quantity = 20,
            preciousness = 5,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Smoked Fish
               purpose = pgettext("frisians_ware", "Fish is smoked in a smokery. Smoked fish is then consumed by soldiers in training or turned into rations and meals for miners and scouts.")
            }
         },
         {
            name = "smoked_meat",
            default_target_quantity = 10,
            preciousness = 7,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Smoked Meat
               purpose = pgettext("frisians_ware", "Meat is smoked in a smokery. Smoked meat is then consumed by soldiers in training or turned into rations and meals for miners and scouts.")
            }
         },
         {
            name = "ration",
            default_target_quantity = 20,
            preciousness = 3,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a frisian ware: Ration, part 1
                  pgettext("ware", "A small bite to keep miners strong and working. The scout also consumes rations on his scouting trips."),
                  -- TRANSLATORS: Helptext for a frisian ware: Ration, part 2
                  pgettext("frisians_ware", "Rations are produced in taverns and drinking halls out of something to eat: Fruit, bread or smoked meat or fish.")
               }
            }
         },
         {
            name = "meal",
            default_target_quantity = 5,
            preciousness = 6,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Meal
               purpose = pgettext("frisians_ware", "A meal is made out of honey bread and beer and either smoked fish or meat. It is consumed by miners in deep mines.")
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
               purpose = {
                  -- TRANSLATORS: Helptext for a frisian ware: Coal, part 1
                  pgettext("ware", "Coal is mined in coal mines or produced out of logs by a charcoal burner in a charcoal kiln or charcoal burner’s house."),
                  -- TRANSLATORS: Helptext for a frisian ware: Coal, part 2
                  pgettext("frisians_ware", "The fires of the brick kilns, furnaces and armor smithies are fed with coal.")
               }
            }
         },
         {
            name = "iron_ore",
            default_target_quantity = 15,
            preciousness = 2,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a frisian ware: Iron Ore, part 1
                  pgettext("default_ware", "Iron ore is mined in iron mines."),
                  -- TRANSLATORS: Helptext for a frisian ware: Iron Ore, part 2
                  pgettext("frisians_ware", "It is smelted in a furnace to retrieve the iron.")
               }
            }
         },
         {
            name = "iron",
            default_target_quantity = 20,
            preciousness = 4,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a frisian ware: Iron, part 1
                  pgettext("ware", "Iron is smelted out of iron ores."),
                  -- TRANSLATORS: Helptext for a frisian ware: Iron, part 2
                  pgettext("frisians_ware", "It is produced by the furnace. Tools and weapons are made of iron. It is also used as jewellery for fur garment armor to give it a silver shine.")
               }
            }
         },
         {
            name = "gold_ore",
            default_target_quantity = 15,
            preciousness = 3,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a frisian ware: Gold Ore, part 1
                  pgettext("ware", "Gold ore is mined in a gold mine."),
                  -- TRANSLATORS: Helptext for a frisian ware: Gold Ore, part 2
                  pgettext("frisians_ware", "Smelted in a furnace, it turns into gold which is used as a precious building material and to produce weapons and armor.")
               }
            }
         },
         {
            name = "gold",
            default_target_quantity = 20,
            preciousness = 6,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a frisian ware: Gold, part 1
                  pgettext("ware", "Gold is the most valuable of all metals, and it is smelted out of gold ore."),
                  -- TRANSLATORS: Helptext for a frisian ware: Gold, part 2
                  pgettext("frisians_ware", "Only very important things are embellished with gold. It is produced by the furnace and is used to produce better swords and the best helmets. The best armor is also decorated with gold.")
               }
            }
         },
         {
            name = "scrap_iron",
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Scrap Iron
               purpose = pgettext("frisians_ware", "Discarded weapons and armor can be recycled in a recycling center to produce new tools, weapon and armor.")
            }
         },
         {
            name = "scrap_metal_mixed",
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Scrap metal (mixed)
               purpose = pgettext("frisians_ware", "Discarded weapons and armor can be recycled in a recycling center to produce new tools, weapon and armor.")
            }
         },
         {
            name = "fur_garment_old",
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Old Fur Garment
               purpose = pgettext("frisians_ware", "Old garments can be turned into fur in a recycling center.")
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
               -- TRANSLATORS: Helptext for a frisian ware: Pick
               purpose = pgettext("frisians_ware", "Picks are used by stonemasons and miners.")
            }
         },
         {
            name = "felling_ax",
            default_target_quantity = 3,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a frisian ware: Felling Ax, part 1
                  pgettext("ware", "The felling ax is the tool to chop down trees."),
                  -- TRANSLATORS: Helptext for a frisian ware: Felling Ax, part 2
                  pgettext("frisians_ware", "Felling axes are used by woodcutters and produced by the blacksmithy.")
               }
            }
         },
         {
            name = "shovel",
            default_target_quantity = 4,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a frisian ware: Shovel, part 1
                  pgettext("ware", "Shovels are needed for the proper handling of plants."),
                  -- TRANSLATORS: Helptext for a frisian ware: Shovel, part 2
                  pgettext("frisians_ware", "They are used by berry and reed farmers as well as foresters. Clay diggers also need them to dig mud out of hard soil.")
               }
            }
         },
         {
            name = "hammer",
            default_target_quantity = 2,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a frisian ware: Hammer, part 1
                  pgettext("ware", "The hammer is an essential tool."),
                  -- TRANSLATORS: Helptext for a frisian ware: Hammer, part 2
                  pgettext("frisians_ware", "Geologists, builders and blacksmiths all need a hammer. Make sure you’ve always got some in reserve! They are produced by the blacksmithy.")
               }
            }
         },
         {
            name = "fishing_net",
            default_target_quantity = 2,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Fishing Net
               purpose = pgettext("frisians_ware", "Fishing nets are the tool used by fishers." )
            }
         },
         {
            name = "hunting_spear",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a frisian ware: Hunting Spear, part 1
                  pgettext("ware", "This spear is light enough to be thrown, but heavy enough to kill any animal in one blow. It is only used by hunters."),
                  -- TRANSLATORS: Helptext for a frisian ware: Hunting Spear, part 2
                  pgettext("frisians_ware", "Hunting spears are produced by the blacksmithy")
               }
            }
         },
         {
            name = "scythe",
            default_target_quantity = 2,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a frisian ware: Scythe, part 1
                  pgettext("ware", "The scythe is the tool of the farmers."),
                  -- TRANSLATORS: Helptext for a frisian ware: Scythe, part 2
                  pgettext("frisians_ware", "Scythes are produced by the blacksmithy.")
               }
            }
         },
         {
            name = "bread_paddle",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a frisian ware: Bread Paddle, part 1
                  pgettext("ware", "The bread paddle is the tool of the baker, each baker needs one."),
                  -- TRANSLATORS: Helptext for a frisian ware: Bread Paddle, part 2
                  pgettext("frisians_ware", "Bread paddles are produced by the blacksmithy.")
               }
            }
         },
         {
            name = "kitchen_tools",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Kitchen Tools
               purpose = pgettext("frisians_ware", "Kitchen tools are needed for preparing rations and meals. The smoker also needs them.")
            }
         },
         {
            name = "fire_tongs",
            default_target_quantity = 2,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a frisian ware: Fire Tongs, part 1
                  pgettext("ware", "Fire tongs are the tools for smelting ores."),
                  -- TRANSLATORS: Helptext for a frisian ware: Fire Tongs, part 2
                  pgettext("frisians_ware", "They are used in the furnace and the brick kiln and produced by the blacksmithy.")
               }
            }
         },
         {
            name = "basket",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Basket
               purpose = pgettext("frisians_ware", "Baskets are needed by the fruit collector to gather berries. They are woven from reed and wood by the blacksmith.")
            }
         },
         {
            name = "needles",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Needles
               purpose = pgettext("frisians_ware", "Needles are used by seamstresses to sew cloth and fur garments.")
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
               -- TRANSLATORS: Helptext for a frisian ware: Short sword
               purpose = pgettext("frisians_ware", "This is the basic weapon of the Frisian soldiers. Together with a fur garment, it makes up the equipment of young soldiers. Short swords are produced by the small armor smithy.")
            }
         },
         {
            name = "sword_long",
            default_target_quantity = 2,
            preciousness = 3,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Long Sword
               purpose = pgettext("frisians_ware", "The long sword is the weapon used by level 1 soldiers. Level 4 soldiers are equipped with a long and a double-edged sword.")
            }
         },
         {
            name = "sword_broad",
            default_target_quantity = 2,
            preciousness = 3,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Broadsword
               purpose = pgettext("frisians_ware", "The broadsword is the weapon used by level 2 soldiers. Level 5 soldiers are equipped with a broadsword and a double-edged sword.")
            }
         },
         {
            name = "sword_double",
            default_target_quantity = 2,
            preciousness = 3,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Double-edged Sword
               purpose = pgettext("frisians_ware", "The double-edged sword is the weapon used by level 3 soldiers. Level 6 soldiers are equipped with two of these ferocious swords.")
            }
         },
         {
            name = "fur_garment",
            default_target_quantity = 30,
            preciousness = 3,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Fur Garment
               purpose = pgettext("frisians_ware", "Fur can be sewn into garments. They are used as basic armor. All new soldiers are clothed in a fur garment.")
            }
         },
         {
            name = "fur_garment_studded",
            default_target_quantity = 2,
            preciousness = 3,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Studded Fur Garment
               purpose = pgettext("frisians_ware", "Ordinary fur garments can be decorated with iron to give them a silvery shine. These clothes make good armor.")
            }
         },
         {
            name = "fur_garment_golden",
            default_target_quantity = 2,
            preciousness = 3,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Golden Fur Garment
               purpose = pgettext("frisians_ware", "Ordinary fur garments can be decorated with iron and gold. Such clothes are the best armor.")
            }
         },
         {
            name = "helmet",
            default_target_quantity = 2,
            preciousness = 3,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Helmet
               purpose = pgettext("frisians_ware", "A helmet is a basic tool to protect soldiers. It is produced in the small armor smithy and used to train soldiers from health level 0 to level 1.")
            }
         },
         {
            name = "helmet_golden",
            default_target_quantity = 2,
            preciousness = 3,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian ware: Golden Helmet
               purpose = pgettext("frisians_ware", "A golden helmet protects soldiers. It is produced in the large armor smithy and used to train soldiers from health level 1 to level 2.")
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
               -- TRANSLATORS: Helptext for a frisian worker: Carrier
               purpose = pgettext("frisians_worker", "Carries items along your roads.")
            }
         },
         {
            name = "frisians_ferry",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Ferry
               purpose = pgettext("frisians_worker", "Ships wares across narrow rivers.")
            }
         },
         {
            name = "frisians_reindeer",
            default_target_quantity = 10,
            preciousness = 2,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Reindeer
               purpose = pgettext("frisians_worker", "Reindeer help to carry items along busy roads. They are reared in a reindeer farm.")
            }
         },
         {
            name = "frisians_reindeer_breeder",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Reindeer Breeder
               purpose = pgettext("frisians_worker", "Breeds reindeer as carriers and for their fur.")
            }
         }
      },
      {
         -- Building Materials
         {
            name = "frisians_stonemason",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Stonemason
               purpose = pgettext("frisians_worker", "Cuts raw pieces of granite out of rocks in the vicinity.")
            }
         },
         {
            name = "frisians_woodcutter",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Woodcutter
               purpose = pgettext("frisians_worker", "Fells trees.")
            }
         },
         {
            name = "frisians_forester",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Forester
               purpose = pgettext("frisians_worker", "Plants trees.")
            }
         },
         {
            name = "frisians_claydigger",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Clay Digger
               purpose = pgettext("frisians_worker", "Makes clay out of mud and water.")
            }
         },
         {
            name = "frisians_brickmaker",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Brickmaker
               purpose = pgettext("frisians_worker", "Burns bricks out of clay and granite.")
            }
         },
         {
            name = "frisians_builder",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Builder
               purpose = pgettext("frisians_worker", "Works at construction sites to raise new buildings.")
            }
         },
         {
            name = "frisians_reed_farmer",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Gardener
               purpose = pgettext("frisians_worker", "Plants and harvests reed fields.")
            }
         },
         {
            name = "frisians_seamstress",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Seamstress
               purpose = pgettext("frisians_worker", "Produces cloth and sews fur garments.")
            }
         },
         {
            name = "frisians_seamstress_master",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Master Seamstress
               purpose = pgettext("frisians_worker", "Sews armor out of fur garments and metal.")
            }
         },
         {
            name = "frisians_shipwright",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Shipwright
               purpose = pgettext("frisians_worker", "Works at the shipyard and constructs new ships.")
            }
         }
      },
      {
         -- Food
         {
            name = "frisians_fisher",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Fisher
               purpose = pgettext("frisians_worker", "Catches fish in the sea.")
            }
         },
         {
            name = "frisians_hunter",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Hunter
               purpose = pgettext("frisians_worker", "The hunter brings fresh, raw meat to the colonists.")
            }
         },
         {
            name = "frisians_farmer",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Farmer
               purpose = pgettext("frisians_worker", "Plants fields.")
            }
         },
         {
            name = "frisians_berry_farmer",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Berry Farmer
               purpose = pgettext("frisians_worker", "Plants berry bushes.")
            }
         },
         {
            name = "frisians_fruit_collector",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Fruit Collector
               purpose = pgettext("frisians_worker", "Gathers berries.")
            }
         },
         {
            name = "frisians_smoker",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Smoker
               purpose = pgettext("frisians_worker", "Refines meat and fish by smoking them.")
            }
         },
         {
            name = "frisians_beekeeper",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Beekeeper
               purpose = pgettext("frisians_worker", "Lets bees swarm over flowers, then gathers the honey.")
            }
         },
         {
            name = "frisians_baker",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Baker
               purpose = pgettext("frisians_worker", "Bakes bread for miners and soldiers.")
            }
         },
         {
            name = "frisians_baker_master",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Master Baker
               purpose = pgettext("frisians_worker", "This baker is skilled enough to bake bread sweetened with honey.")
            }
         },
         {
            name = "frisians_brewer",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Brewer
               purpose = pgettext("frisians_worker", "Brews beer.")
            }
         },
         {
            name = "frisians_brewer_master",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Master Brewer
               purpose = pgettext("frisians_worker", "Brews beer and mead.")
            }
         },
         {
            name = "frisians_landlady",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Landlady
               purpose = pgettext("frisians_worker", "Prepares rations and meals for miners and scouts.")
            }
         }
      },
      {
         -- Mining
         {
            name = "frisians_geologist",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Geologist
               purpose = pgettext("frisians_worker", "Discovers resources for mining.")
            }
         },
         {
            name = "frisians_miner",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Miner
               purpose = pgettext("frisians_worker", "Works deep in the mines to obtain coal, iron, gold or granite.")
            }
         },
         {
            name = "frisians_miner_master",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Master Miner
               purpose = pgettext("frisians_worker", "Works deep in the mines to obtain coal, iron, gold or granite.")
            }
         },
         {
            name = "frisians_charcoal_burner",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Charcoal Burner
               purpose = pgettext("frisians_worker", "Burns logs and clay to produce coal.")
            }
         },
         {
            name = "frisians_smelter",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Smelter
               purpose = pgettext("frisians_worker", "Smelts iron and gold at furnaces or recycling centers.")
            }
         },
         {
            name = "frisians_blacksmith",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Blacksmith
               purpose = pgettext("frisians_worker", "Produces weapons and armor for soldiers and tools for workers.")
            }
         },
         {
            name = "frisians_blacksmith_master",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Master Blacksmith
               purpose = pgettext("frisians_worker", "Produces the best weapons and armor for soldiers and tools for workers.")
            }
         }
      },
      {
         -- Military
         {
            name = "frisians_soldier",
            default_target_quantity = 10,
            preciousness = 5,
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Soldier
               purpose = pgettext("frisians_worker", "Defend and Conquer!")
            }
         },
         {
            name = "frisians_trainer",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Trainer
               purpose = pgettext("frisians_worker", "Trains the soldiers.")
            }
         },
         {
            name = "frisians_scout",
            helptexts = {
               -- TRANSLATORS: Helptext for a frisian worker: Scout
               purpose = pgettext("frisians_worker", "Explores unknown territory.")
            }
         }
      }
   },

   immovables = {
      {
         name = "ashes",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: Ashes
            purpose = _("The remains of a destroyed building.")
         }
      },
      {
         name = "destroyed_building",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: Destroyed Building
            purpose = _("The remains of a destroyed building.")
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
         name = "barleyfield_tiny",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: Barley Field
            purpose = _("This field has just been planted.")
         }
      },
      {
         name = "barleyfield_small",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: Barley Field
            purpose = _("This field is growing.")
         }
      },
      {
         name = "barleyfield_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: Barley Field
            purpose = _("This field is flowering. Honey can be produced from it by a beekeeper.")
         }
      },
      {
         name = "barleyfield_ripe",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: Barley Field
            purpose = _("This field is ready for harvesting.")
         }
      },
      {
         name = "barleyfield_harvested",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: Barley Field
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
         name = "frisians_resi_none",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian resource indicator: No resources
            purpose = _("There are no resources in the ground here.")
         }
      },
      {
         name = "frisians_resi_water",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian resource indicator: Water
            purpose = _("There is water in the ground here that can be pulled up by a well.")
         }
      },
      {
         name = "frisians_resi_coal_1",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for a frisian resource indicator: Coal, part 1
               _("Coal veins contain coal that can be dug up by coal mines."),
               -- TRANSLATORS: Helptext for a frisian resource indicator: Coal, part 2
               _("There is only a little bit of coal here.")
            }
         }
      },
      {
         name = "frisians_resi_iron_1",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for a frisian resource indicator: Iron, part 1
               _("Iron veins contain iron ore that can be dug up by iron mines."),
               -- TRANSLATORS: Helptext for a frisian resource indicator: Iron, part 2
               _("There is only a little bit of iron here.")
            }
         }
      },
      {
         name = "frisians_resi_gold_1",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for a frisian resource indicator: Gold, part 1
               _("Gold veins contain gold ore that can be dug up by gold mines."),
               -- TRANSLATORS: Helptext for a frisian resource indicator: Gold, part 2
               _("There is only a little bit of gold here.")
            }
         }
      },
      {
         name = "frisians_resi_stones_1",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for a Frisian resource indicator: Stones, part 1
               _("Granite is a basic building material and can be dug up by a rock mine."),
               -- TRANSLATORS: Helptext for a Frisian resource indicator: Stones, part 2
               _("There is only a little bit of granite here.")
            }
         }
      },
      {
         name = "frisians_resi_coal_2",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for a frisian resource indicator: Coal, part 1
               _("Coal veins contain coal that can be dug up by coal mines."),
               -- TRANSLATORS: Helptext for a frisian resource indicator: Coal, part 2
               _("There is a lot of coal here.")
            }
         }
      },
      {
         name = "frisians_resi_iron_2",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for a frisian resource indicator: Iron, part 1
               _("Iron veins contain iron ore that can be dug up by iron mines."),
               -- TRANSLATORS: Helptext for a frisian resource indicator: Iron, part 2
               _("There is a lot of iron here.")
            }
         }
      },
      {
         name = "frisians_resi_gold_2",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for a frisian resource indicator: Gold, part 1
               _("Gold veins contain gold ore that can be dug up by gold mines."),
               -- TRANSLATORS: Helptext for a frisian resource indicator: Gold, part 2
               _("There is a lot of gold here.")
            }
         }
      },
      {
         name = "frisians_resi_stones_2",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for a Frisian resource indicator: Stones, part 1
               _("Granite is a basic building material and can be dug up by a rock mine."),
               -- TRANSLATORS: Helptext for a Frisian resource indicator: Stones, part 2
               _("There is a lot of granite here.")
            }
         }
      },
      {
         name = "frisians_shipconstruction",
         helptexts = {
            -- TRANSLATORS: Helptext for a frisian immovable: Ship Under Construction
            purpose = _("A ship is being constructed at this site.")
         }
      },
      -- These non-frisian immovables can be used by bee-keepers
      -- The tribe would work without defining these here, but this way we add them to the encyclopedia.
      {
         name = "wheatfield_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for an empire/barbarian immovable usable by frisians: Wheat field
            purpose = _("This field is flowering. Honey can be produced from it by a beekeeper.")
         }
      },
      {
         name = "cornfield_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for a an atlantean immovable usable by frisians: Corn Field
            purpose = _("This field is flowering. Honey can be produced from it by a beekeeper.")
         }
      },
      {
         name = "blackrootfield_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for an atlantean immovable usable by frisians: Blackroot Field
            purpose = _("This field is flowering. Honey can be produced from it by a beekeeper.")
         }
      },
      {
         name = "grapevine_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for an empire immovable usable by frisians: Grapevine
            purpose = _("This grapevine is flowering. Honey can be produced from it by a beekeeper.")
         }
      },
      {
         name = "cassavafield_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by frisians: Cassava field
            purpose = _("This field is flowering. Honey can be produced from it by a beekeeper.")
         }
      },
      -- Used by the fruit collector
      {
         name = "grapevine_ripe",
         helptexts = {
            -- TRANSLATORS: Helptext for an empire immovable usable by frisians: Grapevine
            purpose = _("This grapevine is ready for harvesting.")
         }
      },
      -- non frisian Immovables used by the woodcutter
      {
         name = "deadtree7",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by frisians: Dead Tree
            purpose = _("The remains of an old tree.")
         }
      },
      {
         name = "balsa_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by frisians: Balsa Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_black_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by frisians: Balsa Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_desert_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by frisians: Balsa Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_winter_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by frisians: Balsa Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by frisians: Ironwood Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_black_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by frisians: Ironwood Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_desert_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by frisians: Ironwood Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_winter_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by frisians: Ironwood Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by frisians: Rubber Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_black_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by frisians: Rubber Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_desert_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by frisians: Rubber Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_winter_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by frisians: Rubber Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
   },

   -- The order here also determines the order in lists on screen.
   buildings = {
      -- Warehouses
      {
         name = "frisians_headquarters",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a frisian warehouse: Headquarters
            purpose = pgettext("frisians_building", "Accommodation for your people. Also stores your wares and tools."),
            -- TRANSLATORS: Note helptext for a frisian warehouse: Headquarters
            note = pgettext("frisians_building", "The headquarters is your main building.")
         }
      },
      {
         name = "frisians_warehouse",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a frisian warehouse: Warehouse
            purpose = pgettext("frisians_building", "Your workers and soldiers will find shelter here. Also stores your wares and tools.")
         }
      },
      {
         name = "frisians_port",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a frisian warehouse: Port
            purpose = pgettext("frisians_building", "Serves as a base for overseas colonization and trade. Also stores your soldiers, wares and tools."),
            -- TRANSLATORS: Note helptext for a frisian warehouse: Port
            note = pgettext("frisians_building", "Similar to the Headquarters a Port can be attacked and destroyed by an enemy. It is recommendable to send soldiers to defend it.")
         }
      },

      -- Small
      {
         name = "frisians_quarry",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Quarry
            lore = pgettext("frisians_building", "When I swing my pick, whole mountains fall before me!"),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Quarry
            lore_author = pgettext("frisians_building", "A stonemason"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Quarry
            purpose = pgettext("frisians_building", "Cuts raw pieces of granite out of rocks in the vicinity."),
            -- TRANSLATORS: Note helptext for a frisian production site: Quarry
            note = pgettext("frisians_building", "The quarry needs rocks to cut within the work area."),
            -- TRANSLATORS: Performance helptext for a frisian production site: Quarry
            performance = pgettext("frisians_building", "The stonemason pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 25):bformat(25))
         }
      },
      {
         name = "frisians_woodcutters_house",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Woodcutter's House
            lore = pgettext("frisians_building", "I cannot see a tree without imagining what it would look like in terms of furniture."),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Woodcutter's House
            lore_author = pgettext("frisians_building", "An over-enthusiastic woodcutter"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Woodcutter's House
            purpose = pgettext("building", "Fells trees in the surrounding area and processes them into logs."),
            -- TRANSLATORS: Note helptext for a frisian production site: Woodcutter's House
            note = pgettext("frisians_building", "The woodcutter’s house needs trees to fell within the work area.")
         }
      },
      {
         name = "frisians_foresters_house",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Forester's House
            lore = pgettext("frisians_building", "What can ever be more beautiful than the brilliant sun’s beams shining through the glistering canopy of leaves?"),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Forester's House
            lore_author = pgettext("frisians_building", "A forester explaining his choice of profession"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Forester's House
            purpose = pgettext("building", "Plants trees in the surrounding area."),
            -- TRANSLATORS: Note helptext for a frisian production site: Forester's House
            note = pgettext("frisians_building", "The forester’s house needs free space within the work area to plant the trees."),
            -- TRANSLATORS: Performance helptext for a frisian production site: Forester's House
            performance = pgettext("frisians_building", "The forester pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 12):bformat(12))
         }
      },
      {
         name = "frisians_hunters_house",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Hunter's House
            purpose = pgettext("building", "Hunts animals to produce meat."),
            -- TRANSLATORS: Note helptext for a frisian production site: Hunter's House
            note = pgettext("frisians_building", "The hunter’s house needs animals to hunt within the work area."),
            -- TRANSLATORS: Performance helptext for a frisian production site: Hunter's House
            performance = pgettext("frisians_building", "The hunter pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 35):bformat(35))
         }
      },
      {
         name = "frisians_fishers_house",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Fisher's House
            lore = pgettext("frisians_building", "Hunters can’t sell anything on a Friday, but fishers don’t have such problems."),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Fisher's House
            lore_author = pgettext("frisians_building", "A hunter admiring a fisher"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Fisher's House
            purpose = pgettext("frisians_building", "Fishes on the coast near the fisher’s house."),
            -- TRANSLATORS: Note helptext for a frisian production site: Fisher's House
            note = pgettext("frisians_building", "The fisher’s house needs water full of fish within the work area."),
            -- TRANSLATORS: Performance helptext for a frisian production site: Fisher's House
            performance = pgettext("frisians_building", "The fisher pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 16):bformat(16))
         }
      },
      {
         name = "frisians_reed_farm",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Reed Farm
            lore = pgettext("frisians_building", "No worse fortune can befall a reed farmer than to see his roof leaking."),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Reed Farm
            lore_author = pgettext("frisians_building", "Anonymous reed farmer"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Reed Farm
            purpose = pgettext("frisians_building", "Cultivates reed that serves three different purposes for the Frisians."),
            -- TRANSLATORS: Note helptext for a frisian production site: Reed Farm
            note = pgettext("frisians_building", "Reed is the traditional material for roofing. It is also needed to produce baskets and fishing nets, and it is woven – together with reindeer fur – into the cloth used for ships’ sails.")
         }
      },
      {
         name = "frisians_well",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Well
            lore = pgettext("frisians_building", "We love the sea so much that we don’t want to drink it empty!"),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Well
            lore_author = pgettext("frisians_building", "Chieftain Arldor’s retort when he was asked why his tribe can’t drink salt water"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Well
            purpose = pgettext("building", "Draws water out of the deep."),
            -- TRANSLATORS: Performance helptext for a frisian production site: Well
            performance = pgettext("frisians_building", "The well needs %s on average to produce one bucket of water."):bformat(ngettext("%d second", "%d seconds", 40):bformat(40))
         }
      },
      {
         name = "frisians_clay_pit",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Clay Pit
            lore = pgettext("frisians_building", "You think you can build a house without my help?"),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Clay Pit
            lore_author = pgettext("frisians_building", "A clay digger arguing with a builder"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Clay Pit
            purpose = pgettext("building", "Digs up mud from the ground and uses water to turn it into clay. Clay is used to make bricks, reinforce the charcoal kiln and to build ships.")
         }
      },
      {
         name = "frisians_charcoal_burners_house",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Charcoal Burner's House
            lore = pgettext("frisians_building", "No other tribe has ever mastered the art of charcoal burning as we have!"),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Charcoal Burner's House
            lore_author = pgettext("frisians_building", "The inventor of the Frisian charcoal kiln"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Charcoal Burner's House
            purpose = pgettext("building", "Burns logs into charcoal."),
            -- TRANSLATORS: Note helptext for a frisian production site: Charcoal Burner's House
            note = pgettext("building", "The charcoal burner’s house needs holes in the ground that were dug by a clay pit’s worker nearby to erect charcoal stacks in them."),
            -- TRANSLATORS: Performance helptext for a frisian production site: Charcoal Burner's House
            performance = pgettext("frisians_building", "The charcoal burner’s house needs %s on average to produce one coal."):bformat(ngettext("%d second", "%d seconds", 80):bformat(80))
         }
      },
      {
         name = "frisians_berry_farm",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Berry Farm
            lore = pgettext("frisians_building", "My bushes may not be as tall as your trees, but I don’t know anybody who likes to eat bark!"),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Berry Farm
            lore_author = pgettext("frisians_building", "A berry farmer to a forester"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Berry Farm
            purpose = pgettext("building", "Plants berry bushes in the surrounding area."),
            -- TRANSLATORS: Note helptext for a frisian production site: Berry Farm
            note = pgettext("frisians_building", "The berry farm needs free space within the work area to plant the bushes."),
            -- TRANSLATORS: Performance helptext for a frisian production site: Berry Farm
            performance = pgettext("frisians_building", "The berry farmer pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 21):bformat(21))
         }
      },
      {
         name = "frisians_collectors_house",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Fruit Collector's House
            lore = pgettext("frisians_building", "Meat doesn’t grow on bushes. Fruit does."),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Fruit Collector's House
            lore_author = pgettext("frisians_building", "A fruit collector advertising his harvest to a landlady"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Fruit Collector's House
            purpose = pgettext("building", "Collects berries from nearby bushes."),
            -- TRANSLATORS: Note helptext for a frisian production site: Fruit Collector's House
            note = pgettext("frisians_building", "The fruit collector needs bushes full of berries within the work area."),
            -- TRANSLATORS: Performance helptext for a frisian production site: Fruit Collector's House
            performance = pgettext("frisians_building", "The fruit collector pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 21):bformat(21))
         }
      },
      {
         name = "frisians_beekeepers_house",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Beekeeper's House
            lore = pgettext("frisians_building", "If my honey tastes bitter, I must have left some bee stings in it. There are never any bee stings in my honey, therefore, it is not bitter."),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Beekeeper's House
            lore_author = pgettext("frisians_building", "A beekeeper ignoring a customer’s complaint"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Beekeeper's House
            purpose = pgettext("frisians_building", "Keeps bees and lets them swarm over flowering fields to produce honey."),
            -- TRANSLATORS: Note helptext for a frisian production site: Beekeeper's House
            note = pgettext("frisians_building", "Needs medium-sized fields (barley, wheat, reed, corn or blackroot) or bushes (berry bushes or grapevines) nearby."),
            -- TRANSLATORS: Performance helptext for a frisian production site: Beekeeper's House
            performance = pgettext("frisians_building", "The beekeeper pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 45):bformat(45))
         }
      },
      {
         name = "frisians_aqua_farm",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Aqua Farm
            lore = pgettext("frisians_building", "Why on earth shouldn’t we be able to catch fish even in the desert?"),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Aqua Farm
            lore_author = pgettext("frisians_building", "The fisherman who invented aqua farming"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Aqua Farm
            purpose = pgettext("frisians_building", "Breeds fish as food for soldiers and miners."),
            -- TRANSLATORS: Note helptext for a frisian production site: Aqua Farm
            note = pgettext("building", "The aqua farm needs holes in the ground that were dug by a clay pit’s worker nearby to use as fishing ponds.")
         }
      },
      {
         name = "frisians_scouts_house",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Scout's House
            lore = pgettext("frisians_building", "Everyone has their own ideas on how exactly we should explore the enemy’s territory… One more ‘improvement’ suggestion and we’ll demand meals instead of rations!"),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Scout's House
            lore_author = pgettext("frisians_building", "The spokesman of the scouts’ labor union"),
            no_scouting_building_connected = pgettext("frisians_building", "You need to connect this flag to a scout’s house before you can send a scout here."),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Scout's House
            purpose = pgettext("building", "Explores unknown territory."),
            -- TRANSLATORS: Performance helptext for a frisian production site: Scout's House
            performance = pgettext("frisians_building", "The scout pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 30):bformat(30))
         }
      },

      -- Medium
      {
         name = "frisians_brick_kiln",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Brick Kiln
            lore = pgettext("frisians_building", "If there is not enough coal, only the most foolish of leaders would deprive his brick kilns of it first."),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Brick Kiln
            lore_author = pgettext("frisians_building", "A brickmaker arguing with his chieftain who was doing just that"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Brick Kiln
            purpose = pgettext("building", "Burns bricks using granite and clay, and coal as fuel. Bricks are the most important building material."),
            -- TRANSLATORS: Performance helptext for a frisian production site: Brick Kiln
            performance = pgettext("frisians_building", "The brick kiln needs %s on average to produce three bricks."):bformat(ngettext("%d second", "%d seconds", 84):bformat(84))
         }
      },
      {
         name = "frisians_furnace",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Furnace
            lore = pgettext("frisians_building", "Miners get ores from the depths of the hills; but without our work, their labour is in vain."),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Furnace
            lore_author = pgettext("frisians_building", "Slogan of the Smelters’ Guild"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Furnace
            purpose = pgettext("building", "Smelts iron ore and gold ore into iron and gold ingots using coal.")
         }
      },
      {
         name = "frisians_recycling_center",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Recycling Center
            lore = pgettext("frisians_building", "Of course these weapons could be used by other soldiers again without being smelted down first! The only drawback is that they’d break in two at the first blow."),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Recycling Center
            lore_author = pgettext("frisians_building", "A smelter explaining the need for recycling to his impatient chieftain"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Recycling Center
            purpose = pgettext("frisians_building", "Recycles old armor and weapon parts that have been discarded by training sites into fur, iron and gold.")
         }
      },
      {
         name = "frisians_blacksmithy",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Blacksmithy
            lore = pgettext("frisians_building", "If you don’t have iron, change your name from blacksmith to lacksmith!"),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Blacksmithy
            lore_author = pgettext("frisians_building", "Irritated chieftain during a metal shortage"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Blacksmithy
            purpose = pgettext("building", "Forges tools to equip new workers."),
            -- TRANSLATORS: Performance helptext for a frisian production site: Blacksmithy
            performance = pgettext("frisians_building", "The blacksmith needs %s on average to produce one tool."):bformat(ngettext("%d second", "%d seconds", 67):bformat(67))
         }
      },
      {
         name = "frisians_armor_smithy_small",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Small Armor Smithy
            lore = pgettext("frisians_building", "I don’t forge swords because soldiers need ’em, but soldiers get ’em because I forge ’em."),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Small Armor Smithy
            lore_author = pgettext("frisians_building", "A blacksmith pointing out his influence on soldier training"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Small Armor Smithy
            purpose = pgettext("frisians_building", "Produces basic weapons and helmets for the soldiers.")
         }
      },
      {
         name = "frisians_armor_smithy_large",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Large Armor Smithy
            lore = pgettext("frisians_building", "Of course I could still forge short and long swords, but it is beneath my honor to bother with such basic equipment now."),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Large Armor Smithy
            lore_author = pgettext("frisians_building", "A master blacksmith refusing to forge anything but the most sophisticated helmets and weapons"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Large Armor Smithy
            purpose = pgettext("frisians_building", "Produces advanced weapons and golden helmets for the soldiers.")
         }
      },
      {
         name = "frisians_sewing_room",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Sewing Room
            lore = pgettext("frisians_building", "You soldiers think a good sword is everything, but where would you be if you had no garments?"),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Sewing Room
            lore_author = pgettext("frisians_building", "A seamstress scolding a soldier for disrespecting her profession"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Sewing Room
            purpose = pgettext("building", "Sews fur garments out of reindeer fur."),
            -- TRANSLATORS: Performance helptext for a frisian production site: Sewing Room
            performance = pgettext("frisians_building", "The sewing room needs %s on average to produce one fur garment."):bformat(ngettext("%d second", "%d seconds", 45):bformat(45))
         }
      },
      {
         name = "frisians_tailors_shop",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Tailor's Shop
            lore = pgettext("frisians_building", "Don’t complain if these garments are too heavy – they’re not supposed to be light but to keep you alive a bit longer!"),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Tailor's Shop
            lore_author = pgettext("frisians_building", "A trainer scolding a soldier"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Tailor's Shop
            purpose = pgettext("building", "Equips fur garments with iron or gold to produce good armor.")
         }
      },
      {
         name = "frisians_charcoal_kiln",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Charcoal Kiln
            lore = pgettext("frisians_building", "No other tribe has ever mastered the art of charcoal burning as we have!"),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Charcoal Kiln
            lore_author = pgettext("frisians_building", "The inventor of the Frisian charcoal kiln"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Charcoal Kiln
            purpose = pgettext("building", "Burns logs into charcoal."),
            -- TRANSLATORS: Performance helptext for a frisian production site: Charcoal Kiln
            performance = pgettext("frisians_building", "The charcoal kiln needs %s on average to produce one coal."):bformat(ngettext("%d second", "%d seconds", 60):bformat(60))
         }
      },
      {
         name = "frisians_smokery",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Smokery
            lore = pgettext("frisians_building", "Miners and soldiers are so picky… But who am I to complain, as I make my living from it?"),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Smokery
            lore_author = pgettext("frisians_building", "A smoker explaining his profession"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Smokery
            purpose = pgettext("frisians_building", "Smokes fish and meat using logs. Only smoked meat and fish are good enough to be eaten by miners and soldiers."),
            -- TRANSLATORS: Performance helptext for a frisian production site: Smokery
            performance = pgettext("frisians_building", "The smokery needs %s on average to smoke two fish or two meat."):bformat(ngettext("%d second", "%d seconds", 46):bformat(46))
         }
      },
      {
         name = "frisians_bakery",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Bakery
            lore = pgettext("frisians_building", "Why shouldn’t my bread taste good? It’s only barley and water!"),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Bakery
            lore_author = pgettext("frisians_building", "A baker"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Bakery
            purpose = pgettext("frisians_building", "Bakes bread out of barley and water to feed miners and soldiers."),
            -- TRANSLATORS: Performance helptext for a frisian production site: Bakery
            performance = pgettext("frisians_building", "The bakery needs %s on average to produce one loaf of bread."):bformat(ngettext("%d second", "%d seconds", 40):bformat(40))
         }
      },
      {
         name = "frisians_honey_bread_bakery",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Honey Bread Bakery
            lore = pgettext("frisians_building", "Rookies will say that vengeance is sweet. Heroes will say that honey bread is sweeter."),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Honey Bread Bakery
            lore_author = pgettext("frisians_building", "A trainer in conversation with a baker"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Honey Bread Bakery
            purpose = pgettext("frisians_building", "Bakes honey bread out of barley, water and honey to feed miners in deep mines and soldiers in advanced training.")
         }
      },
      {
         name = "frisians_brewery",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Brewery
            lore = pgettext("frisians_building", "I know no single master miner who’ll ever work without a nice pint of beer!"),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Brewery
            lore_author = pgettext("frisians_building", "A brewer boasting about the importance of his profession"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Brewery
            purpose = pgettext("frisians_building", "Brews beer for miners and soldier training."),
            -- TRANSLATORS: Performance helptext for a frisian production site: Brewery
            performance = pgettext("frisians_building", "The brewery needs %s on average to brew one mug of beer."):bformat(ngettext("%d second", "%d seconds", 60):bformat(60))
         }
      },
      {
         name = "frisians_mead_brewery",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Mead Brewery
            lore = pgettext("frisians_building", "If you like beer, you’ve never tasted mead."),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Mead Brewery
            lore_author = pgettext("frisians_building", "Slogan over a mead brewery"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Mead Brewery
            purpose = pgettext("frisians_building", "Brews beer out of barley and water. It also brews mead, which is beer refined with honey. Mead is consumed by experienced soldiers.")
         }
      },
      {
         name = "frisians_tavern",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Tavern
            lore = pgettext("frisians_building", "Nothing but fruit all day… Couldn’t you hurry up a bit?"),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Tavern
            lore_author = pgettext("frisians_building", "Hungry customers in times of a shortage of smoked fish and meat"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Tavern
            purpose = pgettext("building", "Prepares rations to feed the scouts and miners."),
            -- TRANSLATORS: Performance helptext for a frisian production site: Tavern
            performance = pgettext("frisians_building", "The tavern can produce one ration in %s on average if the supply is steady; otherwise, it will take 50%% longer."):bformat(ngettext("%d second", "%d seconds", 33):bformat(33))
         }
      },
      {
         name = "frisians_drinking_hall",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Drinking Hall
            lore = pgettext("frisians_building", "All I need to be happy is a slice of honey bread with some smoked meat and a beer."),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Drinking Hall
            lore_author = pgettext("frisians_building", "A master miner to the landlady"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Drinking Hall
            purpose = pgettext("frisians_building", "Prepares rations for scouts and rations and meals to feed the miners in all mines.")
         }
      },
      {
         name = "frisians_barracks",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Barracks
            lore = pgettext("frisians_building", "You have thirty seconds to learn the basics of swordfighting and how to stay alive in battle. A third of that time has gone by for the introduction alone! You’d better pay close attention to me in order to make the most of it. Now here is your new short sword, forged just for you by our best blacksmiths. Time’s up everyone, now go occupy your sentinels!"),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Barracks
            lore_author = pgettext("frisians_building", "A trainer greeting the new recruits"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Barracks
            purpose = pgettext("frisians_building", "Equips recruits and trains them as soldiers."),
            -- TRANSLATORS: Performance helptext for a frisian production site: Barracks
            performance = pgettext("frisians_building", "The barracks needs %s on average to recruit one soldier."):bformat(ngettext("%d second", "%d seconds", 30):bformat(30))
         }
      },

      -- Big
      {
         name = "frisians_reindeer_farm",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Reindeer Farm
            lore = pgettext("frisians_building", "Who says a beast of burden cannot be useful for other things than transport?"),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Reindeer Farm
            lore_author = pgettext("frisians_building", "The reindeer breeder who first proposed using reindeer fur for clothing"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Reindeer Farm
            purpose = pgettext("frisians_building", "Breeds strong reindeer for adding them to the transportation system. Also keeps them for their fur, which is turned into armor and cloth."),
            -- TRANSLATORS: Note helptext for a frisian production site: Reindeer Farm
            note = pgettext("frisians_building", "If the supply is steady, the reindeer farm produces one meat after producing three pieces of fur.")
         }
      },
      {
         name = "frisians_farm",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Farm
            lore = pgettext("frisians_building", "No frost, no heat; no rain, no drought; no rats, no locusts; naught can destroy my harvest."),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Farm
            lore_author = pgettext("frisians_building", "A farmer’s reply when asked by his chieftain why he was planting such a slow-growing grain."),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Farm
            purpose = pgettext("building", "Sows and harvests barley."),
            -- TRANSLATORS: Note helptext for a frisian production site: Farm
            note = pgettext("frisians_building", "The farm needs free space within the work area to plant seeds.")
         }
      },

      -- Mines
      {
         name = "frisians_rockmine",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Rock Mine
            purpose = pgettext("building", "Digs granite out of the ground in mountain terrain."),
            -- TRANSLATORS: Performance helptext for a frisian production site: Rock Mine
            performance = pgettext("frisians_building", "If the food supply is steady, the rock mine can produce two blocks of granite in %s on average."):bformat(ngettext("%d second", "%d seconds", 85):bformat(85))
         }
      },
      {
         name = "frisians_rockmine_deep",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Deep Rock Mine
            purpose = pgettext("building", "Digs granite out of the ground in mountain terrain."),
            -- TRANSLATORS: Performance helptext for a frisian production site: Deep Rock Mine
            performance = pgettext("frisians_building", "If the food supply is steady, the deep rock mine can produce three blocks of granite in %s on average."):bformat(ngettext("%d second", "%d seconds", 76):bformat(76))
         }
      },
      {
         name = "frisians_coalmine",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Coal Mine
            purpose = pgettext("building", "Digs coal out of the ground in mountain terrain."),
            -- TRANSLATORS: Performance helptext for a frisian production site: Coal Mine
            performance = pgettext("frisians_building", "If the food supply is steady, the coal mine can produce two pieces of coal in %s on average."):bformat(ngettext("%d second", "%d seconds", 85):bformat(85))
         }
      },
      {
         name = "frisians_coalmine_deep",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Deep Coal Mine
            purpose = pgettext("building", "Digs coal out of the ground in mountain terrain."),
            -- TRANSLATORS: Performance helptext for a frisian production site: Deep Coal Mine
            performance = pgettext("frisians_building", "If the food supply is steady, the deep coal mine can produce four pieces of coal in %s on average."):bformat(ngettext("%d second", "%d seconds", 76):bformat(76))
         }
      },
      {
         name = "frisians_ironmine",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Iron Mine
            purpose = pgettext("building", "Digs iron ore out of the ground in mountain terrain."),
            -- TRANSLATORS: Performance helptext for a frisian production site: Iron Mine
            performance = pgettext("frisians_building", "If the food supply is steady, the iron mine can produce one piece of iron ore in %s on average."):bformat(ngettext("%d second", "%d seconds", 65):bformat(65))
         }
      },
      {
         name = "frisians_ironmine_deep",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Deep Iron Mine
            purpose = pgettext("building", "Digs iron ore out of the ground in mountain terrain."),
            -- TRANSLATORS: Performance helptext for a frisian production site: Deep Iron Mine
            performance = pgettext("frisians_building", "If the food supply is steady, the deep iron mine can produce two pieces of iron ore in %s on average."):bformat(ngettext("%d second", "%d seconds", 76):bformat(76))
         }
      },
      {
         name = "frisians_goldmine",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Gold Mine
            purpose = pgettext("building", "Digs gold ore out of the ground in mountain terrain."),
            -- TRANSLATORS: Performance helptext for a frisian production site: Gold Mine
            performance = pgettext("frisians_building", "If the food supply is steady, the gold mine can produce one piece of gold ore in %s on average."):bformat(ngettext("%d second", "%d seconds", 65):bformat(65))
         }
      },
      {
         name = "frisians_goldmine_deep",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Deep Gold Mine
            purpose = pgettext("building", "Digs gold ore out of the ground in mountain terrain."),
            -- TRANSLATORS: Performance helptext for a frisian production site: Deep Gold Mine
            performance = pgettext("frisians_building", "If the food supply is steady, the deep gold mine can produce two pieces of gold ore in %s on average."):bformat(ngettext("%d second", "%d seconds", 76):bformat(76))
         }
      },

      -- Training Sites
      {
         name = "frisians_training_camp",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian training site: Training Camp
            lore = pgettext("frisians_building", "Just be quiet, listen carefully, and do try not to stab yourself until I’ve explained to you how to hold a broadsword."),
            -- TRANSLATORS: Lore author helptext for a frisian training site: Training Camp
            lore_author = pgettext("frisians_building", "A trainer training a soldier"),
            -- TRANSLATORS: Purpose helptext for a frisian training site: Training Camp
            purpose = pgettext("frisians_building", "Trains soldiers in Attack up to level 3 as well as in Defense and Health to level 1. Equips the soldiers with all necessary weapons and armor parts."),
            -- TRANSLATORS: Note helptext for a frisian training site: Training Camp
            note = pgettext("frisians_building", "Frisian soldiers cannot train in Evade and will remain at their initial level.")
         }
      },
      {
         name = "frisians_training_arena",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian training site: Training Arena
            lore = pgettext("frisians_building", "Now that you have two swords, there’s more of a risk you’ll accidentally stab yourself, but if you got this far, you’ll likely master this challenge as well."),
            -- TRANSLATORS: Lore author helptext for a frisian training site: Training Arena
            lore_author = pgettext("frisians_building", "A trainer training a soldier"),
            -- TRANSLATORS: Purpose helptext for a frisian training site: Training Arena
            purpose = pgettext("frisians_building", "Trains soldiers in Attack, Defense and Health to the final level. Equips the soldiers with all necessary weapons and armor parts."),
            -- TRANSLATORS: Note helptext for a frisian training site: Training Arena
            note = pgettext("frisians_building", "Trains only soldiers who have been trained to the maximum level by the Training Camp.")
         }
      },

      -- Military Sites
      {
         name = "frisians_wooden_tower",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a frisian military site: Wooden Tower
            purpose = pgettext("frisians_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for a frisian military site: Wooden Tower
            note = pgettext("frisians_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "frisians_wooden_tower_high",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a frisian military site: High Wooden Tower
            purpose = pgettext("frisians_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for a frisian military site: High Wooden Tower
            note = pgettext("frisians_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "frisians_sentinel",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a frisian military site: Sentinel
            purpose = pgettext("frisians_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for a frisian military site: Sentinel
            note = pgettext("frisians_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "frisians_outpost",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a frisian military site: Outpost
            purpose = pgettext("frisians_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for a frisian military site: Outpost
            note = pgettext("frisians_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "frisians_tower",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a frisian military site: Tower
            purpose = pgettext("frisians_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for a frisian military site: Tower
            note = pgettext("frisians_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "frisians_fortress",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a frisian military site: Fortress
            purpose = pgettext("frisians_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for a frisian military site: Fortress
            note = pgettext("frisians_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },

      -- Seafaring/Ferry Sites - these are only displayed on seafaring/ferry maps
      {
         name = "frisians_ferry_yard",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Ferry Yard
            purpose = pgettext("building", "Builds ferries."),
            -- TRANSLATORS: Note helptext for a frisian production site: Ferry Yard
            note = pgettext("building", "Needs water nearby.")
         }
      },
      {
         name = "frisians_shipyard",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Shipyard
            lore = pgettext("frisians_building", "This house may be called a shipyard, but my ships are rather longer than one yard!"),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Shipyard
            lore_author = pgettext("frisians_building", "A shipwright who only constructed toy ships after being chid that his ships were too small"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Shipyard
            purpose = pgettext("building", "Constructs ships that are used for overseas colonization and for trading between ports.")
         }
      },
      {
         name = "frisians_weaving_mill",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Weaving Mill
            lore = pgettext("frisians_building", "Reindeer’s fur and roofing reed<br>These items two make up the seed<br>For ships of wood to chain the gales<br>In sturdy, beautious, blowing sails!"),
            -- TRANSLATORS: Lore author helptext for a frisian production site: Weaving Mill
            lore_author = pgettext("frisians_building", "A seamstress’ work song"),
            -- TRANSLATORS: Purpose helptext for a frisian production site: Weaving Mill
            purpose = pgettext("building", "Sews cloth for ship sails out of reed and reindeer fur."),
            -- TRANSLATORS: Performance helptext for a frisian production site: Weaving Mill
            performance = pgettext("frisians_building", "The weaving mill needs %s on average to produce one piece of cloth."):bformat(ngettext("%d second", "%d seconds", 45):bformat(45))
         }
      },

      -- Partially Finished Buildings - these are the same 2 buildings for all tribes
      {
         name = "constructionsite",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian building: Construction Site
            lore = pgettext("building", "‘Don’t swear at the builder who is short of building materials.’"),
            -- TRANSLATORS: Lore author helptext for a frisian building: Construction Site
            lore_author = pgettext("building", "Proverb widely used for impossible tasks of any kind"),
            -- TRANSLATORS: Purpose helptext for a frisian building: Construction Site
            purpose = pgettext("building", "A new building is being built at this construction site.")
         }
      },
      {
         name = "dismantlesite",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a frisian building: Dismantle Site
            lore = pgettext("building", "‘New paths will appear when you are willing to tear down the old.’"),
            -- TRANSLATORS: Lore author helptext for a frisian building: Dismantle Site
            lore_author = pgettext("building", "Proverb"),
            -- TRANSLATORS: Purpose helptext for a frisian building: Dismantle Site
            purpose = pgettext("building", "A building is being dismantled at this dismantle site, returning some of the resources that were used during this building’s construction to your tribe’s stores.")
         }
      }
   },

   -- Productionsite status strings

   -- TRANSLATORS: Productivity label on a frisian building if there is 1 worker missing
   productionsite_worker_missing = pgettext("frisians", "Worker missing"),
   -- TRANSLATORS: Productivity label on a frisian building if there is 1 worker coming
   productionsite_worker_coming = pgettext("frisians", "Worker is coming"),
   -- TRANSLATORS: Productivity label on a frisian building if there is more than 1 worker missing. If you need plural forms here, please let us know.
   productionsite_workers_missing = pgettext("frisians", "Workers missing"),
   -- TRANSLATORS: Productivity label on a frisian building if there is more than 1 worker coming. If you need plural forms here, please let us know.
   productionsite_workers_coming = pgettext("frisians", "Workers are coming"),

   -- Special types
   builder = "frisians_builder",
   carriers = {"frisians_carrier", "frisians_reindeer"},
   geologist = "frisians_geologist",
   scouts_house = "frisians_scouts_house",
   soldier = "frisians_soldier",
   ship = "frisians_ship",
   ferry = "frisians_ferry",
   port = "frisians_port",

   toolbar = {
      bottom_left_corner  = image_dirname .. "toolbar_left.png",
      bottom_left         = image_dirname .. "toolbar_main.png",
      bottom_center       = image_dirname .. "toolbar_center.png",
      bottom_right        = image_dirname .. "toolbar_main.png",
      bottom_right_corner = image_dirname .. "toolbar_right.png",
      top_left_corner     = image_dirname .. "toolbar_left.png",
      top_left            = image_dirname .. "toolbar_main.png",
      top_center          = image_dirname .. "toolbar_center.png",
      top_right           = image_dirname .. "toolbar_main.png",
      top_right_corner    = image_dirname .. "toolbar_right.png",
   },

   fastplace = {
      warehouse = "frisians_warehouse",
      port = "frisians_port",
      training_small = "frisians_training_camp",
      training_large = "frisians_training_arena",
      military_small_primary = "frisians_sentinel",
      military_small_secondary = "frisians_wooden_tower",
      military_medium_primary = "frisians_outpost",
      military_tower = "frisians_tower",
      military_fortress = "frisians_fortress",
      woodcutter = "frisians_woodcutters_house",
      forester = "frisians_foresters_house",
      quarry = "frisians_quarry",
      building_materials_primary = "frisians_brick_kiln",
      building_materials_secondary = "frisians_clay_pit",
      building_materials_tertiary = "frisians_reed_farm",
      fisher = "frisians_fishers_house",
      hunter = "frisians_hunters_house",
      fish_meat_replenisher = "frisians_aqua_farm",
      well = "frisians_well",
      farm_primary = "frisians_farm",
      bakery = "frisians_bakery",
      brewery = "frisians_brewery",
      smokery = "frisians_smokery",
      tavern = "frisians_tavern",
      smelting = "frisians_furnace",
      tool_smithy = "frisians_blacksmithy",
      weapon_smithy = "frisians_armor_smithy_small",
      armor_smithy = "frisians_sewing_room",
      weaving_mill = "frisians_weaving_mill",
      shipyard = "frisians_shipyard",
      ferry_yard = "frisians_ferry_yard",
      scout = "frisians_scouts_house",
      barracks = "frisians_barracks",
      second_carrier = "frisians_reindeer_farm",
      charcoal = "frisians_charcoal_kiln",
      mine_stone = "frisians_rockmine",
      mine_coal = "frisians_coalmine",
      mine_iron = "frisians_ironmine",
      mine_gold = "frisians_goldmine",
      agriculture_producer = "frisians_berry_farm",
      agriculture_consumer_primary = "frisians_collectors_house",
      agriculture_consumer_secondary = "frisians_beekeepers_house",
      industry_alternative = "frisians_charcoal_burners_house",
      industry_supporter = "frisians_recycling_center",
      terraforming = "frisians_dikers_house",
   },
}

pop_textdomain()
