-- Atlantean ware helptexts

-- For formatting time strings
include "tribes/scripting/help/global_helptexts.lua"

-- RST
-- .. _lua_tribes_tribes_helptexts:
--
-- Helptexts
-- ---------
--
-- Helptexts are defined in ``tribes/initialization/<tribename>/helptexts.lua`` and linked to from :ref:`units.lua <lua_tribes_tribes_units>`.
--
-- First, you will need to define a message context. The form is ``"<tribe_name>_<category>"``.
-- Do not deviate from this -- the translations will not work otherwise.
--
-- You will then need to return a table of helptexts by map object type. It looks like this:
--
-- .. code-block:: lua
--
--    local warectxt = "atlanteans_ware"
--    return {
--       wares = {
--          atlanteans_bread = {
--             -- TRANSLATORS: Helptext for a ware: Bread
--             pgettext(warectxt, "This tasty bread is made in bakeries out of cornmeal, blackroot flour and water.")
--          },
--          bread_paddle = {
--             -- TRANSLATORS: Helptext for a ware: Bread Paddle
--             pgettext(warectxt, "The bread paddle is the tool of the baker, each baker needs one."),
--             -- TRANSLATORS: Helptext for a ware: Bread Paddle
--             pgettext(warectxt, "Bread paddles are produced by the toolsmith.")
--          },
--          ...
--       }
--    }
--
-- To make life easier for our translators, you can split long helptexts into multiple entries as with the ``bread_paddle`` example above.
-- The helptexts are then joined by the engine.
-- In our example, we will get ``"The bread paddle is the tool of the baker, each baker needs one. Bread paddles are produced by the toolsmith."``

-- NOCOM update documentation + document scenario helptexts
local buildingctxt = "atlanteans_building"
local immctxt = "atlanteans_immovable"
local warectxt = "atlanteans_ware"
local workerctxt = "atlanteans_worker"

push_textdomain("tribes_encyclopedia")

