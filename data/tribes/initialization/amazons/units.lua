descriptions = wl.Descriptions() -- TODO(matthiakl): only for savegame compatibility with 1.0, do not use.

image_dirname = path.dirname(__file__) .. "images/"

push_textdomain("tribes_encyclopedia")

-- For formatting time strings
include "tribes/scripting/help/time_strings.lua"

wl.Descriptions():new_tribe {
   name = "amazons",
   animation_directory = image_dirname,
   animations = {
      frontier = { hotspot = {8, 36} },
      pinned_note = { hotspot = {18, 67} },
      bridge_normal_e =  { hotspot = {-1, 15}},
      bridge_busy_e =    { hotspot = {-1, 15}},
      bridge_normal_se = { hotspot = {5, 4}},
      bridge_busy_se =   { hotspot = {5, 4}},
      bridge_normal_sw = { hotspot = {40, 1}},
      bridge_busy_sw =   { hotspot = {40, 1}}
   },
   spritesheets = {
      flag = {
         hotspot = {26, 39},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 5
      }
   },

   bridge_height = 6,

   collectors_points_table = {
      { ware = "gold", points = 12},
      { ware = "spear_stone_tipped", points = 1},
      { ware = "spear_hardened", points = 1},
      { ware = "armor_wooden", points = 1},
      { ware = "helmet_wooden", points = 1},
      { ware = "warriors_coat", points = 16},
      { ware = "tunic", points = 1},
      { ware = "vest_padded", points = 2},
      { ware = "protector_padded", points = 17},
      { ware = "boots_sturdy", points = 1},
      { ware = "boots_swift", points = 1},
      { ware = "boots_hero", points = 16},
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
         [0] = "amazons_resi_none",
      },
      resource_gold = {
         [10] = "amazons_resi_gold_1",
         [20] = "amazons_resi_gold_2",
      },
      resource_stones = {
         [10] = "amazons_resi_stones_1",
         [20] = "amazons_resi_stones_2",
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
            preciousness = 24,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Log, part 1
                  pgettext("ware", "Logs are an important basic building material. They are produced by felling trees."),
                  -- TRANSLATORS: Helptext for an Amazon ware: Log, part 2
                  pgettext("amazons_ware", "Amazon woodcutters fell trees and jungle preservers will plant them. The resulting logs are used by the stone carver. The charcoal kiln also needs logs to produce charcoal.")
               }
            }
         },
         {
            name = "granite",
            default_target_quantity = 20,
            preciousness = 7,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Granite, part 1
                  pgettext("ware", "Granite is a basic building material."),
                  -- TRANSLATORS: Helptext for an Amazon ware: Granite, part 2
                  pgettext("amazons_ware", "The Amazons use granite for making tools and weapons as well. It is produced by the stonecutters and in stone mines."),
               }
            }
         },
         {
            name = "balsa",
            default_target_quantity = 20,
            preciousness = 20,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Balsa
                  pgettext("amazons_ware", "Balsa wood is an important building material. It is also used to make light armor. Balsa trees are cut by an experienced woodcutter.")
               }
            }
         },
         {
            name = "ironwood",
            default_target_quantity = 40,
            preciousness = 50,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Ironwood
                  pgettext("amazons_ware", "This very hard wood is as hard as iron and it is used for several buildings, tools and weapons. It is cut by a very experienced woodcutter.")
               }
            }
         },
         {
            name = "rubber",
            default_target_quantity = 30,
            preciousness = 40,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Rubber
                  pgettext("amazons_ware", "Rubber is important for making warriors’ armor and shoes. Rubber trees are collected by experienced woodcutters.")
               }
            }
         },
         {
            name = "liana",
            preciousness = 8,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Liana
                  pgettext("amazons_ware", "Lianas grow on trees. Very durable ropes are made out of them.")
               }
            }
         },
         {
            name = "rope",
            preciousness = 2,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Rope
                  pgettext("amazons_ware", "This rope is made out of lianas. It is used for armor dresses and to construct buildings and ships.")
               }
            }
         },
      },
      {
         -- Food
         {
            name = "water",
            preciousness = 15,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Water, part 1
                  pgettext("ware", "Water is the essence of life!"),
                  -- TRANSLATORS: Helptext for an Amazon ware: Water, part 2
                  pgettext("amazons_ware", "Water is used by the cassava root cooker, the chocolate brewery, the gold digger and the tapir farm.")
               }
            }
         },
         {
            name = "cassavaroot",
            preciousness = 10,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Cassava Root
                  pgettext("amazons_ware", "Cassava roots are a special kind of root produced at cassava plantations. The Amazons like their strong taste for making bread and they also feed their tapirs with them.")
               }
            }
         },
         {
            name = "cocoa_beans",
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Cocoa Beans
                  pgettext("amazons_ware", "Cocoa beans are gathered from cocoa trees by a cocoa farmer. They are used for producing chocolate.")
               }
            }
         },
         {
            name = "amazons_bread",
            default_target_quantity = 20,
            preciousness = 5,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Bread
                  pgettext("amazons_ware", "This tasty bread is made in cassava root cookeries out of cassava root and water. It is the basic ingredient of a ration that is made by a food preserver. Soldiers like it too.")
               }
            }
         },
         {
            name = "chocolate",
            default_target_quantity = 15,
            preciousness = 5,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Chocolate
                  pgettext("amazons_ware", "Chocolate is produced by chocolate breweries and used to train soldiers.")
               }
            }
         },
         {
            name = "fish",
            preciousness = 4,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Fish
                  pgettext("amazons_ware", "Fish is one of the two major food resources of the Amazons. It is used by the food preserver to prepare rations for the workers digging for gold dust and granite. It is also consumed at the training sites (training glade and warriors’ gathering) and for recruiting new soldiers.")
               }
            }
         },
         {
            name = "meat",
            preciousness = 2,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Meat, part 1
                  pgettext("ware", "Meat contains a lot of energy, and it is obtained from wild game taken by hunter-gatherers."),
                  -- TRANSLATORS: Helptext for an Amazon ware: Meat, part 2
                  pgettext("amazons_ware", "Meat is used by the food preserver to prepare rations for the workers digging for gold dust and granite. It is also consumed at the training sites (training glade and warriors’ gathering) and for recruiting new soldiers.")
               }
            }
         },
         {
            name = "ration",
            default_target_quantity = 20,
            preciousness = 5,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Ration, part 1
                  pgettext("amazons_ware", "A small bite to keep the workers digging for gold dust and granite strong and working. The scout also consumes rations on her scouting trips."),
                  -- TRANSLATORS: Helptext for an Amazon ware: Ration, part 2
                  pgettext("amazons_ware", "Rations are produced by the food preserver out of fish or meat and bread. They are dried using a charcoal fire.")
               }
            }
         },
      },
      {
         -- Mining
         {
            name = "gold_dust",
            default_target_quantity = 15,
            preciousness = 2,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Gold Dust, part 1
                  pgettext("amazons_ware", "Gold dust is washed in a gold digger dwelling."),
                  -- TRANSLATORS: Helptext for an Amazon ware: Gold Dust, part 2
                  pgettext("amazons_ware", "Smelted in a furnace, it turns into gold which is used as a precious building material and to produce armor.")
               }
            }
         },
         {
            name = "gold",
            default_target_quantity = 20,
            preciousness = 2,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Gold, part 1
                  pgettext("amazons_ware", "Gold is the most valuable of all metals."),
                  -- TRANSLATORS: Helptext for an Amazon ware: Gold, part 2
                  pgettext("amazons_ware", "It is smelted out of gold dust by the furnace and used by the dressmaker and for special buildings.")
               }
            }
         },
         {
            name = "coal",
            default_target_quantity = 20,
            preciousness = 10,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Charcoal, part 1
                  pgettext("amazons_ware", "Charcoal is produced out of logs by a charcoal burner. The Amazons use charcoal for drying their food and for smelting gold.")
               }
            }
         },
      },
      {
         -- Tools
         {
            name = "pick",
            default_target_quantity = 3,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Pick
                  pgettext("amazons_ware", "Picks are used by stonecutters to cut granite from rocks or in mines.")
               }
            }
         },
         {
            name = "felling_ax",
            default_target_quantity = 5,
            preciousness = 3,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Felling Ax, part 1
                  pgettext("ware", "The felling ax is the tool to chop down trees."),
                  -- TRANSLATORS: Helptext for an Amazon ware: Felling Ax, part 2
                  pgettext("amazons_ware", "It is used by a woodcutter and produced by the stone carver.")
               }
            }
         },
         {
            name = "shovel",
            default_target_quantity = 2,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Shovel, part 1
                  pgettext("ware", "Shovels are needed for the proper handling of plants."),
                  -- TRANSLATORS: Helptext for an Amazon ware: Shovel, part 2
                  pgettext("amazons_ware", "Therefore the jungle preservers, the cassava farmers and the cocoa farmers use them. They are produced by the stone carver.")
               }
            }
         },
         {
            name = "hammer",
            default_target_quantity = 2,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Hammer, part 1
                  pgettext("ware", "The hammer is an essential tool."),
                  -- TRANSLATORS: Helptext for an Amazon ware: Hammer, part 2
                  pgettext("amazons_ware", "Geologists, builders, shipwrights and stone carvers all need a hammer. Make sure you’ve always got some in reserve! They are produced by the stone carver.")
               }
            }
         },
         {
            name = "machete",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Machete
                  pgettext("amazons_ware", "The machete is the tool of the liana cutters."),
               }
            }
         },
         {
            name = "spear_wooden",
            default_target_quantity = 30,
            preciousness = 3,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Wooden Spear, part 1
                  pgettext("amazons_ware", "This spear is the basic weapon in the Amazon military system. It is produced by the stone carver. In combination with a tunic, it is the equipment to fit out young soldiers."),
                  -- TRANSLATORS: Helptext for an Amazon ware: Wooden Spear, part 2
                  pgettext("amazons_ware", "It is used by the Amazon hunter-gatherers as well.")
               }
            }
         },
         {
            name = "chisel",
            default_target_quantity = 2,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Chisel, part 1
                  pgettext("ware", "The chisel is an essential tool."),
                  -- TRANSLATORS: Helptext for an Amazon ware: Chisel, part 2
                  pgettext("amazons_ware", "Stone carvers need a chisel. Make sure you’ve always got some in reserve! They are produced by the stone carvers themselves.")
               }
            }
         },
         {
            name = "kitchen_tools",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Kitchen Tools
                  pgettext("amazons_ware", "Kitchen tools are needed for preparing rations, baking cassava bread and brewing chocolate. Be sure to have a stone carver to produce this basic tool.")
               }
            }
         },
         {
            name = "needles",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Needles
                  pgettext("amazons_ware", "Needles are used by dressmakers to make dresses, boots and armor for the soldiers.")
               }
            }
         },
         {
            name = "stone_bowl",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Stone Bowl
                  pgettext("amazons_ware", "Stone bowls are used to wash gold and to smelt gold dust.")
               }
            }
         },
      },
      {
         -- Military
         {
            name = "spear_stone_tipped",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Stone Tipped Spear
                  pgettext("amazons_ware", "This spear has a stone spike. It is produced in a stone workshop and used in a warriors’ gathering – together with food – to train soldiers from attack level 0 to attack level 1.")
               }
            }
         },
         {
            name = "spear_hardened",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Hardened Spear
                  pgettext("amazons_ware", "This is a hardened spear with a stone tip. It is produced in a stone workshop and used in a warriors’ gathering – together with food – to train soldiers from attack level 1 to level 2.")
               }
            }
         },
         {
            name = "armor_wooden",
            default_target_quantity = 1,
            preciousness = 0,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Wooden Armor
                  pgettext("amazons_ware", "This light wooden armor is a basic piece of armor for the Amazons’ soldiers. It is produced by a dressmaker and used in a training glade – together with food – to train soldiers from health level 0 to level 1.")
               }
            }
         },
         {
            name = "helmet_wooden",
            default_target_quantity = 2,
            preciousness = 3,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Wodden Helmet
                  pgettext ("amazons_ware", "A wooden helmet is a basic piece of equipment for protecting soldiers. It is produced by the dressmaker and used to train soldiers from health level 1 to level 2."),
               }
            }
         },
         {
            name = "warriors_coat",
            default_target_quantity = 1,
            preciousness = 3,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Warrior's Coat
                  pgettext("amazons_ware", "Ordinary tunics can be decorated and reinforced with gold and wood chunks. Such tunics are the best armor.")
               }
            }
         },
         {
            name = "tunic",
            default_target_quantity = 30,
            preciousness = 3,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Tunic
                  pgettext("amazons_ware", "Rubber can be bound into a tunic, which is used as basic armor. All new soldiers are clothed in a tunic.")
               }
            }
         },
         {
            name = "vest_padded",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Padded Vest
                  pgettext("amazons_ware", "Padded vests are light but durable armor. They are produced by the dressmaker and used to train soldiers’ defence from level 0 to level 1.")
               }
            }
         },
         {
            name = "protector_padded",
            default_target_quantity = 1,
            preciousness = 1,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Padded Protector
                  pgettext("amazons_ware", "Padded protectors are strong and durable armor. They are produced by the dressmaker and used to train soldiers’ defence from level 1 to level 2.")
               }
            }
         },
         {
            name = "boots_sturdy",
            default_target_quantity = 1,
            preciousness = 5,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Sturdy Boots
                  pgettext("amazons_ware", "Sturdy boots are excellent footwear for soldiers to be quicker on their feet. They are produced by the dressmaker and used to train soldiers’ evade from level 0 to level 1.")
               }
            }
         },
         {
            name = "boots_swift",
            default_target_quantity = 1,
            preciousness = 5,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Swift Boots
                  pgettext("amazons_ware", "Swift boots are light and durable footwear for soldiers to be quicker on their feet. They are produced by the dressmaker and used to train soldiers’ evade from level 1 to level 2.")
               }
            }
         },
                  {
            name = "boots_hero",
            default_target_quantity = 1,
            preciousness = 5,
            helptexts = {
               purpose = {
                  -- TRANSLATORS: Helptext for an Amazon ware: Hero Boots
                  pgettext("amazons_ware", "Hero boots! The only suitable footwear for a real hero. They are produced by the dressmaker and used to train soldiers’ evade from level 2 to level 3.")
               }
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
            name = "amazons_carrier",
            helptexts = {
               -- TRANSLATORS: Helptext for an Amazon worker: Carrier
               purpose = pgettext("amazons_worker", "Carries items along your roads.")
            }
         },
         {
            name = "amazons_ferry",
            helptexts = {
               -- TRANSLATORS: Helptext for an Amazon worker: Ferry
               purpose = pgettext("amazons_worker", "Ships wares across narrow rivers.")
            }
         },
         {
            name = "amazons_tapir",
            default_target_quantity = 10,
            preciousness = 2,
            helptexts = {
               -- TRANSLATORS: Helptext for an Amazon worker: Tapir
               purpose = pgettext("amazons_worker", "Tapirs help to carry items along busy roads. They are reared in a tapir farm.")
            }
         },
         {
            name = "amazons_tapir_breeder",
            helptexts = {
               -- TRANSLATORS: Helptext for an Amazon worker: Tapir Breeder
               purpose = pgettext("amazons_worker", "Breeds tapirs for adding them to the transportation system.")
            }
         }
      },
      {
         -- Building Materials
         {
            name = "amazons_stonecutter",
            helptexts = {
               -- TRANSLATORS: Helptext for an Amazon worker: Stonecutter
               purpose = pgettext("amazons_worker", "Cuts blocks of granite out of rocks in the vicinity.")
            }
         },
         {
            name = "amazons_woodcutter",
            helptexts = {
               -- TRANSLATORS: Helptext for an Amazon worker: Woodcutter
               purpose = pgettext("amazons_worker", "Fells trees.")
            }
         },
         {
            name = "amazons_woodcutter_master",
            helptexts = {
               -- TRANSLATORS: Helptext for an Amazon worker: Master Woodcutter
               purpose = pgettext("amazons_worker", "Harvests special trees: ironwood, rubber and balsa.")
            }
         },
         {
            name = "amazons_jungle_preserver",
            helptexts = {
               -- TRANSLATORS: Helptext for an Amazon worker: Jungle Preserver
               purpose = pgettext("amazons_worker", "Plants trees.")
            }
         },
         {
            name = "amazons_jungle_master",
            helptexts = {
               -- TRANSLATORS: Helptext for an Amazon worker: Jungle Master
               purpose = pgettext("amazons_worker", "Plants special trees: ironwood, rubber and balsa.")
            }
         },
         {
            name = "amazons_liana_cutter",
            helptexts = {
               -- TRANSLATORS: Helptext for an Amazon worker: Liana Cutter
               purpose = pgettext("amazons_worker", "Cuts lianas from trees.")
            }
         },
         {
            name = "amazons_builder",
            helptexts = {
               -- TRANSLATORS: Helptext for an Amazon worker: Builder
               purpose = pgettext("amazons_worker", "Works at construction sites to raise new buildings.")
            }
         },
         {
            name = "amazons_shipwright",
            helptexts = {
               -- TRANSLATORS: Helptext for an Amazon worker: Shipwright
               purpose = pgettext("amazons_worker", "Works at the shipyard and constructs new ships.")
            }
         },
      },
      {
         -- Food
         {
            name = "amazons_hunter_gatherer",
            helptexts = {
               -- TRANSLATORS: Helptext for an Amazon worker: Hunter-Gatherer
               purpose = pgettext("amazons_worker", "The hunter-gatherer gets fresh fish and raw meat for the tribe.")
            }
         },
         {
            name = "amazons_cassava_farmer",
            helptexts = {
               -- TRANSLATORS: Helptext for an Amazon worker: Cassava Farmer
               purpose = pgettext("amazons_worker", "Grows cassava.")
            }
         },
         {
            name = "amazons_cocoa_farmer",
            helptexts = {
               -- TRANSLATORS: Helptext for an Amazon worker: Cocoa Farmer
               purpose = pgettext("amazons_worker", "Grows cocoa.")
            }
         },
         {
            name = "amazons_cook",
            helptexts = {
               -- TRANSLATORS: Helptext for an Amazon worker: Cook
               purpose = pgettext("amazons_worker", "Prepares rations, bakes cassava bread and brews chocolate.")
            }
         },
         {
            name = "amazons_wilderness_keeper",
            helptexts = {
               -- TRANSLATORS: Helptext for an Amazon worker: Wilderness Keeper
               purpose = pgettext("amazons_worker", "Breeds game or fish.")
            }
         },
      },
      {
         -- Mining
         {
            name = "amazons_geologist",
            helptexts = {
               -- TRANSLATORS: Helptext for an Amazon worker: Geologist
               purpose = pgettext("amazons_worker", "Discovers resources for mining.")
            }
         },
         {
            name = "amazons_gold_digger",
            helptexts = {
               -- TRANSLATORS: Helptext for an Amazon worker: Gold Digger
               purpose = pgettext("amazons_worker", "Works hard in the gold dwellings in mountains to obtain gold by washing it from gravel.")
            }
         },
         {
            name = "amazons_charcoal_burner",
            helptexts = {
               -- TRANSLATORS: Helptext for an Amazon worker: Charcoal Burner
               purpose = pgettext("amazons_worker", "Burns logs to produce charcoal.")
            }
         },
         {
            name = "amazons_gold_smelter",
            helptexts = {
               -- TRANSLATORS: Helptext for an Amazon worker: Gold Smelter
               purpose = pgettext("amazons_worker", "Smelts gold at furnaces.")
            }
         },
      },
      {
         -- Tools
         {
            name = "amazons_stone_carver",
            helptexts = {
               -- TRANSLATORS: Helptext for an Amazon worker: Stone Carver
               purpose = pgettext("amazons_worker", "Produces spears for soldiers and tools for workers.")
            }
         }
      },
      {
         -- Military
         {
            name = "amazons_soldier",
            default_target_quantity = 10,
            preciousness = 5,
            helptexts = {
               -- TRANSLATORS: Helptext for an Amazon worker: Soldier
               purpose = pgettext("amazons_worker", "Defend and Conquer!")
            }
         },
         {
            name = "amazons_trainer",
            helptexts = {
               -- TRANSLATORS: Helptext for an Amazon worker: Trainer
               purpose = pgettext("amazons_worker", "Trains the soldiers.")
            }
         },
         {
            name = "amazons_dressmaker",
            helptexts = {
               -- TRANSLATORS: Helptext for an Amazon worker: Dressmaker
               purpose = pgettext("amazons_worker", "Produces all kinds of dresses, boots and armor.")
            }
         },
         {
            name = "amazons_scout",
            helptexts = {
               -- TRANSLATORS: Helptext for an Amazon worker: Scout
               purpose = pgettext("amazons_worker", "Explores unknown territory.")
            }
         }
      }
   },

   immovables = {
      {
         name = "ashes",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Ashes
            purpose = _("The remains of a destroyed building.")
         }
      },
      {
         name = "destroyed_building",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Destroyed Building
            purpose = _("The remains of a destroyed building.")
         }
      },
      {
         name = "deadtree7",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Ashes
            purpose = _("The remains of an old tree.")
         }
      },
      {
         name = "balsa_amazons_sapling",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Balsa Tree Sapling
            purpose = _("This balsa tree has just been planted.")
         }
      },
      {
         name = "balsa_amazons_pole",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Balsa Tree Pole
            purpose = _("This balsa tree is growing.")
         }
      },
      {
         name = "balsa_amazons_mature",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Balsa Tree Mature
            purpose = _("This balsa tree is fully grown. Lianas can be gathered from it.")
         }
      },
      {
         name = "balsa_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Balsa Tree Old
            purpose = _("This balsa tree is ready for harvesting balsa wood by a master woodcutter. Other tribes can harvest it for normal logs.")
         }
      },
      {
         name = "balsa_black_amazons_sapling",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Balsa Tree Sapling
            purpose = _("This balsa tree has just been planted.")
         }
      },
      {
         name = "balsa_black_amazons_pole",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Balsa Tree Pole
            purpose = _("This balsa tree is growing.")
         }
      },
      {
         name = "balsa_black_amazons_mature",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Balsa Tree Mature
            purpose = _("This balsa tree is fully grown. Lianas can be gathered from it.")
         }
      },
      {
         name = "balsa_black_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Balsa Tree Old
            purpose = _("This balsa tree is ready for harvesting balsa wood by a master woodcutter. Other tribes can harvest it for normal logs.")
         }
      },
      {
         name = "balsa_desert_amazons_sapling",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Balsa Tree Sapling
            purpose = _("This balsa tree has just been planted.")
         }
      },
      {
         name = "balsa_desert_amazons_pole",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Balsa Tree Pole
            purpose = _("This balsa tree is growing.")
         }
      },
      {
         name = "balsa_desert_amazons_mature",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Balsa Tree Mature
            purpose = _("This balsa tree is fully grown. Lianas can be gathered from it.")
         }
      },
      {
         name = "balsa_desert_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Balsa Tree Old
            purpose = _("This balsa tree is ready for harvesting balsa wood by a master woodcutter. Other tribes can harvest it for normal logs.")
         }
      },
      {
         name = "balsa_winter_amazons_sapling",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Balsa Tree Sapling
            purpose = _("This balsa tree has just been planted.")
         }
      },
      {
         name = "balsa_winter_amazons_pole",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Balsa Tree Pole
            purpose = _("This balsa tree is growing.")
         }
      },
      {
         name = "balsa_winter_amazons_mature",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Balsa Tree Mature
            purpose = _("This balsa tree is fully grown. Lianas can be gathered from it.")
         }
      },
      {
         name = "balsa_winter_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Balsa Tree Old
            purpose = _("This balsa tree is ready for harvesting balsa wood by a master woodcutter. Other tribes can harvest it for normal logs.")
         }
      },
      {
         name = "ironwood_amazons_sapling",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Ironwood Tree Sapling
            purpose = _("This ironwood tree has just been planted.")
         }
      },
      {
         name = "ironwood_amazons_pole",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Ironwood Tree Pole
            purpose = _("This ironwood tree is growing.")
         }
      },
      {
         name = "ironwood_amazons_mature",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Ironwood Tree Mature
            purpose = _("This ironwood tree is fully grown. Lianas can be gathered from it.")
         }
      },
      {
         name = "ironwood_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Ironwood Tree Old
            purpose = _("This ironwood tree is ready for harvesting ironwood by a master woodcutter. Other tribes can harvest it for normal logs.")
         }
      },
      {
         name = "ironwood_black_amazons_sapling",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Ironwood Tree Sapling
            purpose = _("This ironwood tree has just been planted.")
         }
      },
      {
         name = "ironwood_black_amazons_pole",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Ironwood Tree Pole
            purpose = _("This ironwood tree is growing.")
         }
      },
      {
         name = "ironwood_black_amazons_mature",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Ironwood Tree Mature
            purpose = _("This ironwood tree is fully grown. Lianas can be gathered from it.")
         }
      },
      {
         name = "ironwood_black_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Ironwood Tree Old
            purpose = _("This ironwood tree is ready for harvesting ironwood by a master woodcutter. Other tribes can harvest it for normal logs.")
         }
      },
      {
         name = "ironwood_desert_amazons_sapling",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Ironwood Tree Sapling
            purpose = _("This ironwood tree has just been planted.")
         }
      },
      {
         name = "ironwood_desert_amazons_pole",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Ironwood Tree Pole
            purpose = _("This ironwood tree is growing.")
         }
      },
      {
         name = "ironwood_desert_amazons_mature",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Ironwood Tree Mature
            purpose = _("This ironwood tree is fully grown. Lianas can be gathered from it.")
         }
      },
      {
         name = "ironwood_desert_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Ironwood Tree Old
            purpose = _("This ironwood tree is ready for harvesting ironwood by a master woodcutter. Other tribes can harvest it for normal logs.")
         }
      },
      {
         name = "ironwood_winter_amazons_sapling",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Ironwood Tree Sapling
            purpose = _("This ironwood tree has just been planted.")
         }
      },
      {
         name = "ironwood_winter_amazons_pole",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Ironwood Tree Pole
            purpose = _("This ironwood tree is growing.")
         }
      },
      {
         name = "ironwood_winter_amazons_mature",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Ironwood Tree Mature
            purpose = _("This ironwood tree is fully grown. Lianas can be gathered from it.")
         }
      },
      {
         name = "ironwood_winter_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Ironwood Tree Old
            purpose = _("This ironwood tree is ready for harvesting ironwood by a master woodcutter. Other tribes can harvest it for normal logs.")
         }
      },
      {
         name = "rubber_amazons_sapling",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Rubber Tree Sapling
            purpose = _("This rubber tree has just been planted.")
         }
      },
      {
         name = "rubber_amazons_pole",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Rubber Tree Pole
            purpose = _("This rubber tree is growing.")
         }
      },
      {
         name = "rubber_amazons_mature",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Rubber Tree Mature
            purpose = _("This rubber tree is fully grown. Lianas can be gathered from it.")
         }
      },
      {
         name = "rubber_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Rubber Tree Old
            purpose = _("This rubber tree is ready for harvesting rubber by a master woodcutter. Other tribes can harvest it for normal logs.")
         }
      },
      {
         name = "rubber_black_amazons_sapling",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Rubber Tree Sapling
            purpose = _("This rubber tree has just been planted.")
         }
      },
      {
         name = "rubber_black_amazons_pole",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Rubber Tree Pole
            purpose = _("This rubber tree is growing.")
         }
      },
      {
         name = "rubber_black_amazons_mature",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Rubber Tree Mature
            purpose = _("This rubber tree is fully grown. Lianas can be gathered from it.")
         }
      },
      {
         name = "rubber_black_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Rubber Tree Old
            purpose = _("This rubber tree is ready for harvesting rubber by a master woodcutter. Other tribes can harvest it for normal logs.")
         }
      },
      {
         name = "rubber_desert_amazons_sapling",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Rubber Tree Sapling
            purpose = _("This rubber tree has just been planted.")
         }
      },
      {
         name = "rubber_desert_amazons_pole",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Rubber Tree Pole
            purpose = _("This rubber tree is growing.")
         }
      },
      {
         name = "rubber_desert_amazons_mature",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Rubber Tree Mature
            purpose = _("This rubber tree is fully grown. Lianas can be gathered from it.")
         }
      },
      {
         name = "rubber_desert_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Rubber Tree Old
            purpose = _("This rubber tree is ready for harvesting rubber by a master woodcutter. Other tribes can harvest it for normal logs.")
         }
      },
      {
         name = "rubber_winter_amazons_sapling",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Rubber Tree Sapling
            purpose = _("This rubber tree has just been planted.")
         }
      },
      {
         name = "rubber_winter_amazons_pole",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Rubber Tree Pole
            purpose = _("This rubber tree is growing.")
         }
      },
      {
         name = "rubber_winter_amazons_mature",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Rubber Tree Mature
            purpose = _("This rubber tree is fully grown. Lianas can be gathered from it.")
         }
      },
      {
         name = "rubber_winter_amazons_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Rubber Tree Old
            purpose = _("This rubber tree is ready for harvesting rubber by a master woodcutter. Other tribes can harvest it for normal logs.")
         }
      },
      {
         name = "cassavafield_tiny",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Cassava Field
            purpose = _("This field has just been planted.")
         }
      },
      {
         name = "cassavafield_small",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Cassava Field
            purpose = _("This field is growing.")
         }
      },
      {
         name = "cassavafield_medium",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Cassava Field
            purpose = _("This field is growing.")
         }
      },
      {
         name = "cassavafield_ripe",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Cassava Field
            purpose = _("This field is ready for harvesting.")
         }
      },
      {
         name = "cassavafield_harvested",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Cassava Field
            purpose = _("This field has been harvested.")
         }
      },
      {
         name = "cocoa_tree_sapling",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Cocoa Tree Sapling
            purpose = _("This cocoa tree has just been planted.")
         }
      },
      {
         name = "cocoa_tree_pole",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Cocoa Tree Pole
            purpose = _("This cocoa tree is growing.")
         }
      },
      {
         name = "cocoa_tree_mature",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Cocoa Tree Mature
            purpose = _("This cocoa tree is fully grown.")
         }
      },
      {
         name = "cocoa_tree_old",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Cocoa Tree Old
            purpose = _("The beans on this cocoa tree are ready for harvesting.")
         }
      },
      {
         name = "amazons_resi_none",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon resource indicator: No resources
            purpose = _("There are no resources in the ground here.")
         }
      },
      {
         name = "amazons_resi_gold_1",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an Amazon resource indicator: Gold, part 1
               _("Gold veins contain gold ore that can be washed from the gravel at gold digger dwellings."),
               -- TRANSLATORS: Helptext for an Amazon resource indicator: Gold, part 2
               _("There is only a little bit of gold here.")
            }
         }
      },
      {
         name = "amazons_resi_stones_1",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an Amazon resource indicator: Stones, part 1
               _("Granite is a basic building material and can be dug up by a stone mine."),
               -- TRANSLATORS: Helptext for an Amazon resource indicator: Stones, part 2
               _("There is only a little bit of granite here."),
            }
         }
      },
      {
         name = "amazons_resi_gold_2",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an Amazon resource indicator: Gold, part 1
               _("Gold veins contain gold ore that can be washed from the gravel at gold digger dwellings."),
               -- TRANSLATORS: Helptext for an Amazon resource indicator: Gold, part 2
               _("There is a lot of gold here.")
            }
         }
      },
      {
         name = "amazons_resi_stones_2",
         helptexts = {
            purpose = {
               -- TRANSLATORS: Helptext for an Amazon resource indicator: Stones, part 1
               _("Granite is a basic building material and can be dug up by a stone mine."),
               -- TRANSLATORS: Helptext for an Amazon resource indicator: Stones, part 2
               _("There is a lot of granite here.")
            }
         }
      },
      {
         name = "amazons_shipconstruction",
         helptexts = {
            -- TRANSLATORS: Helptext for an Amazon immovable: Ship Under Construction
            purpose = _("A ship is being constructed at this site.")
         }
      },
   },

   -- The order here also determines the order in lists on screen.
   buildings = {
      -- Warehouses
      {
         name = "amazons_headquarters",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon warehouse: Headquarters
            purpose = pgettext("amazons_building", "Accommodation for your people. Also stores your wares and tools."),
            -- TRANSLATORS: Note helptext for an Amazon warehouse: Headquarters
            note = pgettext("amazons_building", "The headquarters is your main building.")
         }
      },
      {
         name = "amazons_warehouse",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon warehouse: Warehouse
            purpose = pgettext("amazons_building", "Your workers and soldiers will find shelter here. Also stores your wares and tools.")
         }
      },
      {
         name = "amazons_port",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon warehouse: Port
            purpose = pgettext("amazons_building", "Serves as a base for overseas colonization and trade. Also stores your soldiers, wares and tools."),
            -- TRANSLATORS: Note helptext for an Amazon warehouse: Port
            note = pgettext("amazons_building", "Similar to the Headquarters a Port can be attacked and destroyed by an enemy. It is recommendable to send soldiers to defend it.")
         }
      },


      -- Small
      {
         name = "amazons_stonecutters_hut",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon production site: Stonecutter's Hut
            purpose = pgettext("amazons_building", "Cuts raw pieces of granite out of rocks in the vicinity."),
            -- TRANSLATORS: Note helptext for an Amazon production site: Stonecutter's Hut
            note = pgettext("amazons_building", "The stonecutter’s hut needs rocks to cut within the work area.")
         }
      },
      {
         name = "amazons_woodcutters_hut",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon production site: Woodcutter's Hut
            purpose = pgettext("amazons_building", "Fells trees in the surrounding area and processes them into logs."),
            -- TRANSLATORS: Note helptext for an Amazon production site: Woodcutter's Hut
            note = pgettext("amazons_building", "The woodcutter’s hut needs trees to fell within the work area.")
         }
      },
      {
         name = "amazons_jungle_preservers_hut",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon production site: Jungle Preserver's Hut
            purpose = pgettext("amazons_building", "Plants trees in the surrounding area."),
            -- TRANSLATORS: Note helptext for an Amazon production site: Jungle Preserver's Hut
            note = pgettext("amazons_building", "The jungle preserver’s hut needs free space within the work area to plant the trees.")
         }
      },
      {
         name = "amazons_hunter_gatherers_hut",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon production site: Hunter-Gatherer's Hut
            purpose = pgettext("amazons_building", "Hunts animals to produce meat. Catches fish in the waters."),
            note = {
               -- TRANSLATORS: Note helptext for an Amazon production site: Hunter-Gatherer's Hut
               pgettext("amazons_building", "The hunter-gatherer’s hut needs animals or fish to hunt or catch within the work area."),
               -- TRANSLATORS: Note helptext for an Amazon production site: Hunter-Gatherer's Hut
               pgettext("amazons_building", "Roads and trees along the shoreline block fishing."),
            },
            -- TRANSLATORS: Performance helptext for an Amazon production site: Hunter-Gatherer's Hut
            performance = pgettext("amazons_building", "The hunter-gatherer pauses %s before going to work again."):bformat(format_seconds(34))

         }
      },
      {
         name = "amazons_liana_cutters_hut",
         helptexts = {
            -- TRANSLATORS: Lore author helptext for an Amazon production site: Liana Cutter's Hut
            lore = pgettext("amazons_building", "‘Sunlight smiled upon us and our Ancestors while we worked to clear liana from the trees. I was not as efficient as my aunt and sisters, and so I learned to ply rope.’"),
            -- TRANSLATORS: Lore author helptext for an Amazon production site: Liana Cutter's Hut
            lore_author = pgettext("amazons_building", "Mistress WeaverSong, recollecting her early years as a liana cutter."),
            -- TRANSLATORS: Purpose helptext for an Amazon production site: Liana Cutter's Hut
            purpose = pgettext("amazons_building", "Home of the liana cutter who cuts lianas for making ropes."),
            -- TRANSLATORS: Note helptext for an Amazon production site: Liana Cutter's Hut
            note = pgettext("amazons_building", "The liana cutter harvests liana from fully matured trees within the work area which are not being used as lookouts."),
            -- TRANSLATORS: Performance helptext for an Amazon production site: Liana Cutter's Hut
            performance = pgettext("amazons_building", "The liana cutter rests %s before going to work again."):bformat(format_seconds(30))
         }
      },
      {
         name = "amazons_water_gatherers_hut",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon production site: Water Gatherer's Hut
            purpose = pgettext("amazons_building", "Draws water out of the rivers and lakes."),
            note = {
               -- TRANSLATORS: Note helptext for an Amazon production site: Water Gatherer's Hut, part 1
               pgettext("amazons_building", "The water gatherer’s hut needs open water within the work area. Your workers cannot dig up water from the ground!"),
               -- TRANSLATORS: Note helptext for an Amazon production site: Water Gatherer's Hut, part 2
               pgettext("amazons_building", "Roads and trees along the shoreline block drawing water."),
            }
         }
      },
      {
         name = "amazons_rare_tree_cutters_hut",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon production site: Rare Tree Cutter's Hut
            purpose = pgettext("amazons_building", "Harvests rare trees in the surrounding area. Delivers ironwood, balsa and rubber.")
         }
      },
      {
         name = "amazons_wilderness_keepers_tent",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon production site: Wilderness Keeper's Tent
            purpose = pgettext("amazons_building", "Breeds fish and game to renew food resources."),
            -- TRANSLATORS: Note helptext for an Amazon production site: Wilderness Keeper's Tent
            note = pgettext("amazons_building", "The wilderness keeper will only release new game at a nearby tree which is not being used as a lookout.")
         }
      },
      {
         name = "amazons_scouts_hut",
         helptexts = {
            no_scouting_building_connected = pgettext("amazons_building", "You need to connect this flag to a scout’s hut before you can send a scout here."),
            -- TRANSLATORS: Purpose helptext for an Amazon production site: Scout's Hut
            purpose = pgettext("amazons_building", "Explores unknown territory."),
            -- TRANSLATORS: Performance helptext for an Amazon production site: Scout's Hut
            performance = pgettext("amazons_building", "The scout pauses %s before going to work again."):bformat(format_seconds(30))
         }
      },

      -- Medium
      {
         name = "amazons_rope_weaver_booth",
         helptexts = {
            -- TRANSLATORS: Lore author helptext for an Amazon production site: Rope Weaver's Booth
            lore = pgettext("amazons_building", "‘My mother taught me to ply the ropes from Liana, how to choose the strongest cuttings and parts of the harvest, how to trim away flaws. I bless her wisdom in every length I use.’"),
            -- TRANSLATORS: Lore author helptext for an Amazon production site: Rope Weaver's Booth
            lore_author = pgettext("amazons_building", "Mistress WeaverSong, reflecting on training."),
            -- TRANSLATORS: Purpose helptext for an Amazon production site: Rope Weaver's Booth
            purpose = pgettext("amazons_building", "Makes ropes from lianas.")
         }
      },
      {
         name = "amazons_furnace",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon production site: Furnace
            purpose = pgettext("amazons_building", "Smelts gold dust into gold ingots using charcoal."),
            -- TRANSLATORS: Performance helptext for an Amazon production site: Furnace
            performance = pgettext("amazons_building", "The furnace can produce one gold ingot in %s on average if the supply is steady."):bformat(format_minutes_seconds(1, 12))
         }
      },
      {
         name = "amazons_rare_tree_plantation",
         helptexts = {
            -- TRANSLATORS: Lore helptext for an Amazon production site: Rare Tree Plantation
            lore = pgettext("amazons_building", "‘If properly harvested, cured and stored, ironwood becomes almost indestructible, and will not rust in damp conditions.’"),
            -- TRANSLATORS: Lore author helptext for an Amazon production site: Rare Tree Plantation
            lore_author = pgettext("amazons_building", "Jungle Master Abacaxis"),
            -- TRANSLATORS: Purpose helptext for an Amazon production site: Rare Tree Plantation
            purpose = pgettext("amazons_building", "Plants rare trees which are needed by the Amazons, like balsa, rubber and ironwood."),
            -- TRANSLATORS: Note helptext for an Amazon production site: Rare Tree Plantation
            note = pgettext("amazons_building", "Rare trees are planted according to the economy requirements. Keep roads connected to rare tree plantations so they adhere to economy targets.")
         }
      },
      {
         name = "amazons_stone_workshop",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon production site: Stone Workshop
            purpose = pgettext("amazons_building", "Makes tools and spears for our soldiers."),
            -- TRANSLATORS: Performance helptext for an Amazon production site: Stone Workshop
            performance = pgettext("amazons_building", "If this building is fully supplied and all tools and spears are needed by the economy, production of one of each tool takes %1$s and one of each spear takes %2$s on average. If only one kind of tool or spear is needed by the economy, its production takes %3$s on average."):bformat(format_minutes_seconds(21, 18), format_minutes_seconds(7, 6), format_minutes_seconds(1, 11))
         }
      },
      {
         name = "amazons_dressmakery",
         helptexts = {
            -- TRANSLATORS: Lore helptext for an Amazon production site: Dressmakery
            lore = pgettext("amazons_building", "‘A skilled maker crafts garments of unsurpassed defence,<br> with each stitch imbued with conscious intent.’"),
            -- TRANSLATORS: Lore author helptext for an Amazon production site: Dressmakery
            lore_author = pgettext("amazons_building", "Dressmakers’ ethos, adapted from the traditional verse ‘Mother Jungle’s wisdom: Song of the Conscientious.’"),
            -- TRANSLATORS: Purpose helptext for an Amazon production site: Dressmakery
            purpose = pgettext("amazons_building", "Sews all dresses, boots and armor for our soldiers."),
            -- TRANSLATORS: Performance helptext for an Amazon production site: Dressmakery
            performance = pgettext("amazons_building", "If this building is fully supplied and all dresses, boots and armors are needed by the economy, production of one of each of them takes %1$s on average. If only one kind of them is needed by the economy, its production takes %2$s on average."):bformat(format_minutes_seconds(10, 39), format_minutes_seconds(1, 11))
         }
      },
      {
         name = "amazons_charcoal_kiln",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon production site: Charcoal Kiln
            purpose = pgettext("amazons_building", "Burns logs into charcoal."),
            -- TRANSLATORS: Performance helptext for an Amazon production site: Charcoal Kiln
            performance = pgettext("amazons_building", "The charcoal kiln needs %s on average to produce one lump of coal."):bformat(format_minutes_seconds(2,1))
         }
      },
      {
         name = "amazons_cassava_root_cooker",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon production site: Cassava Root Cooker
            purpose = pgettext("amazons_building", "Bakes bread from cassava root.")
         }
      },
      {
         name = "amazons_chocolate_brewery",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon production site: Chocolate Brewery
            purpose = pgettext("amazons_building", "Brews chocolate for soldier training."),
            -- TRANSLATORS: Performance helptext for an Amazon production site: Chocolate Brewery
            performance = pgettext("amazons_building", "The chocolate brewery needs %s on average to brew one mug of chocolate."):bformat(format_minutes(1))
         }
      },
      {
         name = "amazons_food_preserver",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon production site: Food Preserver
            purpose = pgettext("amazons_building", "Prepares rations to feed the scouts and the workers digging for gold dust and granite."),
            -- TRANSLATORS: Performance helptext for an Amazon production site: Food Preserver
            performance = pgettext("amazons_building", "The food preserver can produce one ration in %s on average if the supply is steady."):bformat(format_seconds(17))
         }
      },
      {
         name = "amazons_initiation_site",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon production site: Initiation Site
            purpose = pgettext("amazons_building", "Equips recruits and trains them as soldiers."),
            -- TRANSLATORS: Performance helptext for an Amazon production site: Initiation Site
            performance = pgettext("amazons_building", "The initiation site needs %s on average to recruit one soldier."):bformat(format_seconds(30))
         }
      },
      {
         name = "amazons_gardening_center",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon production site: Gardening Center
            purpose = pgettext("amazons_building", "Enhances the fertility of the landscape for trees."),
            -- TRANSLATORS: Note helptext for an Amazon production site: Gardening Center
            note = pgettext ("amazons_building", "Only a Jungle Master has the necessary experience to coax more growth from land.")
         }
      },

      -- Big
      {
         name = "amazons_tapir_farm",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon production site: Tapir Farm
            purpose = pgettext("amazons_building", "Breeds tapirs for adding them to the transportation system."),
            -- TRANSLATORS: Note helptext for an Amazon production site: Tapir Farm
            note = pgettext ("amazons_building", "Tapirs help to prevent traffic jams on highly used roads.")
         }
      },
      {
         name = "amazons_cassava_plantation",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon production site: Cassava Plantation
            purpose = pgettext("amazons_building", "Sows and harvests cassava."),
            -- TRANSLATORS: Note helptext for an Amazon production site: Cassava Plantation
            note = pgettext ("amazons_building", "The cassava plantation needs free space within the work area to plant seeds.")
         }
      },
      {
         name = "amazons_cocoa_farm",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon production site: Cocoa Farm
            purpose = pgettext("amazons_building", "Cultivates cocoa that is needed to brew chocolate drinks."),
            -- TRANSLATORS: Note helptext for an Amazon production site: Cocoa Farm
            note = pgettext("amazons_building", "Chocolate is essential for training Amazon soldiers.")
         }
      },

      -- Mines
      {
         name = "amazons_stonemine",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon production site: Stone Mine
            purpose = pgettext("amazons_building", "Digs granite out of the ground in mountain terrain."),
            -- TRANSLATORS: Performance helptext for an Amazon production site: Stone Mine
            performance = pgettext("amazons_building", "The stone mine can produce one slab of granite in %s on average if the supply with rations is steady and the mine is not worn out."):bformat(format_seconds(32))
         }
      },
      {
         name = "amazons_gold_digger_dwelling",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon production site: Gold Digger Dwelling
            purpose = pgettext("amazons_building", "Washes gold dust out of the ground in mountain terrain.")
         }
      },

      -- Training Sites
      {
         name = "amazons_warriors_gathering",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon training site: Warriors’ Gathering
            purpose = pgettext("amazons_building", "Trains soldiers in Attack up to level 2. Equips the soldiers with all necessary weapons."),
            -- TRANSLATORS: Note helptext for an Amazon training site: Warriors’ Gathering
            note = pgettext("amazons_building", "Amazon soldiers need chocolate to train properly.")
         }
      },
      {
         name = "amazons_training_glade",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon training site: Training Glade
            purpose = pgettext("amazons_building", "Trains soldiers in Evade, Defense and Health. Equips the soldiers with all necessary equipment and armor parts."),
            -- TRANSLATORS: Note helptext for an Amazon training site: Training Glade
            note = pgettext("amazons_building", "Amazon soldiers need chocolate to train properly.")
         }
      },

      -- Military Sites
      {
         name = "amazons_patrol_post",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon military site: Patrol Post
            purpose = pgettext("amazons_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an Amazon military site: Patrol Post
            note = pgettext("amazons_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send her away.")
         }
      },
      {
         name = "amazons_treetop_sentry",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon military site: Treetop Sentry
            purpose = pgettext("amazons_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an Amazon military site: Treetop Sentry
            note = pgettext("amazons_building", "You can only build this building on top of a tree.")
         }
      },
      {
         name = "amazons_warriors_dwelling",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon military site: Warriors’ Dwelling
            purpose = pgettext("amazons_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an Amazon military site: Warriors’ Dwelling
            note = pgettext("amazons_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send her away.")
         }
      },
      {
         name = "amazons_tower",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon military site: Tower
            purpose = pgettext("amazons_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an Amazon military site: Tower
            note = pgettext("amazons_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send her away.")
         }
      },
      {
         name = "amazons_observation_tower",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon military site: Observation Tower
            purpose = pgettext("amazons_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an Amazon military site: Observation Tower
            note = pgettext("amazons_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send her away.")
         }
      },
      {
         name = "amazons_fortress",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon military site: Fortress
            purpose = pgettext("amazons_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an Amazon military site: Fortress
            note = pgettext("amazons_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send her away.")
         }
      },
      {
         name = "amazons_fortification",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon military site: Fortification
            purpose = pgettext("amazons_building", "Garrisons soldiers to expand your territory."),
            -- TRANSLATORS: Note helptext for an Amazon military site: Fortification
            note = pgettext("amazons_building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send her away.")
         }
      },

      -- Seafaring/Ferry Sites - these are only displayed on seafaring/ferry maps
      {
         name = "amazons_ferry_yard",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon production site: Ferry Yard
            purpose = pgettext("amazons_building", "Builds ferries."),
            note = {
               -- TRANSLATORS: Note helptext for an Amazon production site: Ferry Yard, part 1
               pgettext("amazons_building", "Needs water nearby. Be aware ferries carry wares only, no workers."),
               -- TRANSLATORS: Note helptext for an Amazon production site: Ferry Yard, part 2
               pgettext("building", "Roads and trees along the shoreline block access to water."),
            }
         }
      },
      {
         name = "amazons_shipyard",
         helptexts = {
            -- TRANSLATORS: Purpose helptext for an Amazon production site: Shipyard
            purpose = pgettext("amazons_building", "Constructs ships that are used for overseas colonization and for trading between ports."),
            note = {
               -- TRANSLATORS: Note helptext for an Amazon production site: Shipyard, part 1
               pgettext("building", "Needs wide open water nearby."),
               -- TRANSLATORS: Note helptext for an Amazon production site: Shipyard, part 2
               pgettext("building", "Roads and trees along the shoreline block access to water."),
            }
         }
      },

      -- Partially Finished Buildings - these are the same 2 buildings for all tribes
      {
         name = "constructionsite",
         helptexts = {
            -- TRANSLATORS: Lore helptext for an Amazon building: Construction Site
            lore = pgettext("building", "‘Don’t swear at the builder who is short of building materials.’"),
            -- TRANSLATORS: Lore author helptext for an Amazon building: Construction Site
            lore_author = pgettext("building", "Proverb widely used for impossible tasks of any kind"),
            -- TRANSLATORS: Purpose helptext for an Amazon building: Construction Site
            purpose = pgettext("building", "A new building is being built at this construction site.")
         }
      },
      {
         name = "dismantlesite",
         helptexts = {
            -- TRANSLATORS: Lore helptext for an Amazon building: Dismantle Site
            lore = pgettext("building", "‘New paths will appear when you are willing to tear down the old.’"),
            -- TRANSLATORS: Lore author helptext for an Amazon building: Dismantle Site
            lore_author = pgettext("building", "Proverb"),
            -- TRANSLATORS: Purpose helptext for an Amazon building: Dismantle Site
            purpose = pgettext("building", "A building is being dismantled at this dismantle site, returning some of the resources that were used during this building’s construction to your tribe’s stores.")
         }
      }
   },

   warehouse_names = {
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Alter"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Anumā"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Aquiqui"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Arapixuna"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Aritapera"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Atamanai"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Amorim"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Canindé"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Cuia"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Cacoal"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Capichauā"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Cipoal"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Cupiranga"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Curariaca"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Curupari"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Curuá"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Diogo"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Guajará"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Guarana"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Gurupá"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Ipanema"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Ipaupixuna"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Irateua"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Itacoatiara"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Itanduba"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Itapari"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Jaburu"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Jacarecapá"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Jaguara"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Janauacá"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Macapá"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Maguari"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Mambeca"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Manacapuru"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Mararu"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Maripi"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Maruá"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Mataraí"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Miri"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Moju"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Mojui"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Morimutuba"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Oriximiná"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Pacoval"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Paranquara"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Paricó"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Pedreira"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Povoado"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Prainha"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Preta"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Santarém"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Socorro"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Solimōes"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Surucá"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Tambaqui"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Tapará"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Taperinha"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Tiningu"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Tomé"),
      -- TRANSLATORS: This Amazonian warehouse is named after a town in the Amazon Basin
      pgettext("warehousename", "Una"),
      -- TRANSLATORS: This Amazonian warehouse is named after a Maya city
      pgettext("warehousename", "Bonampak"),
      -- TRANSLATORS: This Amazonian warehouse is named after a Maya city
      pgettext("warehousename", "Calakmul"),
      -- TRANSLATORS: This Amazonian warehouse is named after a Maya city
      pgettext("warehousename", "Chichén Itzá"),
      -- TRANSLATORS: This Amazonian warehouse is named after a Maya city
      pgettext("warehousename", "Copán"),
      -- TRANSLATORS: This Amazonian warehouse is named after a Maya city
      pgettext("warehousename", "Kaminaljuyú"),
      -- TRANSLATORS: This Amazonian warehouse is named after a Maya city
      pgettext("warehousename", "Lamanai"),
      -- TRANSLATORS: This Amazonian warehouse is named after a Maya city
      pgettext("warehousename", "Mayapán"),
      -- TRANSLATORS: This Amazonian warehouse is named after a Maya city
      pgettext("warehousename", "Pomoná"),
      -- TRANSLATORS: This Amazonian warehouse is named after a Maya city
      pgettext("warehousename", "Quiriguá"),
      -- TRANSLATORS: This Amazonian warehouse is named after a Maya city
      pgettext("warehousename", "Tikal"),
      -- TRANSLATORS: This Amazonian warehouse is named after a Maya city
      pgettext("warehousename", "Tononá"),
      -- TRANSLATORS: This Amazonian warehouse is named after a Maya city
      pgettext("warehousename", "Uxmal"),
      -- TRANSLATORS: This Amazonian warehouse is named after an Inca city
      pgettext("warehousename", "Arequipa"),
      -- TRANSLATORS: This Amazonian warehouse is named after an Inca city
      pgettext("warehousename", "Aypate"),
      -- TRANSLATORS: This Amazonian warehouse is named after an Inca city
      pgettext("warehousename", "Cahuachi"),
      -- TRANSLATORS: This Amazonian warehouse is named after an Inca city
      pgettext("warehousename", "Choquequirao"),
      -- TRANSLATORS: This Amazonian warehouse is named after an Inca city
      pgettext("warehousename", "Kashamarka"),
      -- TRANSLATORS: This Amazonian warehouse is named after an Inca city
      pgettext("warehousename", "Kuelap"),
      -- TRANSLATORS: This Amazonian warehouse is named after an Inca city
      pgettext("warehousename", "Machu Picchu"),
      -- TRANSLATORS: This Amazonian warehouse is named after an Inca city
      pgettext("warehousename", "Nazca"),
      -- TRANSLATORS: This Amazonian warehouse is named after an Inca city
      pgettext("warehousename", "Ollantaytambo"),
      -- TRANSLATORS: This Amazonian warehouse is named after an Inca city
      pgettext("warehousename", "Pachacamac"),
      -- TRANSLATORS: This Amazonian warehouse is named after an Inca city
      pgettext("warehousename", "Paititi"),
      -- TRANSLATORS: This Amazonian warehouse is named after an Inca city
      pgettext("warehousename", "Paqari-tampu"),
      -- TRANSLATORS: This Amazonian warehouse is named after an Inca city
      pgettext("warehousename", "Cuzco"),
      -- TRANSLATORS: This Amazonian warehouse is named after an Inca city
      pgettext("warehousename", "Teyuna"),
      -- TRANSLATORS: This Amazonian warehouse is named after an Inca city
      pgettext("warehousename", "Tiahuanaco"),
      -- TRANSLATORS: This Amazonian warehouse is named after an Inca city
      pgettext("warehousename", "Vilcabamba"),
      -- TRANSLATORS: This Amazonian warehouse is named after an Inca city
      pgettext("warehousename", "Vitcos"),
      -- TRANSLATORS: This Amazonian warehouse is named after a part of the Inca Empire
      pgettext("warehousename", "Antisuyu"),
      -- TRANSLATORS: This Amazonian warehouse is named after a part of the Inca Empire
      pgettext("warehousename", "Chinchasuyu"),
      -- TRANSLATORS: This Amazonian warehouse is named after a part of the Inca Empire
      pgettext("warehousename", "Kuntisuyu"),
      -- TRANSLATORS: This Amazonian warehouse is named after a part of the Inca Empire
      pgettext("warehousename", "Qullasuyu"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Achuar"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Aguaruna"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Aikanã"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Aimoré"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Amahuaca"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Amuesha"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Apiacá"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Arapaco"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Arara"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Ararandeura"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Arasairi"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Araueté"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Awá"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Aweti"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Banawá"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Baniwa"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Barasana"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Caeté"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Chaywita"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Cocama"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Enawene Nawe"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Goitacaz"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Himarimã"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Huambisa"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Irántxe"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Kachá"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Kagwahiva"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Kamayurá"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Kanamarí"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Kapixaná"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Kareneri"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Karitiâna"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Kaxinawá"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Kayapo"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Kobeua"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Korubu"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Kuikoro"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Machiguenga"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Majaguaje"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Makuna"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Marajoara"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Marubo"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Mayoruna"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Miriti"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Nahukuá"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Nambikwara"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Ocaina"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Omagua"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Pacanawa"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Pacawara"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Pirahã"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Potiguara"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Saraguro"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Sateré-Mawé"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Suruwaha"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Suyá"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Tabajara"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Tacana"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Tamoio"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Tapirapé"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Tariana"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Temiminó"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Terena"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Toromona"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Tremembé"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Tsimané"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Tupinambá"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Tupiniquim"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Uru-Eu-Wau-Wau"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Waorani"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Waurá"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Yaminawá"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Yaruna"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Yawalipiti"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Yawanawa"),
      -- TRANSLATORS: This Amazonian warehouse is named after an indigenous people in the Amazon Basin
      pgettext("warehousename", "Yora"),
   },

   -- Productionsite status strings

   -- TRANSLATORS: Productivity label on an Amazon building if there is 1 worker missing
   productionsite_worker_missing = pgettext("amazons", "Worker missing"),
   -- TRANSLATORS: Productivity label on an Amazon building if there is 1 worker coming
   productionsite_worker_coming = pgettext("amazons", "Worker is coming"),
   -- TRANSLATORS: Productivity label on an Amazon building if there is more than 1 worker missing. If you need plural forms here, please let us know.
   productionsite_workers_missing = pgettext("amazons", "Workers missing"),
   -- TRANSLATORS: Productivity label on an Amazon building if there is more than 1 worker coming. If you need plural forms here, please let us know.
   productionsite_workers_coming = pgettext("amazons", "Workers are coming"),
   -- TRANSLATORS: Productivity label on an Amazon building if there is 1 experienced worker missing
   productionsite_experienced_worker_missing = pgettext("amazons", "Expert missing"),
   -- TRANSLATORS: Productivity label on an Amazon building if there is more than 1 experienced worker missing. If you need plural forms here, please let us know.
   productionsite_experienced_workers_missing = pgettext("amazons", "Experts missing"),

   -- Soldier strings to be used in Military Status strings

   soldier_context = "amazons_soldier",
   soldier_0_sg = "%1% soldier (+%2%)",
   soldier_0_pl = "%1% soldiers (+%2%)",
   soldier_1_sg = "%1% soldier",
   soldier_1_pl = "%1% soldiers",
   soldier_2_sg = "%1%(+%2%) soldier (+%3%)",
   soldier_2_pl = "%1%(+%2%) soldiers (+%3%)",
   soldier_3_sg = "%1%(+%2%) soldier",
   soldier_3_pl = "%1%(+%2%) soldiers",
   -- TRANSLATORS: %1% is the number of Amazon soldiers the plural refers to. %2% is the maximum number of soldier slots in the building.
   UNUSED_soldier_0 = npgettext("amazons_soldier", "%1% soldier (+%2%)", "%1% soldiers (+%2%)", 0),
   -- TRANSLATORS: Number of Amazon soldiers stationed at a militarysite.
   UNUSED_soldier_1 = npgettext("amazons_soldier", "%1% soldier", "%1% soldiers", 0),
   -- TRANSLATORS: %1% is the number of Amazon soldiers the plural refers to. %2% are currently open soldier slots in the building. %3% is the maximum number of soldier slots in the building
   UNUSED_soldier_2 = npgettext("amazons_soldier", "%1%(+%2%) soldier (+%3%)", "%1%(+%2%) soldiers (+%3%)", 0),
   -- TRANSLATORS: %1% is the number of Amazon soldiers the plural refers to. %2% are currently open soldier slots in the building.
   UNUSED_soldier_3 = npgettext("amazons_soldier", "%1%(+%2%) soldier", "%1%(+%2%) soldiers", 0),

   -- Special types
   builder = "amazons_builder",
   carriers = {"amazons_carrier", "amazons_tapir"},
   geologist = "amazons_geologist",
   scouts_house = "amazons_scouts_hut",
   soldier = "amazons_soldier",
   ship = "amazons_ship",
   ferry = "amazons_ferry",
   port = "amazons_port",

   fastplace = {
      warehouse = "amazons_warehouse",
      port = "amazons_port",
      training_small = "amazons_warriors_gathering",
      training_large = "amazons_training_glade",
      military_small_primary = "amazons_patrol_post",
      military_small_secondary = "amazons_treetop_sentry",
      military_medium_primary = "amazons_warriors_dwelling",
      military_tower = "amazons_tower",
      military_fortress = "amazons_fortress",
      woodcutter = "amazons_woodcutters_hut",
      forester = "amazons_jungle_preservers_hut",
      quarry = "amazons_stonecutters_hut",
      building_materials_primary = "amazons_rope_weaver_booth",
      building_materials_secondary = "amazons_liana_cutters_hut",
      building_materials_tertiary = "amazons_rare_tree_plantation",
      fisher = "amazons_hunter_gatherers_hut",
      fish_meat_replenisher = "amazons_wilderness_keepers_tent",
      well = "amazons_water_gatherers_hut",
      farm_primary = "amazons_cassava_plantation",
      farm_secondary = "amazons_cocoa_farm",
      bakery = "amazons_cassava_root_cooker",
      brewery = "amazons_chocolate_brewery",
      tavern = "amazons_food_preserver",
      smelting = "amazons_furnace",
      tool_smithy = "amazons_stone_workshop",
      armor_smithy = "amazons_dressmakery",
      shipyard = "amazons_shipyard",
      ferry_yard = "amazons_ferry_yard",
      scout = "amazons_scouts_hut",
      barracks = "amazons_initiation_site",
      second_carrier = "amazons_tapir_farm",
      charcoal = "amazons_charcoal_kiln",
      mine_stone = "amazons_stonemine",
      mine_gold = "amazons_gold_digger_dwelling",
      terraforming = "amazons_gardening_center",
   },
}

pop_textdomain()
