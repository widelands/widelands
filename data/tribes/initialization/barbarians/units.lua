descriptions = wl.Descriptions() -- TODO(matthiakl): only for savegame compatibility with 1.0, do not use.

image_dirname = path.dirname(__file__) .. "images/"

push_textdomain("tribes_encyclopedia")

-- For formatting time strings
include "tribes/scripting/help/time_strings.lua"

wl.Descriptions():new_tribe {
   name = "barbarians",
   animation_directory = image_dirname,
   animations = {
      frontier = { hotspot = {1, 19} },
      bridge_normal_e = { hotspot = {-1, 13} },
      bridge_busy_e = { hotspot = {-1, 13} },
      bridge_normal_se = { hotspot = {8, 3} },
      bridge_busy_se = { hotspot = {8, 3} },
      bridge_normal_sw = { hotspot = {41, 3} },
      bridge_busy_sw = { hotspot = {41, 3} }
   },
   spritesheets = {
      flag = {
         fps = 5,
         frames = 16,
         columns = 4,
         rows = 4,
         hotspot = { 11, 39 }
      }
   },

   bridge_height = 8,

   collectors_points_table = {
      { ware = "gold", points = 3},
      { ware = "ax", points = 2},
      { ware = "ax_sharp", points = 3},
      { ware = "ax_broad", points = 4},
      { ware = "ax_bronze", points = 4},
      { ware = "ax_battle", points = 6},
      { ware = "ax_warriors", points = 10},
      { ware = "helmet", points = 2},
      { ware = "helmet_mask", points = 3},
      { ware = "helmet_warhelm", points = 6},
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
         [0] = "barbarians_resi_none",
      },
      resource_coal = {
         [10] = "barbarians_resi_coal_1",
         [20] = "barbarians_resi_coal_2",
      },
      resource_iron = {
         [10] = "barbarians_resi_iron_1",
         [20] = "barbarians_resi_iron_2",
      },
      resource_gold = {
         [10] = "barbarians_resi_gold_1",
         [20] = "barbarians_resi_gold_2",
      },
      resource_stones = {
         [10] = "barbarians_resi_stones_1",
         [20] = "barbarians_resi_stones_2",
      },
      resource_water = {
         [100] = "barbarians_resi_water",
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
            default_target_quantity = 20,
            preciousness = 5,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a barbarian ware: Granite, part 1
                  pgettext("ware", "Granite is a basic building material."),
                  -- TRANSLATORS: Helptext for a barbarian ware: Granite, part 2
                  pgettext("barbarians_ware", "The Barbarians produce granite blocks in quarries and granite mines.")
               }
            }
         },
         {
            name = "log",
            preciousness = 14,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a barbarian ware: Log, part 1
                  pgettext("ware", "Logs are an important basic building material. They are produced by felling trees."),
                  -- TRANSLATORS: Helptext for a barbarian ware: Log, part 2
                  pgettext("barbarians_ware", "Barbarian lumberjacks fell the trees; rangers take care of the supply of trees. Logs are also used in the metal workshop to build basic tools, and in the charcoal kiln for the production of coal. The wood hardener refines logs into blackwood by hardening them with fire.")
               }
            }
         },
         {
            name = "blackwood",
            default_target_quantity = 40,
            preciousness = 10,
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian ware: Blackwood
               purpose = pgettext("barbarians_ware", "This fire-hardened wood is as hard as iron and it is used for several buildings. It is produced out of logs in the wood hardener.")
            }
         },
         {
            name = "grout",
            default_target_quantity = 10,
            preciousness = 5,
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian ware: Grout
               purpose = pgettext("barbarians_ware", "Granite can be processed into grout which provides a solid, non burning building material. Grout is produced in a lime kiln.")
            }
         },
         {
            name = "reed",
            preciousness = 5,
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian ware: Reed
               purpose = pgettext("barbarians_ware", "Reed is produced in a reed yard and used to make the roofs of buildings waterproof.")
            }
         },
         {
            name = "cloth",
            default_target_quantity = 10,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian ware: Cloth
               purpose = pgettext("barbarians_ware", "Cloth is needed for Barbarian ships. It is produced out of reed.")
            }
         }
      },
      {
         -- Food
         {
            name = "fish",
            preciousness = 3,
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian ware: Fish
               purpose = pgettext("barbarians_ware", "Besides pitta bread and meat, fish is also a foodstuff for the Barbarians. It is used in the taverns, inns and big inns and at the training sites (training camp and battle arena).")
            }
         },
         {
            name = "meat",
            preciousness = 3,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a barbarian ware: Meat, part 1
                  pgettext("ware", "Meat contains a lot of energy, and it is obtained from wild game taken by hunters."),
                  -- TRANSLATORS: Helptext for a barbarian ware: Meat, part 2
                  pgettext("barbarians_ware", "Meat is used in the taverns, inns and big inns to prepare rations, snacks and meals for the miners. It is also consumed at the training sites (training camp and battle arena).")
               }
            }
         },
         {
            name = "water",
            preciousness = 8,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a barbarian ware: Water, part 1
                  pgettext("ware", "Water is the essence of life!"),
                  -- TRANSLATORS: Helptext for a barbarian ware: Water, part 2
                  pgettext("barbarians_ware", "Water is used in the bakery, the micro brewery and the brewery. The lime kiln and the cattle farm also need to be supplied with water.")
               }
            }
         },
         {
            name = "wheat",
            preciousness = 12,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a barbarian ware: Wheat, part 1
                  pgettext("ware", "Wheat is essential for survival."),
                  -- TRANSLATORS: Helptext for a barbarian ware: Wheat, part 2
                  pgettext("barbarians_ware", "Wheat is produced by farms and consumed by bakeries, micro breweries and breweries. Cattle farms also need to be supplied with wheat.")
               }
            }
         },
         {
            name = "barbarians_bread",
            default_target_quantity = 20,
            preciousness = 4,
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian ware: Pitta Bread
               purpose = pgettext("barbarians_ware", "The Barbarian bakers are best in making this flat and tasty pitta bread. It is made out of wheat and water following a secret recipe. Pitta bread is used in the taverns, inns and big inns to prepare rations, snacks and meals. It is also consumed at training sites (training camp and battle arena).")
            }
         },
         {
            name = "beer",
            default_target_quantity = 15,
            preciousness =  2,
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian ware: Beer
               purpose = pgettext("barbarians_ware", "Beer is produced in micro breweries and used in inns and big inns to produce snacks.")
            }
         },
         {
            name = "beer_strong",
            default_target_quantity = 15,
            preciousness = 2,
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian ware: Strong Beer
               purpose = pgettext("barbarians_ware", "Only this beer is acceptable for the soldiers in a battle arena. Some say that the whole power of the Barbarians lies in this ale. It helps to train the soldiers’ evade level from 0 to 1 to 2. Strong beer is also used in big inns to prepare meals.")
            }
         },
         {
            name = "ration",
            default_target_quantity = 20,
            preciousness = 5,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a barbarian ware: Ration, part 1
                  pgettext("ware", "A small bite to keep miners strong and working. The scout also consumes rations on his scouting trips."),
                  -- TRANSLATORS: Helptext for a barbarian ware: Ration, part 2
                  pgettext("barbarians_ware", "Rations are produced in a tavern, an inn or a big inn out of fish or meat or pitta bread.")
               }
            }
         },
         {
            name = "snack",
            default_target_quantity = 15,
            preciousness = 5,
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian ware: Snack
               purpose = pgettext("barbarians_ware", "A bigger morsel than the ration to provide miners in deep mines. It is produced in an inn or a big inn out of fish/meat, pitta bread and beer.")
            }
         },
         {
            name = "meal",
            default_target_quantity = 10,
            preciousness = 5,
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian ware: Meal
               purpose = pgettext("barbarians_ware", "A meal is made out of pitta bread, strong beer and fish/meat in a big inn. This substantial food is exactly what workers in a deeper mine need.")
            }
         }
      },
      {
         -- Mining
         {
            name = "coal",
            default_target_quantity = 20,
            preciousness = 20,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a barbarian ware: Coal, part 1
                  pgettext("ware", "Coal is mined in coal mines or produced out of logs by a charcoal kiln."),
                  -- TRANSLATORS: Helptext for a barbarian ware: Coal, part 2
                  pgettext("barbarians_ware", "The fires of the Barbarians are usually fed with coal. Consumers are several buildings: lime kiln, smelting works, ax workshop, war mill, and helm smithy.")
               }
            }
         },
         {
            name = "iron_ore",
            default_target_quantity = 15,
            preciousness = 4,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a barbarian ware: Iron Ore, part 1
                  pgettext("default_ware", "Iron ore is mined in iron mines."),
                  -- TRANSLATORS: Helptext for a barbarian ware: Iron Ore, part 2
                  pgettext("barbarians_ware", "It is smelted in a smelting works to retrieve the iron.")
               }
            }
         },
         {
            name = "iron",
            default_target_quantity = 20,
            preciousness = 4,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a barbarian ware: Iron, part 1
                  pgettext("ware", "Iron is smelted out of iron ores."),
                  -- TRANSLATORS: Helptext for a barbarian ware: Iron, part 2
                  pgettext("barbarians_ware", "It is produced by the smelting works and used to produce weapons and tools in the metal workshop, ax workshop, war mill and helm smithy.")
               }
            }
         },
         {
            name = "gold_ore",
            default_target_quantity = 15,
            preciousness = 2,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a barbarian ware: Gold Ore, part 1
                  pgettext("ware", "Gold ore is mined in a gold mine."),
                  -- TRANSLATORS: Helptext for a barbarian ware: Gold Ore, part 2
                  pgettext("barbarians_ware", "Smelted in a smelting works, it turns into gold which is used as a precious building material and to produce weapons and armor.")
               }
            }
         },
         {
            name = "gold",
            default_target_quantity = 20,
            preciousness = 2,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a barbarian ware: Gold, part 1
                  pgettext("ware", "Gold is the most valuable of all metals, and it is smelted out of gold ore."),
                  -- TRANSLATORS: Helptext for a barbarian ware: Gold, part 2
                  pgettext("barbarians_ware", "Only very important things are embellished with gold. It is produced by the smelting works and used as a precious building material and to produce different axes (in the war mill) and different parts of armor (in the helm smithy).")
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
               -- TRANSLATORS: Helptext for a barbarian ware: Pick
               purpose = pgettext("barbarians_ware", "Picks are used by stonemasons and miners. They are produced in the metal workshop (but cease to be produced by the building if it is enhanced to an ax workshop and war mill).")
            }
         },
         {
            name = "felling_ax",
            default_target_quantity = 5,
            preciousness = 3,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a barbarian ware: Felling Ax, part 1
                  pgettext("ware", "The felling ax is the tool to chop down trees."),
                  -- TRANSLATORS: Helptext for a barbarian ware: Felling Ax, part 2
                  pgettext("barbarians_ware", "Felling axes are used by lumberjacks and produced in the metal workshop (but cease to be produced by the building if it is enhanced to an ax workshop and war mill).")
               }
            }
         },
         {
            name = "shovel",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a barbarian ware: Shovel, part 1
                  pgettext("ware", "Shovels are needed for the proper handling of plants."),
                  -- TRANSLATORS: Helptext for a barbarian ware: Shovel, part 2
                  pgettext("barbarians_ware", "Therefore the gardener and the ranger use them. Produced at the metal workshop (but cease to be produced by the building if it is enhanced to an ax workshop and war mill).")
               }
            }
         },
         {
            name = "hammer",
            default_target_quantity = 2,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a barbarian ware: Hammer, part 1
                  pgettext("ware", "The hammer is an essential tool."),
                  -- TRANSLATORS: Helptext for a barbarian ware: Hammer, part 2
                  pgettext("barbarians_ware", "Geologists, builders, blacksmiths and helmsmiths all need a hammer. Make sure you’ve always got some in reserve! They are one of the basic tools produced at the metal workshop (but cease to be produced by the building if it is enhanced to an ax workshop and war mill).")
               }
            }
         },
         {
            name = "fishing_rod",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a barbarian ware: Fishing Rod, part 1
                  pgettext("ware", "Fishing rods are needed by fishers to catch fish."),
                  -- TRANSLATORS: Helptext for a barbarian ware: Fishing Rod, part 2
                  pgettext("barbarians_ware", "They are one of the basic tools produced in a metal workshop (but cease to be produced by the building if it is enhanced to an ax workshop and war mill).")
               }
            }
         },
         {
            name = "hunting_spear",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a barbarian ware: Hunting Spear, part 1
                  pgettext("ware", "This spear is light enough to be thrown, but heavy enough to kill any animal in one blow. It is only used by hunters."),
                  -- TRANSLATORS: Helptext for a barbarian ware: Hunting Spear, part 2
                  pgettext("barbarians_ware", "Hunting spears are produced in the metal workshop (but cease to be produced by the building if it is enhanced to an ax workshop and war mill).")
               }
            }
         },
         {
            name = "scythe",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a barbarian ware: Scythe, part 1
                  pgettext("ware", "The scythe is the tool of the farmers."),
                  -- TRANSLATORS: Helptext for a barbarian ware: Scythe, part 2
                  pgettext("barbarians_ware", "Scythes are produced by the metal workshop (but cease to be produced by the building if it is enhanced to an ax workshop and war mill).")
               }
            }
         },
         {
            name = "bread_paddle",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a barbarian ware: Bread Paddle, part 1
                  pgettext("ware", "The bread paddle is the tool of the baker, each baker needs one."),
                  -- TRANSLATORS: Helptext for a barbarian ware: Bread Paddle, part 2
                  pgettext("barbarians_ware", "Bread paddles are produced in the metal workshop like all other tools (but cease to be produced by the building if it is enhanced to an ax workshop and war mill).")
               }
            }
         },
         {
            name = "kitchen_tools",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian ware: Kitchen Tools
               purpose = pgettext("barbarians_ware", "Kitchen tools are needed for preparing rations, snacks and meals. Be sure to have a metal workshop to produce this basic tool (but it ceases to be produced by the building if it is enhanced to an ax workshop and war mill).")
            }
         },
         {
            name = "fire_tongs",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for a barbarian ware: Fire Tongs, part 1
                  pgettext("ware", "Fire tongs are the tools for smelting ores."),
                  -- TRANSLATORS: Helptext for a barbarian ware: Fire Tongs, part 2
                  pgettext("barbarians_ware", "They are used in the smelting works and produced by the metal workshop (but they cease to be produced by the building if it is enhanced to an ax workshop and war mill).")
               }
            }
         }
      },
      {
         -- Weapons & Armor
         {
            name = "ax",
            default_target_quantity = 30,
            preciousness = 3,
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian ware: Ax
               purpose = pgettext("barbarians_ware", "The ax is the basic weapon of the Barbarians. All young soldiers are equipped with it.")
            }
         },
         {
            name = "ax_sharp",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian ware: Sharp Ax
               purpose = pgettext("barbarians_ware", "Young soldiers are proud to learn to fight with this powerful weapon. It is heavier and sharper than the ordinary ax. It is produced in ax workshops and war mills. In training camps, it is used – together with food – to train soldiers from attack level 0 to level 1.")
            }
         },
         {
            name = "ax_broad",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian ware: Broad Ax
               purpose = pgettext("barbarians_ware", "The broad ax is produced by the ax workshop and the war mill. It is used in the training camp – together with food – to train soldiers with a fundamental fighting knowledge from attack level 1 to attack level 2.")
            }
         },
         {
            name = "ax_bronze",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian ware: Bronze Ax
               purpose = pgettext("barbarians_ware", "The bronze ax is considered a weapon that is hard to handle. Only skilled soldiers can use it. It is produced at the war mill and used in the training camp – together with food – to train soldiers from attack level 2 to level 3.")
            }
         },
         {
            name = "ax_battle",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian ware: Battle Ax
               purpose = pgettext("barbarians_ware", "This is a dangerous weapon the Barbarians are able to produce. It is produced in the war mill. Only trained soldiers are able to wield such a weapon. It is used – together with food – in the training camp to train soldiers from attack level 3 to 4.")
            }
         },
         {
            name = "ax_warriors",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian ware: Warrior’s Ax
               purpose = pgettext("barbarians_ware", "The warrior’s ax is the most dangerous of all Barbarian weapons. Only a few soldiers ever were able to handle this huge and powerful ax. It is produced in a war mill and used – together with food – in a training camp to train soldiers from attack level 4 to level 5.")
            }
         },
         {
            name = "helmet",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian ware: Helmet
               purpose = pgettext("barbarians_ware", "A helmet is a basic tool to protect soldiers. It is produced in the helm smithy and used in the training camp – together with food – to train soldiers from health level 0 to level 1.")
            }
         },
         {
            name = "helmet_mask",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian ware: Mask
               purpose = pgettext("barbarians_ware", "A mask is an enhanced armor for Barbarian soldiers. It is produced in the helm smithy and used in the training camp – together with food – to train soldiers from health level 1 to level 2.")
            }
         },
         {
            name = "helmet_warhelm",
            default_target_quantity = 1,
            preciousness = 2,
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian ware: Warhelm
               purpose = pgettext("barbarians_ware", "This is the most enhanced Barbarian armor. It is produced in a helm smithy and used in a training camp – together with food – to train soldiers from health level 2 to level 3.")
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
            name = "barbarians_carrier",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Carrier
               purpose = pgettext("barbarians_worker", "Carries items along your roads.")
            }
         },
         {
            name = "barbarians_ferry",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Ferry
               purpose = pgettext("barbarians_worker", "Ships wares across narrow rivers.")
            }
         },
         {
            name = "barbarians_ox",
            default_target_quantity = 10,
            preciousness = 2,
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Ox
               purpose = pgettext("barbarians_worker", "Oxen help to carry items along busy roads. They are reared in a cattle farm.")
            }
         },
         {
            name = "barbarians_cattlebreeder",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Cattle Breeder
               purpose = pgettext("barbarians_worker", "Breeds strong oxen for adding them to the transportation system.")
            }
         }
      },
      {
         -- Building Materials
         {
            name = "barbarians_stonemason",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Stonemason
               purpose = pgettext("barbarians_worker", "Cuts raw pieces of granite out of rocks in the vicinity.")
            }
         },
         {
            name = "barbarians_lumberjack",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Lumberjack
               purpose = pgettext("barbarians_worker", "Fells trees.")
            }
         },
         {
            name = "barbarians_ranger",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Ranger
               purpose = pgettext("barbarians_worker", "Plants trees.")
            }
         },
         {
            name = "barbarians_builder",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Builder
               purpose = pgettext("barbarians_worker", "Works at construction sites to raise new buildings.")
            }
         },
         {
            name = "barbarians_lime_burner",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Lime-Burner
               purpose = pgettext("barbarians_worker", "Mixes grout out of granite, water and coal in the lime kiln.")
            }
         },
         {
            name = "barbarians_gardener",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Gardener
               purpose = pgettext("barbarians_worker", "Plants and harvests reed fields.")
            }
         },
         {
            name = "barbarians_weaver",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Weaver
               purpose = pgettext("barbarians_worker", "Produces cloth for ships’ sails.")
            }
         },
         {
            name = "barbarians_shipwright",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Shipwright
               purpose = pgettext("barbarians_worker", "Works at the shipyard and constructs new ships.")
            }
         }
      },
      {
         -- Food
         {
            name = "barbarians_fisher",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Fisher
               purpose = pgettext("barbarians_worker", "Catches fish in the sea.")
            }
         },
         {
            name = "barbarians_hunter",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Hunter
               purpose = pgettext("barbarians_worker", "The hunter brings fresh, raw meat to the colonists.")
            }
         },
         {
            name = "barbarians_gamekeeper",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Gamekeeper
               purpose = pgettext("barbarians_worker", "The gamekeeper makes sure that the natural population of animals never drops too low.")
            }
         },
         {
            name = "barbarians_farmer",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Farmer
               purpose = pgettext("barbarians_worker", "Plants fields.")
            }
         },
         {
            name = "barbarians_baker",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Baker
               purpose = pgettext("barbarians_worker", "Bakes pitta bread for the miners, soldiers and scouts.")
            }
         },
         {
            name = "barbarians_brewer",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Brewer
               purpose = pgettext("barbarians_worker", "Produces beer to keep the miners strong and happy.")
            }
         },
         {
            name = "barbarians_brewer_master",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Master Brewer
               purpose = pgettext("barbarians_worker", "Produces the finest ales to keep soldiers strong and happy in training.")
            }
         },
         {
            name = "barbarians_innkeeper",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Innkeeper
               purpose = pgettext("barbarians_worker", "Produces food for miners.")
            }
         }
      },
      {
         -- Mining
         {
            name = "barbarians_geologist",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Geologist
               purpose = pgettext("barbarians_worker", "Discovers resources for mining.")
            }
         },
         {
            name = "barbarians_miner",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Miner
               purpose = pgettext("barbarians_worker", "Works deep in the mines to obtain coal, iron, gold or granite.")
            }
         },
         {
            name = "barbarians_miner_chief",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Chief Miner
               purpose = pgettext("barbarians_worker", "Works deep in the mines to obtain coal, iron, gold or granite.")
            }
         },
         {
            name = "barbarians_miner_master",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Master Miner
               purpose = pgettext("barbarians_worker", "Works deep in the mines to obtain coal, iron, gold or granite.")
            }
         },
         {
            name = "barbarians_charcoal_burner",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Charcoal Burner
               purpose = pgettext("barbarians_worker", "Burns coal.")
            }
         },
         {
            name = "barbarians_smelter",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Smelter
               purpose = pgettext("barbarians_worker", "Smelts ores into metal.")
            }
         }
      },
      {
         -- Tools
         {
            name = "barbarians_blacksmith",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Blacksmith
               purpose = pgettext("barbarians_worker", "Produces weapons for soldiers and tools for workers.")
            }
         },
         {
            name = "barbarians_blacksmith_master",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Master Blacksmith
               purpose = pgettext("barbarians_worker", "Produces weapons for soldiers and tools for workers.")
            }
         }
      },
      {
         -- Military
         {
            name = "barbarians_recruit",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Recruit
               purpose = pgettext("barbarians_worker", "Eager to become a soldier and defend his tribe!")
            }
         },
         {
            name = "barbarians_soldier",
            default_target_quantity = 10,
            preciousness = 5,
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Soldier
               purpose = pgettext("barbarians_worker", "Defend and Conquer!")
            }
         },
         {
            name = "barbarians_trainer",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Trainer
               purpose = pgettext("barbarians_worker", "Trains the soldiers.")
            }
         },
         {
            name = "barbarians_helmsmith",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Helmsmith
               purpose = pgettext("barbarians_worker", "Forges helmets for soldiers.")
            }
         },
         {
            name = "barbarians_scout",
            helptexts = {
               -- TRANSLATORS: Helptext for a barbarian worker: Scout
               purpose = pgettext("barbarians_worker", "Scouts like Scotty the scout scouting unscouted areas in a scouty fashion.")
               -- (c) WiHack Team 02.01.2010
            }
         }
      }
   },

   immovables = {
      {
         name = "ashes",
         helptexts = {
            -- TRANSLATORS: Helptext for a barbarian immovable: Ashes
            purpose = _("The remains of a destroyed building.")
         }
      },
      {
         name = "destroyed_building",
         helptexts = {
            -- TRANSLATORS: Helptext for a barbarian immovable: Destroyed Building
            purpose = _("The remains of a destroyed building.")
         }
      },
      {
         name = "wheatfield_tiny",
         helptexts = {
            -- TRANSLATORS: Helptext for a barbarian immovable: Wheat field
            purpose = _("This field has just been planted.")
         }
      },
      {
         name = "wheatfield_small",
         helptexts = {
            -- TRANSLATORS: Helptext for a barbarian immovable: Wheat field
            purpose = _("This field is growing.")
         }
      },
      {
         name = "wheatfield_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for a barbarian immovable: Wheat field
            purpose = _("This field is growing.")
         }
      },
      {
         name = "wheatfield_ripe",
         helptexts = {
            -- TRANSLATORS: Helptext for a barbarian immovable: Wheat field
            purpose = _("This field is ready for harvesting.")
         }
      },
      {
         name = "wheatfield_harvested",
         helptexts = {
            -- TRANSLATORS: Helptext for a barbarian immovable: Wheat field
            purpose = _("This field has been harvested.")
         }
      },
      {
         name = "reedfield_tiny",
         helptexts = {
            -- TRANSLATORS: Helptext for a barbarian immovable: Reed Field
            purpose = _("This reed field has just been planted.")
         }
      },
      {
         name = "reedfield_small",
         helptexts = {
            -- TRANSLATORS: Helptext for a barbarian immovable: Reed Field
            purpose = _("This reed field is growing.")
         }
      },
      {
         name = "reedfield_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for a barbarian immovable: Reed Field
            purpose = _("This reed field is growing.")
         }
      },
      {
         name = "reedfield_ripe",
         helptexts = {
            -- TRANSLATORS: Helptext for a barbarian immovable: Reed Field
            purpose = _("This reed field is ready for harvesting.")
         }
      },
      {
         name = "barbarians_resi_none",
         helptexts = {
            -- TRANSLATORS: Helptext for a barbarian resource indicator: No resources
            purpose = _("There are no resources in the ground here.")
         }
      },
      {
         name = "barbarians_resi_water",
         helptexts = {
            -- TRANSLATORS: Helptext for a barbarian resource indicator: Water
            purpose = _("There is water in the ground here that can be pulled up by a well.")
         }
      },
      {
         name = "barbarians_resi_coal_1",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for a barbarian resource indicator: Coal, part 1
               _("Coal veins contain coal that can be dug up by coal mines."),
               -- TRANSLATORS: Helptext for a barbarian resource indicator: Coal part 2
               _("There is only a little bit of coal here.")
            }
         }
      },
      {
         name = "barbarians_resi_iron_1",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for a barbarian resource indicator: Iron, part 1
               _("Iron veins contain iron ore that can be dug up by iron mines."),
               -- TRANSLATORS: Helptext for a barbarian resource indicator: Iron, part 2
               _("There is only a little bit of iron here.")
            }
         }
      },
      {
         name = "barbarians_resi_gold_1",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for a barbarian resource indicator: Gold, part 1
               _("Gold veins contain gold ore that can be dug up by gold mines."),
               -- TRANSLATORS: Helptext for a barbarian resource indicator: Gold, part 2
               _("There is only a little bit of gold here.")
            }
         }
      },
      {
         name = "barbarians_resi_stones_1",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for a Barbarian resource indicator: Stones, part 1
               _("Granite is a basic building material and can be dug up by a granite mine."),
               -- TRANSLATORS: Helptext for a Barbarian resource indicator: Stones, part 2
               _("There is only a little bit of granite here."),
            }
         }
      },
      {
         name = "barbarians_resi_coal_2",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for a barbarian resource indicator: Coal, part 1
               _("Coal veins contain coal that can be dug up by coal mines."),
               -- TRANSLATORS: Helptext for a barbarian resource indicator: Coal, part 2
               _("There is a lot of coal here.")
            }
         }
      },
      {
         name = "barbarians_resi_iron_2",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for a barbarian resource indicator: Iron, part 1
               _("Iron veins contain iron ore that can be dug up by iron mines."),
               -- TRANSLATORS: Helptext for a barbarian resource indicator: Iron, part 2
               _("There is a lot of iron here.")
            }
         }
      },
      {
         name = "barbarians_resi_gold_2",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for a barbarian resource indicator: Gold, part 1
               _("Gold veins contain gold ore that can be dug up by gold mines."),
               -- TRANSLATORS: Helptext for a barbarian resource indicator: Gold, part 2
               _("There is a lot of gold here.")
            }
         }
      },
      {
         name = "barbarians_resi_stones_2",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for a Barbarian resource indicator: Stones, part 1
               _("Granite is a basic building material and can be dug up by a granite mine."),
               -- TRANSLATORS: Helptext for a Barbarian resource indicator: Stones, part 2
               _("There is a lot of granite here.")
            }
         }
      },
      {
         name = "barbarians_shipconstruction",
         helptexts = {
            -- TRANSLATORS: Helptext for a barbarian immovable: Ship Under Construction
            purpose = _("A ship is being constructed at this site.")
         }
      },
      -- non barbarian Immovables used by the woodcutter
      {
         name = "deadtree7",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by barbarians: Dead Tree
            purpose = _("The remains of an old tree.")
         }
      },
      {
         name = "balsa_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by barbarians: Balsa Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_black_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by barbarians: Balsa Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_desert_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by barbarians: Balsa Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "balsa_winter_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by barbarians: Blackroot Field
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by barbarians: Balsa Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_black_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by barbarians: Ironwood Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_desert_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by barbarians: Ironwood Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "ironwood_winter_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by barbarians: Ironwood Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by barbarians: Rubber Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_black_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by barbarians: Rubber Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_desert_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by barbarians: Rubber Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
      {
         name = "rubber_winter_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an amazon immovable usable by barbarians: Rubber Tree
            purpose = _("This tree is only planted by the amazon tribe but can be harvested for logs.")
         }
      },
   },

   -- The order here also determines the order in lists on screen.
   buildings = {
      -- Warehouses
      {
         name = "barbarians_headquarters",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian warehouse: Headquarters
            lore = pgettext("barbarians_building", "‘Steep like the slopes of Kal’mavrath, shiny like the most delicate armor and strong like our ancestors, that’s how the headquarters of Chat’Karuth presented itself to us.’"),
            -- TRANSLATORS: Lore author helptext for a barbarian warehouse: Headquarters
            lore_author = pgettext("barbarians_building", "Ballad ‘The Battle of Kal’mavrath’ by Hakhor the Bard"),
            -- TRANSLATORS: Purpose helptext for a barbarian warehouse: Headquarters
            purpose = pgettext("barbarians_building", "Accommodation for your people. Also stores your wares and tools."),
            -- TRANSLATORS: Note helptext for a barbarian warehouse: Headquarters
            note = pgettext("barbarians_building", "The headquarters is your main building.")
         }
      },
      {
         name = "barbarians_headquarters_interim",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for a barbarian warehouse: Headquarters
            purpose = pgettext("barbarians_building", "Accommodation for your people. Also stores your wares and tools."),
            -- TRANSLATORS: Note helptext for a barbarian warehouse: Headquarters
            note = pgettext("barbarians_building", "The headquarters is your main building.")
         }
      },
      {
         name = "barbarians_warehouse",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian warehouse: Warehouse
            lore = pgettext("barbarians_building", "‘Who still owns a warehouse is not yet defeated!’"),
            -- TRANSLATORS: Lore author helptext for a barbarian warehouse: Warehouse
            lore_author = pgettext("barbarians_building", "Berthron, chief military adviser of Chat’Karuth,<br>when they lost the headquarters in the battle around the heights of Kal’Megarath"),
            -- TRANSLATORS: Purpose helptext for a barbarian warehouse: Warehouse
            purpose = pgettext("barbarians_building", "Your workers and soldiers will find shelter here. Also stores your wares and tools.")
         }
      },
      {
         name = "barbarians_port",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian warehouse: Port
            lore = pgettext("barbarians_building", "‘I prefer the planks of a ship to any fortress, no matter how strong it is.’"),
            -- TRANSLATORS: Lore author helptext for a barbarian warehouse: Port
            lore_author = pgettext("barbarians_building", "Captain Thanlas the Elder,<br>Explorer"),
            -- TRANSLATORS: Purpose helptext for a barbarian warehouse: Port
            purpose = pgettext("barbarians_building", "Serves as a base for overseas colonization and trade. Also stores your soldiers, wares and tools."),
            -- TRANSLATORS: Note helptext for an barbarian warehouse: Port
            note = pgettext("barbarians_building", "Similar to the Headquarters a Port can be attacked and destroyed by an enemy. It is recommendable to send soldiers to defend it.")
         }
      },

      -- Small
      {
         name = "barbarians_quarry",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Quarry
            lore = pgettext("barbarians_building", "‘We open up roads and make houses from mountains.’"),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Quarry
            lore_author = pgettext("barbarians_building", "Slogan of the stonemasons’ guild"),
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Quarry
            purpose = pgettext("barbarians_building", "Cuts raw pieces of granite out of rocks in the vicinity."),
            -- TRANSLATORS: Note helptext for a barbarian production site: Quarry
            note = pgettext("barbarians_building", "The quarry needs rocks to cut within the work area."),
            -- TRANSLATORS: Performance helptext for a barbarian production site: Quarry
            performance = pgettext("barbarians_building", "The stonemason pauses %s before going back to work again."):bformat(ngettext("%d second", "%d seconds", 65):bformat(65))
         }
      },
      {
         name = "barbarians_lumberjacks_hut",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Lumberjack_s Hut
            lore = pgettext("barbarians_building", "‘Take 200 hits to fell a tree and you’re a baby. Take 100 and you’re a soldier. Take 50 and you’re a hero. Take 20 and soon you will be a honorable lumberjack.’"),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Lumberjack_s Hut
            lore_author = pgettext("barbarians_building", "Krumta, carpenter of Chat’Karuth"),
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Lumberjack_s Hut
            purpose = pgettext("building", "Fells trees in the surrounding area and processes them into logs."),
            -- TRANSLATORS: Note helptext for a barbarian production site: Lumberjack_s Hut
            note = pgettext("barbarians_building", "The lumberjack’s hut needs trees to fell within the work area."),
            -- TRANSLATORS: Performance helptext for a barbarian production site: Lumberjack_s Hut
            performance = pgettext("barbarians_building", "The lumberjack needs %s to fell a tree, not counting the time he needs to reach the destination and go home again."):bformat(ngettext("%d second", "%d seconds", 12):bformat(12))
         }
      },
      {
         name = "barbarians_rangers_hut",
         helptexts = {
            --"Wer zwei Halme dort wachsen lässt, wo sonst nur einer wächst, der ist größer als der größte Feldherr!" – Friedrich der Große
            -- TRANSLATORS: Lore helptext for a barbarian production site: Ranger's Hut
            lore = pgettext("barbarians_building", "‘He who can grow two trees where normally only one will grow exceeds the most important general!’"),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Ranger's Hut
            lore_author = pgettext("barbarians_building", "Chat’Karuth in a conversation with a Ranger"),
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Ranger's Hut
            purpose = pgettext("building", "Plants trees in the surrounding area."),
            -- TRANSLATORS: Note helptext for a barbarian production site: Ranger's Hut
            note = pgettext("barbarians_building", "The ranger’s hut needs free space within the work area to plant the trees."),
            -- TRANSLATORS: Performance helptext for a barbarian production site: Ranger's Hut
            performance = pgettext("barbarians_building", "The ranger needs %s to plant a tree, not counting the time he needs to reach the destination and go home again."):bformat(ngettext("%d second", "%d seconds", 5):bformat(5))
         }
      },
      {
         name = "barbarians_fishers_hut",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Fisher's Hut
            lore = pgettext("barbarians_building", "‘What do you mean, my fish ain’t fresh?!’"),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Fisher's Hut
            lore_author = pgettext("barbarians_building", "Frequent response of a barbarian fisherman,<br>often followed by an enjoyable brawl"),
            -- TRANSLATORS: Purpose helptext for production site: Fisher's Hut
            purpose = pgettext("barbarians_building", "Fishes on the coast near the fisher’s hut."),
            -- TRANSLATORS: Note helptext for production site: Fisher's Hut
            note = pgettext("barbarians_building", "The fisher’s hut needs water full of fish within the work area."),
            -- TRANSLATORS: Performance helptext for production site: Fisher's Hut
            performance = pgettext("barbarians_building", "The fisher pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 18):bformat(18))
         }
      },
      {
         name = "barbarians_hunters_hut",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Hunter's Hut
            lore = pgettext("barbarians_building", "‘As silent as a panther,<br> as deft as a weasel,<br> as swift as an arrow,<br> as deadly as a viper.’"),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Hunter's Hut
            lore_author = pgettext("barbarians_building", "‘The Art of Hunting’"),
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Hunter's Hut
            purpose = pgettext("building", "Hunts animals to produce meat."),
            -- TRANSLATORS: Note helptext for a barbarian production site: Hunter's Hut
            note = pgettext("barbarians_building", "The hunter’s hut needs animals to hunt within the work area."),
            -- TRANSLATORS: Performance helptext for a barbarian production site: Hunter's Hut
            performance = pgettext("barbarians_building", "The hunter pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 35):bformat(35))
         }
      },
      {
         name = "barbarians_gamekeepers_hut",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Gamekeeper's Hut
            lore = pgettext("barbarians_building", "‘He loves the animals and to breed them<br>as we love to cook and eat them.’"),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Gamekeeper's Hut
            lore_author = pgettext("barbarians_building", "Barbarian nursery rhyme"),
            -- TRANSLATORS: Purpose helptext for production site: Gamekeeper's Hut
            purpose = pgettext("barbarians_building", "Releases animals into the wild to steady the meat production."),
            -- TRANSLATORS: Performance helptext for production site: Gamekeeper's Hut
            performance = pgettext("barbarians_building", "The gamekeeper pauses 52.5 seconds before going to work again.")
         }
      },
      {
         name = "barbarians_well",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Well
            lore = pgettext("barbarians_building", [[‘Oh how sweet is the source of life,<br> that comes down from the sky <br> and lets the earth drink.’]]),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Well
            lore_author = pgettext("barbarians_building", "Song written by Sigurd the Bard when the first rain fell after the Great Drought in the 21ˢᵗ year of Chat’Karuth’s reign."),
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Well
            purpose = pgettext("building", "Draws water out of the deep."),
            -- TRANSLATORS: Performance helptext for a barbarian production site: Well
            performance = pgettext("barbarians_building", "The carrier needs %s to get one bucket full of water."):bformat(ngettext("%d second", "%d seconds", 40):bformat(40))
         }
      },
      {
         name = "barbarians_scouts_hut",
         helptexts = {
            no_scouting_building_connected = pgettext("barbarians_building", "You need to connect this flag to a scout’s hut before you can send a scout here."),
            -- TRANSLATORS: Lore helptext for a barbarian production site: Scout's Hut
            lore = pgettext("barbarians_building", "‘Behind the next hill there might be wealth and happiness but also hostility and doom.<br>He who will not explore it commits the crime of stupidity.’"),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Scout's Hut
            lore_author = pgettext("barbarians_building", "Chat’Karuth<br>at the oath taking ceremony of the first scout troupe"),
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Scout's Hut
            purpose = pgettext("building", "Explores unknown territory.")
         }
      },

      -- Medium
      {
         name = "barbarians_wood_hardener",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Wood Hardener
            lore = pgettext("barbarians_building", [[‘Then he hit the door with his ax and, behold,<br>]] ..
                                          [[the door only trembled, but the shaft of the ax burst into a thousand pieces.’]]),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Wood Hardener
            lore_author = pgettext("barbarians_building", "Legend about a quarrel between the brothers Harthor and Manthor,<br>Manthor is said to be the inventor of blackwood"),
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Wood Hardener
            purpose = pgettext("barbarians_building", "Hardens logs by fire into blackwood, which is used to construct robust buildings.")
         }
      },
      {
         name = "barbarians_reed_yard",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Reed Yard
            lore = pgettext("barbarians_building", "‘We grow roofs’"),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Reed Yard
            lore_author = pgettext("barbarians_building", "Slogan of the Guild of Gardeners"),
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Reed Yard
            purpose = pgettext("barbarians_building", "Cultivates reed that serves two different purposes for the Barbarian tribe."),
            -- TRANSLATORS: Note helptext for a barbarian production site: Reed Yard
            note = pgettext("barbarians_building", "Reed is the traditional material for roofing, and it is woven into the extremely durable cloth that they use for their ships’ sails."),
            -- TRANSLATORS: Performance helptext for a barbarian production site: Reed Yard
            performance = pgettext("barbarians_building", "A reed yard can produce a sheaf of reed in about %s on average."):bformat(ngettext("%d second", "%d seconds", 65):bformat(65))
         }
      },
      {
         name = "barbarians_lime_kiln",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Lime Kiln
            lore = pgettext("barbarians_building", "‘Forming new stone from old with fire and water.’"),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Lime Kiln
            lore_author = pgettext("barbarians_building", "Ragnald the Child’s answer to the question, what he’s doing in the dirt. His ‘new stone’ now is an important building material."),
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Lime Kiln
            purpose = pgettext("barbarians_building", "Processes granite to make ‘grout’, a substance that solidifies and so reinforces masonry."),
            -- TRANSLATORS: Note helptext for a barbarian production site: Lime Kiln
            note = pgettext("barbarians_building", "The lime kiln’s output will only go to construction sites that need it. Those are predominantly houses that work with fire, and some military sites."),
            -- TRANSLATORS: Performance helptext for a barbarian production site: Lime Kiln
            performance = pgettext("barbarians_building", "If all needed wares are delivered in time, this building can produce grout in about %s on average."):bformat(ngettext("%d second", "%d seconds", 41):bformat(41))
         }
      },
      {
         name = "barbarians_bakery",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Bakery
            lore = pgettext("barbarians_building", "‘He who has enough bread will never be too tired to dig the ore and wield the ax.’"),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Bakery
            lore_author = pgettext("barbarians_building", "Khantarakh, ‘The Modern Barbarian Economy’,<br>3ʳᵈ cowhide ‘Craftsmanship and Trade’"),
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Bakery
            purpose = pgettext("barbarians_building", "Bakes pitta bread for soldiers and miners alike."),
            -- TRANSLATORS: Performance helptext for a barbarian production site: Bakery
            performance = pgettext("barbarians_building", "If all needed wares are delivered in time, this building can produce a pitta bread in %s on average."):bformat(ngettext("%d second", "%d seconds", 30):bformat(30))
         }
      },
      {
         name = "barbarians_micro_brewery",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Micro Brewery
            lore = pgettext("barbarians_building", "‘Let the first one drive away the hunger, the second one put you at ease; when you have swallowed up your third one, it’s time for the next shift!’"),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Micro Brewery
            lore_author = pgettext("barbarians_building", "Widespread toast among miners"),
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Micro Brewery
            purpose = pgettext("barbarians_building", "Produces beer of the lower grade. This beer is a vital component of the snacks that inns and big inns prepare for miners in deep mines."),
            -- TRANSLATORS: Performance helptext for a barbarian production site: Micro Brewery
            performance = pgettext("barbarians_building", "If all needed wares are delivered in time, this building can produce beer in about %s on average."):bformat(ngettext("%d second", "%d seconds", 60):bformat(60))
         }
      },
      {
         name = "barbarians_brewery",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Brewery
            lore = pgettext("barbarians_building", [[‘When the Barbarian was created,<br>]] ..
                                          [[this was the first thing that he stated:<br>]] ..
                                          [[“You want me fighting without fear?<br>]] ..
                                          [[Then you should create some beer!”’]]),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Brewery
            lore_author = pgettext("barbarians_building", "First verse of the drinking song ‘Way of the Barbarian’"),
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Brewery
            purpose = pgettext("building", "Produces beer to keep the miners strong and happy.")
         }
      },
      {
         name = "barbarians_tavern",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Tavern
            lore = pgettext("barbarians_building", "‘Fresh from the countryside: Bread, fish or game to go.’"),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Tavern
            lore_author = pgettext("barbarians_building", "Menu of the tavern ‘Ye Ol’ Bronze Axe’"),
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Tavern
            purpose = pgettext("building", "Prepares rations to feed the scouts and miners.")
         }
      },
      {
         name = "barbarians_inn",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Inn
            lore = pgettext("barbarians_building", "‘Miner’s Delight: Pitta bread with fish or meat, comes with one beer.’"),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Inn
            lore_author = pgettext("barbarians_building", "Chef’s recommendation in ‘The Singing Master Miner’"),
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Inn
            purpose = pgettext("building", "Prepares rations for scouts and rations and snacks to feed the miners in the basic and deep mines.")
         }
      },
      {
         name = "barbarians_big_inn",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Big Inn
            lore = pgettext("barbarians_building", "‘Seasonal fish and game specialties served with pitta bread.<br>Choose from a prime selection of regional craft beer brands.’"),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Big Inn
            lore_author = pgettext("barbarians_building", "Menu of ‘The Laughing Barbarian’"),
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Big Inn
            purpose = pgettext("barbarians_building", "Prepares rations for scouts and rations, snacks and meals to feed all miners.")
         }
      },
      {
         name = "barbarians_charcoal_kiln",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Charcoal Kiln
            lore = pgettext("barbarians_building", [[‘What’s the work of a charcoal burner?<br>]] ..
                                          [[He’s the tribe’s most plodding earner!<br>]] ..
                                          [[Logs on logs he piles up high,<br>]] ..
                                          [[Until the kiln will reach the sky.<br>]] ..
                                          [[He sets the fire, sees it smolder<br>]] ..
                                          [[The logs he carried on his shoulder.<br>]] ..
                                          [[Burnt down to just one single coal<br>]] ..
                                          [[This troubles much a burner’s soul.’]]),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Charcoal Kiln
            lore_author = pgettext("barbarians_building", "Song of the charcoal burners"),
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Charcoal Kiln
            purpose = pgettext("building", "Burns logs into charcoal.")
         }
      },
      {
         name = "barbarians_smelting_works",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Smelting Works
            lore = pgettext("barbarians_building", [[‘From inferior looking rubble they form shiny gold and gleaming iron.<br>]] ..
                                          [[The smelters are real magicians among our craftsmen.’]]),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Smelting Works
            lore_author = pgettext("barbarians_building", "Chat’Karuth<br>at the 5ᵗʰ conference of master craftsmen"),
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Smelting Works
            purpose = pgettext("building", "Smelts iron ore into iron and gold ore into gold.")
         }
      },
      {
         name = "barbarians_metal_workshop",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Metal Workshop
            lore = pgettext("barbarians_building", "‘We make it work!’"),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Metal Workshop
            lore_author = pgettext("barbarians_building", "Inscription on the threshold of the now ruined Olde Forge at Harradsheim, the eldest known smithy."),
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Metal Workshop
            purpose = pgettext("building", "Forges all the tools that your workers need."),
            -- TRANSLATORS: Note helptext for a barbarian production site: Metal Workshop
            note = pgettext("barbarians_building", "The Barbarian metal workshop is the basic production site in a series of three buildings and creates all the tools that Barbarians need. The others are for weapons."),
            -- TRANSLATORS: Performance helptext for a barbarian production site: Metal Workshop
            performance = pgettext("barbarians_building", "If all needed wares are delivered in time, this building can produce each tool in about %s on average."):bformat(ngettext("%d second", "%d seconds", 67):bformat(67))
         }
      },
      {
         name = "barbarians_warmill",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Warmill
            lore = pgettext("barbarians_building", "‘A new warrior’s ax brings forth the best in its wielder – or the worst in its maker.’"),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Warmill
            lore_author = pgettext("barbarians_building", "An old Barbarian proverb<br> meaning that you need to take some risks sometimes."),
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Warmill
            purpose = pgettext("barbarians_building", "Produces all the axes that the Barbarians use for warfare."),
            -- TRANSLATORS: Note helptext for a barbarian production site: Warmill
            note = pgettext("barbarians_building", "The Barbarian war mill is their most advanced production site for weapons. As such you will need to enhance an ax workshop in order to get one."),
            performance = {
               -- TRANSLATORS: Performance helptext for a barbarian production site: Warmill, part 1
               pgettext("barbarians_building", "If all needed wares are delivered in time, this building can produce each type of ax in about %s on average."):bformat(ngettext("%d second", "%d seconds", 57):bformat(57)),
               -- TRANSLATORS: Performance helptext for a barbarian production site: Warmill, part 2
               pgettext("barbarians_building", "All weapons require the same time for making, but the consumption of the same raw materials increases from step to step. The last two need gold.")
            }
         }
      },
      {
         name = "barbarians_ax_workshop",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Ax Workshop
            lore = pgettext("barbarians_building", "‘A new warrior’s ax brings forth the best in its wielder – or the worst in its maker.’"),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Ax Workshop
            lore_author = pgettext("barbarians_building", "An old Barbarian proverb<br> meaning that you need to take some risks sometimes."),
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Ax Workshop
            purpose = pgettext("barbarians_building", "Produces axes, sharp axes and broad axes."),
            -- TRANSLATORS: Note helptext for a barbarian production site: Ax Workshop
            note = pgettext("barbarians_building", "The Barbarian ax workshop is the intermediate production site in a series of three buildings. It is enhanced from the metal workshop but doesn’t require additional qualification for the worker."),
            performance = {
               -- TRANSLATORS: Performance helptext for a barbarian production site: Ax Workshop
               pgettext("barbarians_building", "If all needed wares are delivered in time, this building can produce each type of ax in about %s on average."):bformat(ngettext("%d second", "%d seconds", 57):bformat(57)),
               -- TRANSLATORS: Performance helptext for a barbarian production site: Ax Workshop
               pgettext("barbarians_building", "All three weapons take the same time for making, but the required raw materials vary.")
            }
         }
      },
      {
         name = "barbarians_barracks",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Barracks
            lore = pgettext("barbarians_building", "‘Don’t ask what your tribe can do for you, ask what you can do for your tribe!’"),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Barracks
            lore_author = pgettext("barbarians_building", "Famous barbarian recruitment poster"),
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Barracks
            purpose = pgettext("barbarians_building", "Equips recruits and trains them as soldiers.")
         }
      },

      -- Big
      {
         name = "barbarians_cattlefarm",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Cattle Farm
            lore = pgettext("barbarians_building", "‘The smart leader builds roads, while the really wise leader breeds cattle.’"),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Cattle Farm
            lore_author = pgettext("barbarians_building", "Khantarakh, ‘The Modern Barbarian Economy’,<br> 5ᵗʰ cowhide ‘Traffic and Logistics’"),
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Cattle Farm
            purpose = pgettext("barbarians_building", "Breeds strong oxen for adding them to the transportation system."),
            -- TRANSLATORS: Performance helptext for a barbarian production site: Cattle Farm
            performance = pgettext("barbarians_building", "If all needed wares are delivered in time, this building can produce an ox in %s on average."):bformat(ngettext("%d second", "%d seconds", 30):bformat(30))
         }
      },
      {
         name = "barbarians_farm",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Farm
            lore = pgettext("barbarians_building", [[‘See the crop fields from here to the horizons. They are a huge, heaving, golden sea.<br>]] ..
                  [[Oh wheat, source of wealth, soul of beer, strength of our warriors!’]]),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Farm
            lore_author = pgettext("barbarians_building", "Line from the harvesting song ‘The Wealth of the Fields’"),
            -- TRANSLATORS: Purpose helptext for production site: Farm
            purpose = pgettext("building", "Sows and harvests wheat."),
            -- TRANSLATORS: Performance helptext for production site: Farm
            performance = pgettext("barbarians_building", "The farmer needs %1% on average to sow and harvest a sheaf of wheat."):bformat(format_minutes_seconds(1, 40))
         }
      },
      {
         name = "barbarians_helmsmithy",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Helm Smithy
            lore = pgettext("barbarians_building", "‘The helmets forgéd by this smithy<br>Are widely known for looking pithy.’"),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Helm Smithy
            lore_author = pgettext("barbarians_building", "Slogan of ‘Harrath’s Handsome Helmets’ in Al’thunran"),
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Helm Smithy
            purpose = pgettext("barbarians_building", "Forges helmets that are used for training soldiers’ health in the training camp.")
         }
      },

      -- Mines
      {
         name = "barbarians_granitemine",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Granite Mine
            lore = pgettext("barbarians_building", "‘I can handle tons of granite, man, but no more of your vain prattle.’"),
            lore_author = {
               -- TRANSLATORS: Lore author helptext for a barbarian production site: Granite Mine, part 1
               pgettext("barbarians_building", "This phrase was the reply Rimbert the miner – later known as Rimbert the loner – gave, when he was asked to remain seated on an emergency meeting at Stonford in the year of the great flood."),
               -- TRANSLATORS: Lore author helptext for a barbarian production site: Granite Mine, part 2
               pgettext("barbarians_building", "The same man had all the 244 granite blocks ready only a week later, and they still fortify the city’s levee.")
            },
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Granite Mine
            purpose = pgettext("barbarians_building", "Carves granite out of the rock in mountain terrain."),
            note = {
               -- TRANSLATORS: 'It' is a mine
               pgettext("barbarians_building", "It cannot be enhanced.")
            },
            -- TRANSLATORS: Performance helptext for a barbarian production site: Granite Mine
            performance = pgettext("barbarians_building", "If the food supply is steady, this mine can produce granite in %s on average."):bformat(ngettext("%d second", "%d seconds", 20):bformat(20))
         }
      },
      {
         name = "barbarians_coalmine",
         helptexts = {
            lore = {
               -- TRANSLATORS: Lore helptext for production site: Coal Mine, part 1
               pgettext("barbarians_building", "Ages ago, the Barbarians learned to delve into mountainsides for that black material that feeds their furnaces."),
               -- TRANSLATORS: Lore helptext for production site: Coal Mine, part 2
               pgettext("barbarians_building", "Wood may serve for a household fire and to keep you warm, but when it comes to working with iron or gold, there is no way around coal.")
            },
            -- TRANSLATORS: Purpose helptext for production site: Coal Mine
            purpose = pgettext("building", "Digs coal out of the ground in mountain terrain."),
            -- TRANSLATORS: Performance helptext for production site: Coal Mine
            performance = pgettext("barbarians_building", "If the food supply is steady, this mine can produce coal in 32.5 seconds on average.")
         }
      },
      {
         name = "barbarians_coalmine_deep",
         helptexts = {
            lore = {
               -- TRANSLATORS: Lore helptext for production site: Deep Coal Mine, part 1
               pgettext("barbarians_building", "Ages ago, the Barbarians learned to delve into mountainsides for that black material that feeds their furnaces."),
               -- TRANSLATORS: Lore helptext for production site: Deep Coal Mine
               pgettext("barbarians_building", "Wood may serve for a household fire and to keep you warm, but when it comes to working with iron or gold, there is no way around coal.")
            },
            -- TRANSLATORS: Purpose helptext for production site: Deep Coal Mine
            purpose = pgettext("building", "Digs coal out of the ground in mountain terrain."),
            -- TRANSLATORS: Performance helptext for production site: Deep Coal Mine
            performance = pgettext("barbarians_building", "If the food supply is steady, this mine can produce coal in 19.5 seconds on average.")
         }
      },
      {
         name = "barbarians_coalmine_deeper",
         helptexts = {
            lore = {
               -- TRANSLATORS: Lore helptext for production site: Deeper Coal Mine, part 1
               pgettext("barbarians_building", "Ages ago, the Barbarians learned to delve into mountainsides for that black material that feeds their furnaces."),
               -- TRANSLATORS: Lore helptext for production site: Deeper Coal Mine, part 2
               pgettext("barbarians_building", "Wood may serve for a household fire and to keep you warm, but when it comes to working with iron or gold, there is no way around coal.")
            },
            -- TRANSLATORS: Purpose helptext for production site: Deeper Coal Mine
            purpose = pgettext("building", "Digs coal out of the ground in mountain terrain."),
            -- TRANSLATORS: Performance helptext for production site: Deeper Coal Mine
            performance = pgettext("barbarians_building", "If the food supply is steady, this mine can produce coal in 14.4 seconds on average.")
         }
      },
      {
         name = "barbarians_ironmine",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Iron Mine
            lore = pgettext("barbarians_building", "‘I look at my own pick wearing away day by day and I realize why my work is important.’"),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Iron Mine
            lore_author = pgettext("barbarians_building", "Quote from an anonymous miner."),
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Iron Mine
            purpose = pgettext("building", "Digs iron ore out of the ground in mountain terrain."),
            -- TRANSLATORS: Performance helptext for a barbarian production site: Iron Mine
            performance = pgettext("barbarians_building", "If the food supply is steady, this mine can produce iron ore in %s on average."):bformat(ngettext("%d second", "%d seconds", 65):bformat(65))
         }
      },
      {
         name = "barbarians_ironmine_deep",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Deep Iron Mine
            lore = pgettext("barbarians_building", "‘I look at my own pick wearing away day by day and I realize why my work is important.’"),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Deep Iron Mine
            lore_author = pgettext("barbarians_building", "Quote from an anonymous miner."),
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Deep Iron Mine
            purpose = pgettext("building", "Digs iron ore out of the ground in mountain terrain."),
            -- TRANSLATORS: Performance helptext for a barbarian production site: Deep Iron Mine
            performance = pgettext("barbarians_building", "If the food supply is steady, this mine can produce iron ore in 39.5 seconds on average.")
         }
      },
      {
         name = "barbarians_ironmine_deeper",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Deeper Iron Mine
            lore = pgettext("barbarians_building", "‘I look at my own pick wearing away day by day and I realize why my work is important.’"),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Deeper Iron Mine
            lore_author = pgettext("barbarians_building", "Quote from an anonymous miner."),
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Deeper Iron Mine
            purpose = pgettext("building", "Digs iron ore out of the ground in mountain terrain."),
            -- TRANSLATORS: Performance helptext for a barbarian production site: Deeper Iron Mine
            performance = pgettext("barbarians_building", "If the food supply is steady, this mine can produce iron ore in 17.6 seconds on average.")
         }
      },
      {
         name = "barbarians_goldmine",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Gold Mine
            lore = pgettext("barbarians_building", "‘Soft and supple.<br> And yet untouched by time and weather.<br> Rays of sun, wrought into eternity…’"),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Gold Mine
            lore_author = pgettext("barbarians_building", "Excerpt from ‘Our Treasures Underground’,<br> a traditional Barbarian song."),
            -- TRANSLATORS: Purpose helptext for production site: Gold Mine
            purpose = pgettext("building", "Digs gold ore out of the ground in mountain terrain."),
            -- TRANSLATORS: Performance helptext for production site: Gold Mine
            performance = pgettext("barbarians_building", "If the food supply is steady, this mine can produce gold ore in %s on average."):bformat(ngettext("%d second", "%d seconds", 65):bformat(65))
         }
      },
      {
         name = "barbarians_goldmine_deep",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Deep Gold Mine
            lore = pgettext("barbarians_building", "‘Soft and supple.<br> And yet untouched by time and weather.<br> Rays of sun, wrought into eternity…’"),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Deep Gold Mine
            lore_author = pgettext("barbarians_building", "Excerpt from ‘Our Treasures Underground’,<br> a traditional Barbarian song."),
            -- TRANSLATORS: Purpose helptext for production site: Deep Gold Mine
            purpose = pgettext("building", "Digs gold ore out of the ground in mountain terrain."),
            -- TRANSLATORS: Performance helptext for production site: Deep Gold Mine
            performance = pgettext("barbarians_building", "If the food supply is steady, this mine can produce gold ore in 19.5 seconds on average.")
         }
      },
      {
         name = "barbarians_goldmine_deeper",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Deeper Gold Mine
            lore = pgettext("barbarians_building", "‘Soft and supple.<br> And yet untouched by time and weather.<br> Rays of sun, wrought into eternity…’"),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Deeper Gold Mine
            lore_author = pgettext("barbarians_building", "Excerpt from ‘Our Treasures Underground’,<br> a traditional Barbarian song."),
            -- TRANSLATORS: Purpose helptext for production site: Deeper Gold Mine
            purpose = pgettext("building", "Digs gold ore out of the ground in mountain terrain."),
            -- TRANSLATORS: Performance helptext for production site: Deeper Gold Mine
            performance = pgettext("barbarians_building", "If the food supply is steady, this mine can produce gold ore in 18.5 seconds on average.")
         }
      },

      -- Training Sites
      {
         name = "barbarians_battlearena",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian training site: Battle Arena
            lore = pgettext("barbarians_building", "‘No better friend you have in battle than the enemy’s blow that misses.’"),
            -- TRANSLATORS: Lore author helptext for a barbarian training site: Battle Arena
            lore_author = pgettext("barbarians_building", "Said to originate from Neidhardt, the famous trainer."),
            purpose = {
               -- TRANSLATORS: Purpose helptext for a barbarian training site: Battle Arena, part 1
               pgettext("barbarians_building", "Trains soldiers in ‘Evade’."),
               -- TRANSLATORS: Purpose helptext for a barbarian training site: Battle Arena, part 2
               pgettext("barbarians_building", "‘Evade’ increases the soldier’s chance not to be hit by the enemy and so to remain totally unaffected.")
            },
            -- TRANSLATORS: Note helptext for a barbarian training site: Battle Arena
            note = pgettext("barbarians_building", "Barbarian soldiers cannot be trained in ‘Defense’ and will remain at their initial level."),
            -- TRANSLATORS: Performance helptext for a barbarian training site: Battle Arena
            performance = pgettext("barbarians_building", "If all needed wares are delivered in time, a battle arena can train evade for one soldier from 0 to the highest level in %1$s and %2$s on average."):bformat(ngettext("%d minute", "%d minutes", 1):bformat(1), ngettext("%d second", "%d seconds", 10):bformat(10))
         }
      },
      {
         name = "barbarians_trainingcamp",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian training site: Training Camp
            lore = pgettext("barbarians_building", "‘He who is strong shall neither forgive nor forget, but revenge injustice suffered – in the past and for all future.’"),
            -- TRANSLATORS: Lore author helptext for a barbarian training site: Training Camp
            lore_author = pgettext("barbarians_building", "Chief Chat’Karuth in a speech to his army."),
            purpose = {
               -- TRANSLATORS: Purpose helptext for a barbarian training site: Training Camp, part 1
               pgettext("barbarians_building", "Trains soldiers in ‘Attack’ and in ‘Health’."),
               -- TRANSLATORS: Purpose helptext for a barbarian training site: Training Camp, part 2
               pgettext("barbarians_building", "Equips the soldiers with all necessary weapons and armor parts.")
            },
            -- TRANSLATORS: Note helptext for a barbarian building: Training Camp
            note = pgettext("barbarians_building", "Barbarian soldiers cannot be trained in ‘Defense’ and will remain at their initial level."),
            -- TRANSLATORS: Performance helptext for a barbarian training site: Training Camp
            performance = pgettext("barbarians_building", "If all needed wares are delivered in time, a training camp can train one new soldier in attack and health to the final level in %1$s and %2$s on average."):bformat(ngettext("%d minute", "%d minutes", 4):bformat(4), ngettext("%d second", "%d seconds", 40):bformat(40))
         }
      },

      -- Military Sites
      {
         name = "barbarians_sentry",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian military site: Sentry
            lore = pgettext("barbarians_building", "‘The log cabin was so small that two men could hardly live there. But we were young and carefree. We just relished our freedom and the responsibility as an outpost.’"),
            -- TRANSLATORS: Lore author helptext for a barbarian military site: Sentry
            lore_author = pgettext("barbarians_building", "Boldreth,<br>about his time as young soldier"),
            -- TRANSLATORS: Purpose helptext for a barbarian military site: Sentry
            purpose = pgettext("barbarians_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for a barbarian military site: Sentry
            note = pgettext("barbarians_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "barbarians_barrier",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian military site: Barrier
            lore = pgettext("barbarians_building", "‘When we looked down to the valley from our newly established barrier, we felt that the spirit of our fathers was with us.’"),
            -- TRANSLATORS: Lore author helptext for a barbarian military site: Barrier
            lore_author = pgettext("barbarians_building", "Ballad ‘The Battle of Kal’mavrath’ by Hakhor the Bard"),
            -- TRANSLATORS: Purpose helptext for a barbarian military site: Barrier
            purpose = pgettext("barbarians_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for a barbarian military site: Barrier
            note = pgettext("barbarians_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },

      {
         name = "barbarians_tower",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian military site: Tower
            lore = pgettext("barbarians_building", "‘From the height of our tower we could see far into enemy territory. The enemy was well prepared, but we also noticed some weak points in their defense.’"),
            -- TRANSLATORS: Lore author helptext for a barbarian military site: Tower
            lore_author = pgettext("barbarians_building", "Ballad ‘The Battle of Kal’mavrath’ by Hakhor the Bard"),
            -- TRANSLATORS: Purpose helptext for a barbarian military site: Tower
            purpose = pgettext("barbarians_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for a barbarian military site: Tower
            note = pgettext("barbarians_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "barbarians_fortress",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian military site: Fortress
            lore = pgettext("barbarians_building", "‘This stronghold made from blackwood and stones will be a hard nut to crack for them.’"),
            -- TRANSLATORS: Lore author helptext for a barbarian military site: Fortress
            lore_author = pgettext("barbarians_building", "Berthron,<br>chief military adviser of Chat’Karuth"),
            -- TRANSLATORS: Purpose helptext for a barbarian military site: Fortress
            purpose = pgettext("barbarians_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for a barbarian military site: Fortress
            note = pgettext("barbarians_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      {
         name = "barbarians_citadel",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian military site: Citadel
            lore = pgettext("barbarians_building", "‘The Citadel of Adlen surely is the finest masterpiece of Barbarian craftsmanship. Nothing as strong and big and beautiful has ever been built in such a short time.’"),
            -- TRANSLATORS: Lore author helptext for a barbarian military site: Citadel
            lore_author = pgettext("barbarians_building", "Colintan, chief planner of the Citadel of Adlen,<br>at its opening ceremony"),
            -- TRANSLATORS: Purpose helptext for a barbarian military site: Citadel
            purpose = pgettext("barbarians_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for a barbarian military site: Citadel
            note = pgettext("barbarians_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },

      -- Seafaring/Ferry Sites - these are only displayed on seafaring/ferry maps
      {
         name = "barbarians_ferry_yard",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for production site: Ferry Yard
            purpose = pgettext("building", "Builds ferries."),
            -- TRANSLATORS: Note helptext for production site: Ferry Yard
            note = pgettext("building", "Needs water nearby.")
         }
      },
      {
         name = "barbarians_shipyard",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Shipyard
            lore = pgettext("barbarians_building", [[‘When I saw the Saxnot for the first time, her majestic dragon head already looked up to the skies and the master was about to install the square sail.<br>]] ..
                                          [[It was the most noble ship I ever saw.’]]),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Shipyard
            lore_author = pgettext("barbarians_building", "Captain Thanlas the Elder,<br>Explorer"),
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Shipyard
            purpose = pgettext("building", "Constructs ships that are used for overseas colonization and for trading between ports.")
         }
      },
      {
         name = "barbarians_weaving_mill",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Weaving Mill
            lore = pgettext("barbarians_building", [[‘Fine cloth and jewelry mean nothing to a true Barbarian.<br>]] ..
                                          [[But a noble sail in the wind will please his eyes.’]]),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Weaving Mill
            lore_author = pgettext("barbarians_building", "Khantarakh, ‘The Modern Barbarian Economy’,<br>9ᵗʰ cowhide ‘Seafaring and Conquest’"),
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Weaving Mill
            purpose = pgettext("barbarians_building", "Weaves cloth out of reed.")
         }
      },

      -- Partially Finished Buildings - these are the same 2 buildings for all tribes
      {
         name = "constructionsite",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian building: Construction Site
            lore = pgettext("building", "‘Don’t swear at the builder who is short of building materials.’"),
            -- TRANSLATORS: Lore author helptext for a barbarian building: Construction Site
            lore_author = pgettext("building", "Proverb widely used for impossible tasks of any kind"),
            -- TRANSLATORS: Purpose helptext for a barbarian building: Construction Site
            purpose = pgettext("building", "A new building is being built at this construction site.")
         }
      },
      {
         name = "dismantlesite",
         helptexts = {
            -- TRANSLATORS: Lore helptext for a barbarian building: Dismantle Site
            lore = pgettext("building", "‘New paths will appear when you are willing to tear down the old.’"),
            -- TRANSLATORS: Lore author helptext for a barbarian building: Dismantle Site
            lore_author = pgettext("building", "Proverb"),
            -- TRANSLATORS: Purpose helptext for a barbarian building: Dismantle Site
            purpose = pgettext("building", "A building is being dismantled at this dismantle site, returning some of the resources that were used during this building’s construction to your tribe’s stores.")
         }
      },
   },

   -- Productionsite status strings

   -- TRANSLATORS: Productivity label on a barbarian building if there is 1 worker missing
   productionsite_worker_missing = pgettext("barbarians", "Worker missing"),
   -- TRANSLATORS: Productivity label on a barbarian building if there is 1 worker coming
   productionsite_worker_coming = pgettext("barbarians", "Worker is coming"),
   -- TRANSLATORS: Productivity label on a barbarian building if there is more than 1 worker missing. If you need plural forms here, please let us know.
   productionsite_workers_missing = pgettext("barbarians", "Workers missing"),
   -- TRANSLATORS: Productivity label on a barbarian building if there is more than 1 worker coming. If you need plural forms here, please let us know.
   productionsite_workers_coming = pgettext("barbarians", "Workers are coming"),
   -- TRANSLATORS: Productivity label on a barbarian building if there is 1 experienced worker missing
   productionsite_experienced_worker_missing = pgettext("barbarians", "Expert missing"),
   -- TRANSLATORS: Productivity label on a barbarian building if there is more than 1 experienced worker missing. If you need plural forms here, please let us know.
   productionsite_experienced_workers_missing = pgettext("barbarians", "Experts missing"),

   -- Soldier strings to be used in Military Status strings

   soldier_context = "barbarians_soldier",
   soldier_0_sg = "%1% soldier (+%2%)",
   soldier_0_pl = "%1% soldiers (+%2%)",
   soldier_1_sg = "%1% soldier",
   soldier_1_pl = "%1% soldiers",
   soldier_2_sg = "%1%(+%2%) soldier (+%3%)",
   soldier_2_pl = "%1%(+%2%) soldiers (+%3%)",
   soldier_3_sg = "%1%(+%2%) soldier",
   soldier_3_pl = "%1%(+%2%) soldiers",
   -- TRANSLATORS: %1% is the number of Barbarian soldiers the plural refers to. %2% is the maximum number of soldier slots in the building.
   UNUSED_soldier_0 = npgettext("barbarians_soldier", "%1% soldier (+%2%)", "%1% soldiers (+%2%)", 0),
   -- TRANSLATORS: Number of Barbarian soldiers stationed at a militarysite.
   UNUSED_soldier_1 = npgettext("barbarians_soldier", "%1% soldier", "%1% soldiers", 0),
   -- TRANSLATORS: %1% is the number of Barbarian soldiers the plural refers to. %2% are currently open soldier slots in the building. %3% is the maximum number of soldier slots in the building
   UNUSED_soldier_2 = npgettext("barbarians_soldier", "%1%(+%2%) soldier (+%3%)", "%1%(+%2%) soldiers (+%3%)", 0),
   -- TRANSLATORS: %1% is the number of Barbarian soldiers the plural refers to. %2% are currently open soldier slots in the building.
   UNUSED_soldier_3 = npgettext("barbarians_soldier", "%1%(+%2%) soldier", "%1%(+%2%) soldiers", 0),

   -- Special types
   builder = "barbarians_builder",
   carriers = {"barbarians_carrier", "barbarians_ox"},
   geologist = "barbarians_geologist",
   scouts_house = "barbarians_scouts_hut",
   soldier = "barbarians_soldier",
   ship = "barbarians_ship",
   ferry = "barbarians_ferry",
   port = "barbarians_port",

   fastplace = {
      warehouse = "barbarians_warehouse",
      port = "barbarians_port",
      training_small = "barbarians_battlearena",
      training_large = "barbarians_trainingcamp",
      military_small_primary = "barbarians_sentry",
      military_medium_primary = "barbarians_barrier",
      military_tower = "barbarians_tower",
      military_fortress = "barbarians_fortress",
      woodcutter = "barbarians_lumberjacks_hut",
      forester = "barbarians_rangers_hut",
      quarry = "barbarians_quarry",
      building_materials_primary = "barbarians_wood_hardener",
      building_materials_secondary = "barbarians_lime_kiln",
      building_materials_tertiary = "barbarians_reed_yard",
      fisher = "barbarians_fishers_hut",
      hunter = "barbarians_hunters_hut",
      fish_meat_replenisher = "barbarians_gamekeepers_hut",
      well = "barbarians_well",
      farm_primary = "barbarians_farm",
      bakery = "barbarians_bakery",
      brewery = "barbarians_micro_brewery",
      tavern = "barbarians_tavern",
      smelting = "barbarians_smelting_works",
      tool_smithy = "barbarians_metal_workshop",
      armor_smithy = "barbarians_helmsmithy",
      weaving_mill = "barbarians_weaving_mill",
      shipyard = "barbarians_shipyard",
      ferry_yard = "barbarians_ferry_yard",
      scout = "barbarians_scouts_hut",
      barracks = "barbarians_barracks",
      second_carrier = "barbarians_cattlefarm",
      charcoal = "barbarians_charcoal_kiln",
      mine_stone = "barbarians_granitemine",
      mine_coal = "barbarians_coalmine",
      mine_iron = "barbarians_ironmine",
      mine_gold = "barbarians_goldmine",
   },
}

pop_textdomain()