local result = {
   buildings = {
      constructionsite = {
         lore = {
            -- TRANSLATORS: Lore helptext for an atlantean building: Construction Site
            pgettext("building", "‘Don’t swear at the builder who is short of building materials.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for an atlantean building: Construction Site
            pgettext("building", "Proverb widely used for impossible tasks of any kind")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean building: Construction Site
            pgettext("building", "A new building is being built at this construction site.")
         }
      },
      dismantlesite = {
         lore = {
            -- TRANSLATORS: Lore helptext for an atlantean building: Dismantle Site
            pgettext("building", "‘New paths will appear when you are willing to tear down the old.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for an atlantean building: Dismantle Site
            pgettext("building", "Proverb")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean building: Dismantle Site
            pgettext("building", "A building is being dismantled at this dismantle site, returning some of the resources that were used during this building’s construction to your tribe’s stores.")
         }
      },
      atlanteans_castle = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean military site: Castle
            pgettext("building", "Garrisons soldiers to expand your territory.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an atlantean military site: Castle
            pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      atlanteans_guardhall = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean military site: Guardhall
            pgettext("building", "Garrisons soldiers to expand your territory.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an atlantean military site: Guardhall
            pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      atlanteans_guardhouse = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean military site: Guardhouse
            pgettext("building", "Garrisons soldiers to expand your territory.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an atlantean military site: Guardhouse
            pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      atlanteans_tower = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean military site: Tower
            pgettext("building", "Garrisons soldiers to expand your territory.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an atlantean military site: Tower
            pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      atlanteans_tower_high = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean military site: High Tower
            pgettext("building", "Garrisons soldiers to expand your territory.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an atlantean military site: High Tower
            pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      atlanteans_tower_small = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean military site: Small Tower
            pgettext("building", "Garrisons soldiers to expand your territory.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an atlantean military site: Small Tower
            pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      atlanteans_armorsmithy = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Armor Smithy
            pgettext(buildingctxt, "Forges shields that are used for training soldiers’ defense in the labyrinth.")
         }
      },
      atlanteans_bakery = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Bakery
            pgettext(buildingctxt, "Bakes bread to feed the scouts and miners and to train soldiers.")
         }
      },
      atlanteans_barracks = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Barracks
            pgettext(buildingctxt, "Equips recruits and trains them as soldiers.")
         }
      },
      atlanteans_blackroot_farm = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Blackroot Farm
            pgettext(buildingctxt, "Sows and harvests blackroot.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for an atlantean production site: Blackroot Farm
            pgettext(buildingctxt, "The blackroot farmer needs %1% on average to sow and harvest a bundle of blackroot."):bformat(format_minutes_seconds(1, 20))
         }
      },
      atlanteans_charcoal_kiln = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Charcoal Kiln
            pgettext("building", "Burns logs into charcoal.")
         }
      },
      atlanteans_coalmine = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Coal Mine
            pgettext("building", "Digs coal out of the ground in mountain terrain.")
         }
      },
      atlanteans_crystalmine = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Crystal Mine
            pgettext(buildingctxt, "Carves precious stones out of the rock in mountain terrain.")
         }
      },
      atlanteans_farm = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Farm
            pgettext(buildingctxt, "Sows and harvests corn.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for an atlantean production site: Farm
            pgettext(buildingctxt, "The farmer needs %1% on average to sow and harvest a sheaf of corn."):bformat(format_minutes_seconds(1, 20))
         }
      },
      atlanteans_ferry_yard = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Ferry Yard
            pgettext("building", "Builds ferries.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an atlantean production site: Ferry Yard
            pgettext("building", "Needs water nearby.")
         }
      },
      atlanteans_fishbreeders_house = {
         lore = {
            -- TRANSLATORS: Lore helptext for an atlantean production site: Fish Breeder's House
            pgettext(buildingctxt, [[‘Only after the last duck has been shot down<br>]] ..
                                   [[Only after the last deer has been put to death<br>]] ..
                                   [[Only after the last fish has been caught<br>]] ..
                                   [[Then will you find that spiders are not to be eaten.’]])
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for an atlantean production site: Fish Breeder's House
            pgettext(buildingctxt, "Prophecy of the fish breeders")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Fish Breeder's House
            pgettext(buildingctxt, "Breeds fish.")
         }
      },
      atlanteans_fishers_house = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Fisher's House
            pgettext(buildingctxt, "Fishes on the coast near the fisher’s house.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an atlantean production site: Fisher's House, part 1
            pgettext(buildingctxt, "The fisher’s house needs water full of fish within the work area."),
            -- TRANSLATORS: Note helptext for an atlantean production site: Fisher's House, part 2
            pgettext(buildingctxt, "Build a fish breeder’s house close to the fisher’s house to make sure that you don’t run out of fish.")
         }
      },
      atlanteans_foresters_house = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Forester's House
            pgettext("building", "Plants trees in the surrounding area.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an atlantean production site: Forester's House
            pgettext(buildingctxt, "The forester’s house needs free space within the work area to plant the trees.")
         }
      },
      atlanteans_goldmine = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Gold Mine
            pgettext("building", "Digs gold ore out of the ground in mountain terrain.")
         }
      },
      atlanteans_gold_spinning_mill = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Gold Spinning Mill
            pgettext(buildingctxt, "Spins gold thread out of gold.")
         }
      },
      atlanteans_horsefarm = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Horse Farm
            pgettext(buildingctxt, "Breeds the strong Atlantean horses for adding them to the transportation system.")
         }
      },
      atlanteans_hunters_house = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Hunter's House
            pgettext("building", "Hunts animals to produce meat.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an atlantean production site: Hunter's House
            pgettext(buildingctxt, "The hunter’s house needs animals to hunt within the work area.")
         }
      },
      atlanteans_ironmine = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Iron Mine
            pgettext("building", "Digs iron ore out of the ground in mountain terrain.")
         }
      },
      atlanteans_mill = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Mill
            pgettext(buildingctxt, "Grinds blackroots and corn to produce blackroot flour and cornmeal, respectively.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an atlantean production site: Mill
            pgettext(buildingctxt, "When no cornmeal is required, the mill will try to produce blackroot flour even when there is no demand for it.")
         }
      },
      atlanteans_quarry = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Quarry
            pgettext(buildingctxt, "Cuts blocks of granite out of rocks in the vicinity.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an atlantean production site: Quarry
            pgettext(buildingctxt, "The quarry needs rocks to cut within the work area.")
         }
      },
      atlanteans_sawmill = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Sawmill
            pgettext("building", "Saws logs to produce planks.")
         }
      },
      atlanteans_scouts_house = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Scout's House
            pgettext("building", "Explores unknown territory.")
         }
      },
      atlanteans_shipyard = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Shipyard
            pgettext("building", "Constructs ships that are used for overseas colonization and for trading between ports.")
         }
      },
      atlanteans_smelting_works = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Smelting Works
            pgettext("building", "Smelts iron ore into iron and gold ore into gold.")
         }
      },
      atlanteans_smokery = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Smokery
            pgettext(buildingctxt, "Smokes meat and fish to feed the scouts and miners and to train soldiers in the dungeon.")
         }
      },
      atlanteans_spiderfarm = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Spider Farm
            pgettext("building", "Breeds spiders for silk.")
         }
      },
      atlanteans_toolsmithy = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Toolsmithy
            pgettext("building", "Forges all the tools that your workers need.")
         }
      },
      atlanteans_weaponsmithy = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Weapon Smithy
            pgettext(buildingctxt, "Forges tridents to equip the soldiers and to train their attack in the dungeon.")
         }
      },
      atlanteans_weaving_mill = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Weaving Mill
            pgettext(buildingctxt, "Weaves spidercloth for buildings and ships’ sails, and tabards to equip and train the soldiers.")
         }
      },
      atlanteans_well = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Well
            pgettext("building", "Draws water out of the deep.")
         }
      },
      atlanteans_woodcutters_house = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean production site: Woodcutter's House
            pgettext("building", "Fells trees in the surrounding area and processes them into logs.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an atlantean production site: Woodcutter's House
            pgettext(buildingctxt, "The woodcutter’s house needs trees to fell within the work area.")
         }
      },
      atlanteans_dungeon = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean training site: Dungeon, part 1
            pgettext("atlanteans_building", "Trains soldiers in ‘Attack’."),
            -- TRANSLATORS: Purpose helptext for an atlantean training site: Dungeon, part 2
            pgettext("atlanteans_building", "Equips the soldiers with all necessary weapons and armor parts.")
         }
      },
      atlanteans_labyrinth = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean training site: Labyrinth, part 1
            pgettext("atlanteans_building", "Trains soldiers in ‘Defense’, ‘Evade’, and ‘Health’."),
            -- TRANSLATORS: Purpose helptext for an atlantean training site: Labyrinth, part 2
            pgettext("atlanteans_building", "Equips the soldiers with all necessary weapons and armor parts.")
         }
      },
      atlanteans_headquarters = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean warehouse: Headquarters
            pgettext(buildingctxt, "Accommodation for your people. Also stores your wares and tools.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an atlantean warehouse: Headquarters
            pgettext(buildingctxt, "The headquarters is your main building.")
         }
      },
      atlanteans_port = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean warehouse: Port
            pgettext(buildingctxt, "Serves as a base for overseas colonization and trade. Also stores your soldiers, wares and tools.")
         }
      },
      atlanteans_warehouse = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an atlantean warehouse: Warehouse
            pgettext("building", "Your workers and soldiers will find shelter here. Also stores your wares and tools.")
         }
      }
   },
   immovables = {
      ashes = {
         -- TRANSLATORS: Helptext for a tribe immovable: Ashes
         purpose = { pgettext(immctxt, "The remains of a destroyed building.") }
      },
      blackrootfield_harvested = {
         -- TRANSLATORS: Helptext for a tribe immovable: Blackroot Field
         purpose = { pgettext(immctxt, "This field has been harvested.") }
      },
      blackrootfield_medium = {
         -- TRANSLATORS: Helptext for a tribe immovable: Blackroot Field
         purpose = { pgettext(immctxt, "This field is growing.") }
      },
      blackrootfield_ripe = {
         -- TRANSLATORS: Helptext for a tribe immovable: Blackroot Field
         purpose = { pgettext(immctxt, "This field is ready for harvesting.") }
      },
      blackrootfield_small = {
         -- TRANSLATORS: Helptext for a tribe immovable: Blackroot Field
         purpose = { pgettext(immctxt, "This field is growing.") }
      },
      blackrootfield_tiny = {
         -- TRANSLATORS: Helptext for a tribe immovable: Blackroot Field
         purpose = { pgettext(immctxt, "This field has just been planted.") }
      },
      cornfield_harvested = {
         -- TRANSLATORS: Helptext for a tribe immovable: Corn Field
         purpose = { pgettext(immctxt, "This field has been harvested.") }
      },
      cornfield_medium = {
         -- TRANSLATORS: Helptext for a tribe immovable: Corn Field
         purpose = { pgettext(immctxt, "This field is growing.") }
      },
      cornfield_ripe = {
         -- TRANSLATORS: Helptext for a tribe immovable: Corn Field
         purpose = { pgettext(immctxt, "This field is ready for harvesting.") }
      },
      cornfield_small = {
         -- TRANSLATORS: Helptext for a tribe immovable: Corn Field
         purpose = { pgettext(immctxt, "This field is growing.") }
      },
      cornfield_tiny = {
         -- TRANSLATORS: Helptext for a tribe immovable: Corn Field
         purpose = { pgettext(immctxt, "This field has just been planted.") }
      },
      destroyed_building = {
         -- TRANSLATORS: Helptext for a tribe immovable: Destroyed Building
         purpose = { pgettext(immctxt, "The remains of a destroyed building.") }
      },
      atlanteans_resi_coal_1 = {
         purpose = {
            -- TRANSLATORS: Helptext for a resource indicator: Coal
            pgettext(immctxt, "Coal veins contain coal that can be dug up by coal mines."),
            -- TRANSLATORS: Helptext for a resource indicator: Coal
            pgettext(immctxt, "There is only a little bit of coal here.")
         }
      },
      atlanteans_resi_coal_2 = {
         purpose = {
            -- TRANSLATORS: Helptext for a resource indicator: Coal
            pgettext(immctxt, "Coal veins contain coal that can be dug up by coal mines."),
            -- TRANSLATORS: Helptext for a resource indicator: Coal
            pgettext(immctxt, "There is a lot of coal here.")
         }
      },
      atlanteans_resi_gold_1 = {
         purpose = {
            -- TRANSLATORS: Helptext for a resource indicator: Gold
            pgettext(immctxt, "Gold veins contain gold ore that can be dug up by gold mines."),
            -- TRANSLATORS: Helptext for a resource indicator: Gold
            pgettext(immctxt, "There is only a little bit of gold here.")
         }
      },
      atlanteans_resi_gold_2 = {
         purpose = {
            -- TRANSLATORS: Helptext for a resource indicator: Gold
            pgettext(immctxt, "Gold veins contain gold ore that can be dug up by gold mines."),
            -- TRANSLATORS: Helptext for a resource indicator: Gold
            pgettext(immctxt, "There is a lot of gold here.")
         }
      },
      atlanteans_resi_iron_1 = {
         purpose = {
            -- TRANSLATORS: Helptext for a resource indicator: Iron
            pgettext(immctxt, "Iron veins contain iron ore that can be dug up by iron mines."),
            -- TRANSLATORS: Helptext for a resource indicator: Iron
            pgettext(immctxt, "There is only a little bit of iron here.")
         }
      },
      atlanteans_resi_iron_2 = {
         purpose = {
            -- TRANSLATORS: Helptext for a resourc indicatore: Iron
            pgettext(immctxt, "Iron veins contain iron ore that can be dug up by iron mines."),
            -- TRANSLATORS: Helptext for a resource indicator: Iron
            pgettext(immctxt, "There is a lot of iron here.")
         }
      },
      atlanteans_resi_none = {
         -- TRANSLATORS: Helptext for a resource indicator: No resources
         purpose = { pgettext(immctxt, "There are no resources in the ground here.") }
      },
      atlanteans_resi_water = {
         -- TRANSLATORS: Helptext for a resource indicator: Water
         purpose = { pgettext(immctxt, "There is water in the ground here that can be pulled up by a well.") }
      },
      atlanteans_resi_stones_1 = {
         purpose = {
            -- TRANSLATORS: Helptext for an Atlantean resource indicator: Stones
            pgettext(immctxt, "Precious stones are used in the construction of big buildings. They can be dug up by a crystal mine. You will also get granite from the mine."),
            -- TRANSLATORS: Helptext for an Atlantean resource indicator: Stones
            pgettext(immctxt, "There are only a few precious stones here.")
         }
      },
      atlanteans_resi_stones_2 = {
         purpose = {
            -- TRANSLATORS: Helptext for an Atlantean resource indicator: Stones
            pgettext(immctxt, "Precious stones are used in the construction of big buildings. They can be dug up by a crystal mine. You will also get granite from the mine."),
            -- TRANSLATORS: Helptext for an Atlantean resource indicator: Stones
            pgettext(immctxt, "There are many precious stones here.")
         }
      },
      atlanteans_shipconstruction = {
         -- TRANSLATORS: Helptext for a tribe immovable: Ship Under Construction
         purpose = { pgettext(immctxt, "A ship is being constructed at this site.") }
      }
   },
   wares = {
      atlanteans_bread = {
         -- TRANSLATORS: Helptext for a ware: Bread
         purpose = { pgettext(warectxt, "This tasty bread is made in bakeries out of cornmeal, blackroot flour and water. It is appreciated as basic food by miners, scouts and soldiers in training sites (labyrinth and dungeon).") }
      },
      blackroot = {
         -- TRANSLATORS: Helptext for a ware: Blackroot
         purpose = { pgettext(warectxt, "Blackroots are a special kind of root produced at blackroot farms and processed in mills. The Atlanteans like their strong taste and use their flour for making bread.") }
      },
      blackroot_flour = {
         -- TRANSLATORS: Helptext for a ware: Blackroot Flour
         purpose = { pgettext(warectxt, "Blackroot Flour is produced in mills out of blackroots. It is used in bakeries to make a tasty bread.") }
      },
      bread_paddle = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Bread Paddle
            pgettext(warectxt, "The bread paddle is the tool of the baker, each baker needs one."),
            -- TRANSLATORS: Helptext for a ware: Bread Paddle
            pgettext(warectxt, "Bread paddles are produced by the toolsmith.")
         }
      },
      buckets = {
         -- TRANSLATORS: Helptext for a ware: Buckets
         purpose = { pgettext(warectxt, "Big buckets for the fish breeder – produced by the toolsmith.") }
      },
      coal = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Coal
            pgettext(warectxt, "Coal is mined in coal mines or produced out of logs by a charcoal kiln."),
            -- TRANSLATORS: Helptext for a ware: Coal
            pgettext(warectxt, "The Atlantean fires in smelting works, armor smithies and weapon smithies are fed with coal.")
         }
      },
      corn = {
         -- TRANSLATORS: Helptext for a ware: Corn
         purpose = { pgettext(warectxt, "This corn is processed in the mill into fine cornmeal that every Atlantean baker needs for a good bread. Also horse and spider farms need to be provided with corn.") }
      },
      cornmeal = {
         -- TRANSLATORS: Helptext for a ware: Cornmeal
         purpose = { pgettext(warectxt, "Cornmeal is produced in a mill out of corn and is one of three parts of the Atlantean bread produced in bakeries.") }
      },
      diamond = {
         -- TRANSLATORS: Helptext for a ware: Diamond
         purpose = { pgettext(warectxt, "These wonderful diamonds are used to build some exclusive buildings. They are mined in a crystal mine.") }
      },
      fire_tongs = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Fire Tongs
            pgettext(warectxt, "Fire tongs are the tools for smelting ores."),
            -- TRANSLATORS: Helptext for a ware: Fire Tongs
            pgettext("atlanteans_ware_fire_tongs", "They are used in the smelting works and produced by the toolsmith.")
         }
      },
      fish = {
         -- TRANSLATORS: Helptext for a ware: Fish
         purpose = { pgettext(warectxt, "Fish is one of the biggest food resources of the Atlanteans. It has to be smoked in a smokery before being delivered to mines, training sites and scouts.") }
      },
      fishing_net = {
         -- TRANSLATORS: Helptext for a ware: Fishing Net
         purpose = { pgettext(warectxt, "The fishing net is used by the fisher and produced by the toolsmith.") }
      },
      gold = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Gold
            pgettext(warectxt, "Gold is the most valuable of all metals, and it is smelted out of gold ore."),
            -- TRANSLATORS: Helptext for a ware: Gold
            pgettext(warectxt, "It is produced by the smelting works and used by the armor smithy, the weapon smithy and the gold spinning mill.")
         }
      },
      gold_ore = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Gold Ore
            pgettext(warectxt, "Gold ore is mined in a gold mine."),
            -- TRANSLATORS: Helptext for a ware: Gold Ore
            pgettext(warectxt, "Smelted in a smelting works, it turns into gold which is used as a precious building material and to produce weapons and armor.")
         }
      },
      gold_thread = {
         -- TRANSLATORS: Helptext for a ware: Gold Thread
         purpose = { pgettext(warectxt, "This thread, made of gold by the gold spinning mill, is used for weaving the exclusive golden tabard in the weaving mill.") }
      },
      granite = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Granite
            pgettext(warectxt, "Granite is a basic building material."),
            -- TRANSLATORS: Helptext for a ware: Granite
            pgettext(warectxt, "The Atlanteans produce granite blocks in quarries and crystal mines.")
         }
      },
      hammer = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Hammer
            pgettext(warectxt, "The hammer is an essential tool."),
            -- TRANSLATORS: Helptext for a ware: Hammer
            pgettext(warectxt, "Geologists, builders, weaponsmiths and armorsmiths all need a hammer. Make sure you’ve always got some in reserve! They are produced by the toolsmith.")
         }
      },
      hook_pole = {
         -- TRANSLATORS: Helptext for a ware: Hook Pole
         purpose = { pgettext(warectxt, "This hook pole is used by the smoker to suspend all the meat and fish from the top of the smokery. It is created by the toolsmith.") }
      },
      hunting_bow = {
         -- TRANSLATORS: Helptext for a ware: Hunting Bow
         purpose = { pgettext(warectxt, "This bow is used by the Atlantean hunter. It is produced by the toolsmith.") }
      },
      iron = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Iron
            pgettext(warectxt, "Iron is smelted out of iron ores."),
            -- TRANSLATORS: Helptext for a ware: Iron
            pgettext(warectxt, "It is produced by the smelting works and used in the toolsmithy, armor smithy and weapon smithy.")
         }
      },
      iron_ore = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Iron Ore
            pgettext(warectxt, "Iron ore is mined in iron mines."),
            -- TRANSLATORS: Helptext for a ware: Iron Ore
            pgettext(warectxt, "It is smelted in a smelting works to retrieve the iron.")
         }
      },
      log = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Log
            pgettext(warectxt, "Logs are an important basic building material. They are produced by felling trees."),
            -- TRANSLATORS: Helptext for a ware: Log
            pgettext(warectxt, "Atlanteans use logs also as the base for planks, which are used in nearly every building. Besides the sawmill, the charcoal kiln, the toolsmithy and the smokery also need logs for their work.")
         }
      },
      meat = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Meat
            pgettext(warectxt, "Meat contains a lot of energy, and it is obtained from wild game taken by hunters."),
            -- TRANSLATORS: Helptext for a ware: Meat
            pgettext(warectxt, "Meat has to be smoked in a smokery before being delivered to mines and training sites (dungeon and labyrinth).")
         }
      },
      milking_tongs = {
         -- TRANSLATORS: Helptext for a ware: Milking Tongs
         purpose = { pgettext(warectxt, "Milking tongs are used by the spider breeder to milk the spiders. They are produced by the toolsmith.") }
      },
      pick = {
         -- TRANSLATORS: Helptext for a ware: Pick
         purpose = { pgettext(warectxt, "Picks are used by stonecutters and miners. They are produced by the toolsmith.") }
      },
      planks = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Planks
            pgettext(warectxt, "Planks are an important building material."),
            -- TRANSLATORS: Helptext for a ware: Planks
            pgettext(warectxt, "They are produced out of logs by the sawmill."),
            -- TRANSLATORS: Helptext for a ware: Planks
            pgettext(warectxt, "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.")
         }
      },
      quartz = {
         -- TRANSLATORS: Helptext for a ware: Quartz
         purpose = { pgettext(warectxt, "These transparent quartz gems are used to build some exclusive buildings. They are produced in a crystal mine.") }
      },
      saw = {
         -- TRANSLATORS: Helptext for a ware: Saw
         purpose = { pgettext(warectxt, "The saw is needed by the sawyer, the woodcutter and the toolsmith. It is produced by the toolsmith.") }
      },
      scythe = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Scythe
            pgettext(warectxt, "The scythe is the tool of the farmers."),
            -- TRANSLATORS: Helptext for a ware: Scythe
            pgettext(warectxt, "Scythes are produced by the toolsmith.")
         }
      },
      shield_advanced = {
         -- TRANSLATORS: Helptext for a ware: Advanced Shield
         purpose = { pgettext(warectxt, "These advanced shields are used by the best soldiers of the Atlanteans. They are produced in the armor smithy and used in the labyrinth – together with food – to train soldiers from defense level 1 to level 2.") }
      },
      shield_steel = {
         -- TRANSLATORS: Helptext for a ware: Steel Shield
         purpose = { pgettext(warectxt, "This steel shield is produced in the armor smithy and used in the labyrinth – together with food – to train soldiers from defense level 0 to level 1.") }
      },
      shovel = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Shovel
            pgettext(warectxt, "Shovels are needed for the proper handling of plants."),
            -- TRANSLATORS: Helptext for a ware: Shovel
            pgettext(warectxt, "Therefore the forester and the blackroot farmer use them. They are produced by the toolsmith.")
         }
      },
      smoked_fish = {
         -- TRANSLATORS: Helptext for a ware: Smoked Fish
         purpose = { pgettext(warectxt, "As no Atlantean likes raw fish, smoking it in a smokery is the most common way to make it edible.") }
      },
      smoked_meat = {
         -- TRANSLATORS: Helptext for a ware: Smoked Meat
         purpose = { pgettext(warectxt, "Smoked meat is made out of meat in a smokery. It is delivered to the mines and training sites (labyrinth and dungeon) where the miners and soldiers prepare a nutritious lunch for themselves.") }
      },
      spider_silk = {
         -- TRANSLATORS: Helptext for a ware: Spider Silk
         purpose = { pgettext(warectxt, "Spider silk is produced by spiders, which are bred by spider farms. It is processed into spidercloth in a weaving mill.") }
      },
      spidercloth = {
         -- TRANSLATORS: Helptext for a ware: Spidercloth
         purpose = { pgettext(warectxt, "Spidercloth is made out of spider silk in a weaving mill. It is used in the toolsmithy and the shipyard. Also some higher developed buildings need spidercloth for their construction.") }
      },
      tabard = {
         -- TRANSLATORS: Helptext for a ware: Tabard
         purpose = { pgettext(warectxt, "A tabard and a light trident are the basic equipment for young soldiers. Tabards are produced in the weaving mill.") }
      },
      tabard_golden = {
         -- TRANSLATORS: Helptext for a ware: Golden Tabard
         purpose = { pgettext(warectxt, "Golden tabards are produced in Atlantean weaving mills out of gold thread. They are used in the labyrinth – together with food – to train soldiers from health level 0 to level 1.") }
      },
      trident_double = {
         -- TRANSLATORS: Helptext for a ware: Double Trident
         purpose = { pgettext(warectxt, "The double trident is one of the best tridents produced by the Atlantean weapon smithy. It is used in a dungeon – together with food – to train soldiers from attack level 2 to level 3.") }
      },
      trident_heavy_double = {
         -- TRANSLATORS: Helptext for a ware: Heavy Double Trident
         purpose = { pgettext(warectxt, "This is the most dangerous weapon of the Atlantean military. Only the best of the best soldiers may use it. It is produced in the weapon smithy and used in the dungeon – together with food – to train soldiers from attack level 3 to level 4.") }
      },
      trident_light = {
         -- TRANSLATORS: Helptext for a ware: Light Trident
         purpose = { pgettext(warectxt, "This is the basic weapon of the Atlantean soldiers. Together with a tabard, it makes up the equipment of young soldiers. Light tridents are produced in the weapon smithy as are all other tridents.") }
      },
      trident_long = {
         -- TRANSLATORS: Helptext for a ware: Long Trident
         purpose = { pgettext(warectxt, "The long trident is the first trident in the training of soldiers. It is produced in the weapon smithy and used in the dungeon – together with food – to train soldiers from attack level 0 to level 1.") }
      },
      trident_steel = {
         -- TRANSLATORS: Helptext for a ware: Steel Trident
         purpose = { pgettext(warectxt, "This is the medium trident. It is produced in the weapon smithy and used by advanced soldiers in the dungeon – together with food – to train from attack level 1 to level 2.") }
      },
      water = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Water
            pgettext(warectxt, "Water is the essence of life!"),
            -- TRANSLATORS: Helptext for a ware: Water
            pgettext(warectxt, "Water is used in the bakery and the horse and spider farms.")
         }
      }
   },
   workers = {
      atlanteans_armorsmith = {
         -- TRANSLATORS: Helptext for an atlantean worker: Armorsmith
         purpose = { pgettext(workerctxt, "Produces armor for the soldiers.") }
      },
   }
}
pop_textdomain()
return result
