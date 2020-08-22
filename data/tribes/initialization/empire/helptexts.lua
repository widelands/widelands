-- Empire ware helptexts

-- For formatting time strings
include "tribes/scripting/help/global_helptexts.lua"

local buildingctxt = "empire_building"
local warectxt = "empire_ware"
local workerctxt = "empire_worker"

push_textdomain("tribes_encyclopedia")

local result = {
   buildings = {
      constructionsite = {
         lore = {
            -- TRANSLATORS: Lore helptext for an empire building: Construction Site
            pgettext("building", "‘Don’t swear at the builder who is short of building materials.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for an empire building: Construction Site
            pgettext("building", "Proverb widely used for impossible tasks of any kind")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire building: Construction Site
            pgettext("building", "A new building is being built at this construction site.")
         }
      },
      dismantlesite = {
         lore = {
            -- TRANSLATORS: Lore helptext for an empire building: Dismantle Site
            pgettext("building", "‘New paths will appear when you are willing to tear down the old.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for an empire building: Dismantle Site
            pgettext("building", "Proverb")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire building: Dismantle Site
            pgettext("building", "A building is being dismantled at this dismantle site, returning some of the resources that were used during this building’s construction to your tribe’s stores.")
         }
      },
      empire_barrier = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire military site: Barrier
            pgettext("building", "Garrisons soldiers to expand your territory.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an empire military site: Barrier
            pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      empire_blockhouse = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire military site: Blockhouse
            pgettext("building", "Garrisons soldiers to expand your territory.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an empire military site: Blockhouse
            pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      empire_castle = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire military site: Castle
            pgettext("building", "Garrisons soldiers to expand your territory.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an empire military site: Castle
            pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      empire_fortress = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire military site: Fortress
            pgettext("building", "Garrisons soldiers to expand your territory.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an empire military site: Fortress
            pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      empire_outpost = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire military site: Outpost
            pgettext("building", "Garrisons soldiers to expand your territory.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an empire military site: Outpost
            pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      empire_sentry = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire military site: Sentry
            pgettext("building", "Garrisons soldiers to expand your territory.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an empire military site: Sentry
            pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      empire_tower = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire military site: Tower
            pgettext("building", "Garrisons soldiers to expand your territory.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an empire military site: Tower
            pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      empire_armorsmithy = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Armor Smithy
            pgettext(buildingctxt, "Forges armor and helmets that are used for training soldiers’ health in the training camp.")
         }
      },
      empire_bakery = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Bakery
            pgettext(buildingctxt, "Bakes bread for soldiers and miners alike.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for an empire production site: Bakery
            pgettext(buildingctxt, "The baker needs %1% on average to bake a loaf of bread."):bformat(format_seconds(34))
         }
      },
      empire_barracks = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Barracks
            pgettext(buildingctxt, "Equips recruits and trains them as soldiers.")
         }
      },
      empire_brewery = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Brewery
            pgettext("building", "Produces beer to keep the miners strong and happy.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for an empire production site: Brewery
            pgettext(buildingctxt, "The brewer needs %1% on average to brew a vat of beer."):bformat(format_minutes_seconds(1, 5))
         }
      },
      empire_charcoal_kiln = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Charcoal Kiln
            pgettext("building", "Burns logs into charcoal.")
         }
      },
      empire_coalmine = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Coal Mine
            pgettext("building", "Digs coal out of the ground in mountain terrain.")
         }
      },
      empire_coalmine_deep = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Deep Coal Mine
            pgettext("building", "Digs coal out of the ground in mountain terrain.")
         },
      },
      empire_donkeyfarm = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Donkey Farm
            pgettext(buildingctxt, "Breeds cute and helpful donkeys for adding them to the transportation system.")
         }
      },
      empire_farm = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Farm
            pgettext("building", "Sows and harvests wheat.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for an empire production site: Farm
            pgettext(buildingctxt, "The farmer needs %1% on average to sow and harvest a sheaf of wheat."):bformat(format_minutes_seconds(1, 20))
         }
      },
      empire_ferry_yard = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Ferry Yard
            pgettext("building", "Builds ferries.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an empire production site: Ferry Yard
            pgettext("building", "Needs water nearby.")
         }
      },
      empire_fishers_house = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Fisher's House
            pgettext(buildingctxt, "Fishes on the coast near the fisher’s house.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an empire production site: Fisher's House
            pgettext(buildingctxt, "The fisher’s house needs water full of fish within the work area.")
         }
      },
      empire_foresters_house = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Forester's House
            pgettext(buildingctxt, "Plants trees in the surrounding area.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an empire production site: Forester's House
            pgettext(buildingctxt, "The forester’s house needs free space within the work area to plant the trees.")
         }
      },
      empire_goldmine = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Gold Mine
            pgettext("building", "Digs gold ore out of the ground in mountain terrain.")
         }
      },
      empire_goldmine_deep = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Deep Gold Mine
            pgettext("building", "Digs gold ore out of the ground in mountain terrain.")
         }
      },
      empire_hunters_house = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Hunter's House
            pgettext("building", "Hunts animals to produce meat.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an empire production site: Hunter's House
            pgettext(buildingctxt, "The hunter’s house needs animals to hunt within the work area.")
         }
      },
      empire_inn = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Inn
            pgettext(buildingctxt, "Prepares rations for scouts and rations and snacks to feed the miners in all mines.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for an empire production site: Inn
            pgettext(buildingctxt, "The innkeeper needs %1% on average to prepare a ration and a meal. If the economy doesn’t need both, the innkeeper needs %2% on average to prepare one ration or one meal."):bformat(format_minutes_seconds(1, 25), format_seconds(48))
         }
      },
      empire_ironmine = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Iron Mine
            pgettext("building", "Digs iron ore out of the ground in mountain terrain.")
         }
      },
      empire_ironmine_deep = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Deep Iron Mine
            pgettext("building", "Digs iron ore out of the ground in mountain terrain.")
         }
      },
      empire_lumberjacks_house = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Lumberjack's House
            pgettext("building", "Fells trees in the surrounding area and processes them into logs.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an empire production site: Lumberjack's House
            pgettext(buildingctxt, "The lumberjack's house needs trees to fell within the work area.")
         }
      },
      empire_marblemine = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Marble Mine
            pgettext(buildingctxt, "Carves marble and granite out of the rock in mountain terrain.")
         }
      },
      empire_marblemine_deep = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Deep Marble Mine
            pgettext(buildingctxt, "Carves marble and granite out of the rock in mountain terrain.")
         }
      },
      empire_mill = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Mill
            pgettext("building", "Grinds wheat to produce flour.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for an empire production site: Mill
            pgettext(buildingctxt, "The miller needs %1% on average to grind wheat into a sack of flour."):bformat(format_seconds(19))
         }
      },
      empire_piggery = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Piggery
            pgettext(buildingctxt, "Breeds pigs for their meat.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for an empire production site: Piggery
            pgettext(buildingctxt, "The pig breeder needs %1% on average to raise and slaughter a pig."):bformat(format_minutes(1))
         }
      },
      empire_quarry = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Quarry
            pgettext(buildingctxt, "Cuts blocks of granite and marble out of rocks in the vicinity.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an empire production site: Quarry
            pgettext(buildingctxt, "The quarry needs rocks to cut within the work area.")
         }
      },
      empire_sawmill = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Sawmill
            pgettext("building", "Saws logs to produce planks.")
         }
      },
      empire_scouts_house = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Scout's House
            pgettext("building", "Explores unknown territory.")
         }
      },
      empire_sheepfarm = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Sheep Farm
            pgettext(buildingctxt, "Keeps sheep for their wool.")
         }
      },
      empire_shipyard = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Shipyard
            pgettext("building", "Constructs ships that are used for overseas colonization and for trading between ports.")
         }
      },
      empire_smelting_works = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Smelting Works
            pgettext("building", "Smelts iron ore into iron and gold ore into gold.")
         }
      },
      empire_stonemasons_house = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Stonemason's House
            pgettext(buildingctxt, "Carves marble columns out of marble.")
         }
      },
      empire_tavern = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Tavern
            pgettext("building", "Prepares rations to feed the scouts and miners.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for an empire production site: Tavern
            pgettext(buildingctxt, "The innkeeper needs %1% on average to prepare a ration."):bformat(format_seconds(36))
         }
      },
      empire_toolsmithy = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Toolsmithy
            pgettext("building", "Forges all the tools that your workers need.")
         }
      },
      empire_vineyard = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Vineyard
            pgettext(buildingctxt, "Plants grapevines and harvests grapes.")
         }
      },
      empire_weaponsmithy = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Weapon Smithy
            pgettext(buildingctxt, "Forges spears to equip the soldiers and to train their attack in the training camp.")
         }
      },
      empire_weaving_mill = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Weaving Mill
            pgettext(buildingctxt, "Weaves cloth out of wool.")
         }
      },
      empire_well = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Well
            pgettext("building", "Draws water out of the deep.")
         }
      },
      empire_winery = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire production site: Winery
            pgettext(buildingctxt, "Produces wine.")
         }
      },
      empire_arena = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire training site: Arena, part 1
            pgettext(buildingctxt, "Trains soldiers in ‘Evade’."),
            -- TRANSLATORS: Purpose helptext for an empire training site: Arena, part 2
            pgettext(buildingctxt, "‘Evade’ increases the soldier’s chance not to be hit by the enemy and so to remain totally unaffected.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an empire training site: Arena
            pgettext(buildingctxt, "Imperial soldiers cannot be trained in ‘Defense’ and will remain at the level with which they came.")
         }
      },
      empire_colosseum = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire training site: Colosseum, part 1
            pgettext(buildingctxt, "Trains soldiers in ‘Evade’."),
            -- TRANSLATORS: Purpose helptext for an empire training site: Colosseum, part 2
            pgettext(buildingctxt, "‘Evade’ increases the soldier’s chance not to be hit by the enemy and so to remain totally unaffected.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an empire training site: Colosseum
            pgettext(buildingctxt, "Imperial soldiers cannot be trained in ‘Defense’ and will remain at the level with which they came.")
         }
      },
      empire_trainingcamp = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire training site: Training Camp, part 1
            pgettext(buildingctxt, "Trains soldiers in ‘Attack’ and in ‘Health’."),
            -- TRANSLATORS: Purpose helptext for an empire training site: Training Camp, part 2
            pgettext(buildingctxt, "Equips the soldiers with all necessary weapons and armor parts.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an empire training site: Training Camp
            pgettext(buildingctxt, "Imperial soldiers cannot be trained in ‘Defense’ and will remain at the level with which they came.")
         }
      },
      empire_headquarters = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire warehouse: Headquarters
            pgettext(buildingctxt, "Accommodation for your people. Also stores your wares and tools.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an empire warehouse: Headquarters
            pgettext(buildingctxt, "The headquarters is your main building.")
         }
      },
      empire_headquarters_shipwreck = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire warehouse: Headquarters Shipwreck
            pgettext(buildingctxt, "Although this ship ran aground, it still serves as accommodation for your people. It also stores your wares and tools.")
         },
         note = {
            -- TRANSLATORS: Note helptext for an empire warehouse: Headquarters Shipwreck
            pgettext(buildingctxt, "The headquarters shipwreck is your main building.")
         }
      },
      empire_port = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire warehouse: Port
            pgettext(buildingctxt, "Serves as a base for overseas colonization and trade. Also stores your soldiers, wares and tools.")
         }
      },
      empire_warehouse = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for an empire warehouse: Warehouse
            pgettext("building", "Your workers and soldiers will find shelter here. Also stores your wares and tools.")
         }
      },
   },
   immovables = {
      ashes = {
         -- TRANSLATORS: Helptext for a tribe immovable: Ashes
         purpose = { _("The remains of a destroyed building.") }
      },
      destroyed_building = {
         -- TRANSLATORS: Helptext for a tribe immovable: Destroyed Building
         purpose = { _("The remains of a destroyed building.") }
      },
      grapevine_medium = {
         -- TRANSLATORS: Helptext for a tribe immovable: Grapevine
         purpose = { _("This grapevine is growing.") }
      },
      grapevine_ripe = {
         -- TRANSLATORS: Helptext for a tribe immovable: Grapevine
         purpose = { _("This grapevine is ready for harvesting.") }
      },
      grapevine_small = {
         -- TRANSLATORS: Helptext for a tribe immovable: Grapevine
         purpose = { _("This grapevine is growing.") }
      },
      grapevine_tiny = {
         -- TRANSLATORS: Helptext for a tribe immovable: Grapevine
         purpose = { _("This grapevine has just been planted.") }
      },
      empire_resi_coal_1 = {
         purpose = {
            -- TRANSLATORS: Helptext for a resource indicator: Coal
            _("Coal veins contain coal that can be dug up by coal mines."),
            -- TRANSLATORS: Helptext for a resource indicator: Coal
            _("There is only a little bit of coal here.")
         }
      },
      empire_resi_coal_2 = {
         purpose = {
            -- TRANSLATORS: Helptext for a resource indicator: Coal
            _("Coal veins contain coal that can be dug up by coal mines."),
            -- TRANSLATORS: Helptext for a resource indicator: Coal
            _("There is a lot of coal here.")
         }
      },
      empire_resi_gold_1 = {
         purpose = {
            -- TRANSLATORS: Helptext for a resource indicator: Gold
            _("Gold veins contain gold ore that can be dug up by gold mines."),
            -- TRANSLATORS: Helptext for a resource indicator: Gold
            _("There is only a little bit of gold here.")
         }
      },
      empire_resi_gold_2 = {
         purpose = {
            -- TRANSLATORS: Helptext for a resource indicator: Gold
            _("Gold veins contain gold ore that can be dug up by gold mines."),
            -- TRANSLATORS: Helptext for a resource indicator: Gold
            _("There is a lot of gold here.")
         }
      },
      empire_resi_iron_1 = {
         purpose = {
            -- TRANSLATORS: Helptext for a resource indicator: Iron
            _("Iron veins contain iron ore that can be dug up by iron mines."),
            -- TRANSLATORS: Helptext for a resource indicator: Iron
            _("There is only a little bit of iron here.")
         }
      },
      empire_resi_iron_2 = {
         purpose = {
            -- TRANSLATORS: Helptext for a resource indicator: Iron
            _("Iron veins contain iron ore that can be dug up by iron mines."),
            -- TRANSLATORS: Helptext for a resource indicator: Iron
            _("There is a lot of iron here.")
         }
      },
      empire_resi_none = {
         -- TRANSLATORS: Helptext for a resource indicator: No resources
         purpose = { _("There are no resources in the ground here.") }
      },
      empire_resi_water = {
         -- TRANSLATORS: Helptext for a resource indicator: Water
         purpose = { _("There is water in the ground here that can be pulled up by a well.") }
      },
      empire_resi_stones_1 = {
         purpose = {
            -- TRANSLATORS: Helptext for an Empire resource indicator: Stones
            _("Marble is a basic building material and can be dug up by a marble mine. You will also get granite from the mine."),
            -- TRANSLATORS: Helptext for an Empire resource indicator: Stones
            _("There is only a little bit of marble here.")
         }
      },
      empire_resi_stones_2 = {
         purpose = {
            -- TRANSLATORS: Helptext for an Empire resource indicator: Stones
            _("Marble is a basic building material and can be dug up by a marble mine. You will also get granite from the mine."),
            -- TRANSLATORS: Helptext for an Empire resource indicator: Stones
            _("There is a lot of marble here.")
         }
      },
      empire_shipconstruction = {
         -- TRANSLATORS: Helptext for a tribe immovable: Ship Under Construction
         purpose = { _("A ship is being constructed at this site.") }
      },
      wheatfield_harvested = {
         -- TRANSLATORS: Helptext for a tribe immovable: Wheat field
         purpose = { _("This field has been harvested.") }
      },
      wheatfield_medium = {
         -- TRANSLATORS: Helptext for a tribe immovable: Wheat field
         purpose = { _("This field is growing.") }
      },
      wheatfield_ripe = {
         -- TRANSLATORS: Helptext for a tribe immovable: Wheat field
         purpose = { _("This field is ready for harvesting.") }
      },
      wheatfield_small = {
         -- TRANSLATORS: Helptext for a tribe immovable: Wheat field
         purpose = { _("This field is growing.") }
      },
      wheatfield_tiny = {
         -- TRANSLATORS: Helptext for a tribe immovable: Wheat field
         purpose = { _("This field has just been planted.") }
      }
   },
   wares = {
      armor = {
         -- TRANSLATORS: Helptext for a ware: Armor
         purpose = { pgettext(warectxt, "Basic armor for Empire soldiers. It is produced in the armor smithy and used in the training camp – together with food – to train soldiers from health level 1 to level 2.") }
      },
      armor_chain = {
         -- TRANSLATORS: Helptext for a ware: Chain Armor
         purpose = { pgettext(warectxt, "The chain armor is a medium armor for Empire soldiers. It is produced in an armor smithy and used in a training camp – together with food – to train soldiers from health level 2 to level 3.") }
      },
      armor_gilded = {
         -- TRANSLATORS: Helptext for a ware: Gilded Armor
         purpose = { pgettext(warectxt, "The gilded armor is the strongest armor an Empire soldier can have. It is produced in the armor smithy and used in the training camp – together with food – to train soldiers from health level 3 to level 4.") }
      },
      armor_helmet = {
         -- TRANSLATORS: Helptext for a ware: Helmet
         purpose = { pgettext(warectxt, "The helmet is the basic defense of a warrior. It is produced in an armor smithy. In combination with a wooden spear, it is the equipment to fit out young soldiers. Helmets are also used in training camps – together with food – to train soldiers from health level 0 to level 1.") }
      },
      basket = {
         -- TRANSLATORS: Helptext for a ware: Basket
         purpose = { pgettext(warectxt, "This basket is needed by the vine farmer for harvesting the grapes. It is produced by the toolsmith.") }
      },
      beer = {
         -- TRANSLATORS: Helptext for a ware: Beer
         purpose = { pgettext(warectxt, "This beer is produced in a brewery out of wheat and water. It is consumed by miners in coal and iron mines.") }
      },
      bread_paddle = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Bread Paddle
            pgettext(warectxt, "The bread paddle is the tool of the baker, each baker needs one."),
            -- TRANSLATORS: Helptext for a ware: Bread Paddle
            pgettext(warectxt, "Bread paddles are produced by the toolsmith.")
         }
      },
      cloth = {
         -- TRANSLATORS: Helptext for a ware: Cloth
         purpose = { pgettext(warectxt, "Cloth is needed to build several buildings. It is also consumed in the armor smithy.") }
      },
      coal = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Coal
            pgettext(warectxt, "Coal is mined in coal mines or produced out of logs by a charcoal kiln."),
            -- TRANSLATORS: Helptext for a ware: Coal
            pgettext(warectxt, "The fires of the Empire smelting works, armor smithies and weapon smithies are usually fed with coal.")
         }
      },
      empire_bread = {
         -- TRANSLATORS: Helptext for a ware: Bread
         purpose = { pgettext(warectxt, "The bakers of the Empire make really tasty bread out of flour and water. It is used in taverns and inns to prepare rations and meals. Bread is also consumed at the training sites (arena, colosseum, training camp).") }
      },
      felling_ax = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Felling Ax
            pgettext(warectxt, "The felling ax is the tool to chop down trees."),
            -- TRANSLATORS: Helptext for a ware: Felling Ax
            pgettext(warectxt, "Felling axes are used by lumberjacks and produced by the toolsmithy.")
         }
      },
      fire_tongs = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Fire Tongs
            pgettext(warectxt, "Fire tongs are the tools for smelting ores."),
            -- TRANSLATORS: Helptext for a ware: Fire Tongs
            pgettext(warectxt, "They are used in the smelting works and produced by the toolsmith.")
         }
      },
      fish = {
         -- TRANSLATORS: Helptext for a ware: Fish
         purpose = { pgettext(warectxt, "Besides bread and meat, fish is also a foodstuff for the Empire. Fish are used in taverns, inns and training sites (arena, colosseum, training camp).") }
      },
      fishing_rod = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Fishing Rod
            pgettext(warectxt, "Fishing rods are needed by fishers to catch fish."),
            -- TRANSLATORS: Helptext for a ware: Fishing Rod
            pgettext(warectxt, "They are produced by the toolsmith.")
         }
      },
      flour = {
         -- TRANSLATORS: Helptext for a ware: Flour
         purpose = { pgettext(warectxt, "Flour is produced by the mill out of wheat and is needed in the bakery to produce the tasty Empire bread.") }
      },
      gold = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Gold
            pgettext(warectxt, "Gold is the most valuable of all metals, and it is smelted out of gold ore."),
            -- TRANSLATORS: Helptext for a ware: Gold
            pgettext(warectxt, "It is produced by the smelting works. Armor and weapons are embellished with gold in the armor smithy and the weapon smithy.")
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
      granite = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Granite
            pgettext(warectxt, "Granite is a basic building material."),
            -- TRANSLATORS: Helptext for a ware: Granite
            pgettext(warectxt, "The Empire produces granite blocks in quarries and marble mines.")
         }
      },
      grape = {
         -- TRANSLATORS: Helptext for a ware: Grape
         purpose = { pgettext(warectxt, "These grapes are the base for a tasty wine. They are harvested in a vineyard and processed in a winery.") }
      },
      hammer = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Hammer
            pgettext(warectxt, "The hammer is an essential tool."),
            -- TRANSLATORS: Helptext for a ware: Hammer
            pgettext(warectxt, "Geologists, builders, weaponsmiths and armorsmiths all need a hammer. Make sure you’ve always got some in reserve! They are produced by the toolsmith.")
         }
      },
      hunting_spear = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Hunting Spear
            pgettext(warectxt, "This spear is light enough to be thrown, but heavy enough to kill any animal in one blow. It is only used by hunters."),
            -- TRANSLATORS: Helptext for a ware: Hunting Spear
            pgettext(warectxt, "They are produced by the toolsmith.")
         }
      },
      iron = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Iron
            pgettext(warectxt, "Iron is smelted out of iron ores."),
            -- TRANSLATORS: Helptext for a ware: Iron
            pgettext(warectxt, "It is produced by the smelting works. Weapons, armor and tools are made of iron.")
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
      kitchen_tools = {
         -- TRANSLATORS: Helptext for a ware: Kitchen Tools
         purpose = { pgettext(warectxt, "Kitchen tools are needed for preparing rations and meals. They are produced in a toolsmithy and used in taverns and inns.") }
      },
      log = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Log
            pgettext(warectxt, "Logs are an important basic building material. They are produced by felling trees."),
            -- TRANSLATORS: Helptext for a ware: Log
            pgettext(warectxt, "Imperial lumberjacks fell the trees; foresters take care of the supply of trees. Logs are also used by the charcoal kiln, the toolsmithy and the sawmill.")
         }
      },
      marble = {
         -- TRANSLATORS: Helptext for a ware: Marble
         purpose = { pgettext(warectxt, "Marble is the beautiful stone which is cut out of the mountains or produced in a quarry. Marble is used as a building material and gets chiseled into marble columns in the stonemason’s house.") }
      },
      marble_column = {
         -- TRANSLATORS: Helptext for a ware: Marble Column
         purpose = { pgettext(warectxt, "Marble columns represent the high culture of the Empire, so they are needed for nearly every larger building. They are produced out of marble at a stonemason’s house.") }
      },
      meal = {
         -- TRANSLATORS: Helptext for a ware: Meal
         purpose = { pgettext(warectxt, "A real meal is made in inns out of bread and fish/meat. It satisfies the needs of miners in deep mines.") }
      },
      meat = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Meat
            pgettext(warectxt, "Meat contains a lot of energy, and it is obtained from wild game taken by hunters."),
            -- TRANSLATORS: Helptext for a ware: Meat
            pgettext(warectxt, "Meat can also be obtained as pork from piggeries. It is used in the inns and taverns to prepare lunch for the miners and is consumed at the training sites (arena, colosseum, training camp).")
         }
      },
      pick = {
         -- TRANSLATORS: Helptext for a ware: Pick
         purpose = { pgettext(warectxt, "Picks are used by stonemasons and miners. They are produced by the toolsmith.") }
      },
      planks = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Planks
            pgettext(warectxt, "Planks are an important building material."),
            -- TRANSLATORS: Helptext for a ware: Planks
            pgettext(warectxt, "They are produced out of logs by the sawmill."),
            -- TRANSLATORS: Helptext for a ware: Planks
            pgettext(warectxt, "They are also used by the weapon smithy.")
         }
      },
      ration = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Ration
            pgettext(warectxt, "A small bite to keep miners strong and working. The scout also consumes rations on his scouting trips."),
            -- TRANSLATORS: Helptext for a ware: Ration
            pgettext(warectxt, "Rations are produced in a tavern out of fish or meat or bread.")
         }
      },
      saw = {
         -- TRANSLATORS: Helptext for a ware: Saw
         purpose = { pgettext(warectxt, "This saw is needed by the carpenter. It is produced by the toolsmith.") }
      },
      scythe = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Scythe
            pgettext(warectxt, "The scythe is the tool of the farmers."),
            -- TRANSLATORS: Helptext for a ware: Scythe
            pgettext(warectxt, "Scythes are produced by the toolsmith.")
         }
      },
      shovel = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Shovel
            pgettext(warectxt, "Shovels are needed for the proper handling of plants."),
            -- TRANSLATORS: Helptext for a ware: Shovel
            pgettext(warectxt, "Therefore the forester and the vine farmer use them. They are produced by the toolsmith.")
         }
      },
      spear = {
         -- TRANSLATORS: Helptext for a ware: Spear
         purpose = { pgettext(warectxt, "This spear has an iron spike. It is produced in a weapon smithy and used in a training camp – together with food – to train soldiers from attack level 0 to attack level 1.") }
      },
      spear_advanced = {
         -- TRANSLATORS: Helptext for a ware: Advanced Spear
         purpose = { pgettext(warectxt, "This is an advanced spear with a steel tip. It is produced in a weapon smithy and used in a training camp – together with food – to train soldiers from attack level 1 to level 2.") }
      },
      spear_heavy = {
         -- TRANSLATORS: Helptext for a ware: Heavy Spear
         purpose = { pgettext(warectxt, "This is a strong spear with a steel-tip and a little blade. It is produced in the weapon smithy and used in the training camp – together with food – train soldiers from attack level 2 to level 3.") }
      },
      spear_war = {
         -- TRANSLATORS: Helptext for a ware: War Spear
         purpose = { pgettext(warectxt, "This is the best and sharpest weapon the Empire weaponsmith creates for the soldiers. It is used in the training camp – together with food – to train soldiers from attack level 3 to level 4.") }
      },
      spear_wooden = {
         -- TRANSLATORS: Helptext for a ware: Wooden Spear
         purpose = { pgettext(warectxt, "This wooden spear is the basic weapon in the Empire military system. It is produced in the weapon smithy. In combination with a helmet, it is the equipment to fit out young soldiers.") }
      },
      water = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Water
            pgettext(warectxt, "Water is the essence of life!"),
            -- TRANSLATORS: Helptext for a ware: Water
            pgettext(warectxt, "Water is used in the bakery and the brewery. The donkey farm, the sheep farm and the piggery also need to be supplied with water.")
         }
      },
      wheat = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Wheat
            pgettext(warectxt, "Wheat is essential for survival."),
            -- TRANSLATORS: Helptext for a ware: Wheat
            pgettext(warectxt, "Wheat is produced by farms and used by mills and breweries. Donkey farms, sheep farms and piggeries also need to be supplied with wheat.")
         }
      },
      wine = {
         -- TRANSLATORS: Helptext for a ware: Wine
         purpose = { pgettext(warectxt, "This tasty wine is drunk by the miners working the marble and gold mines. It is produced in a winery.") }
      },
      wool = {
         -- TRANSLATORS: Helptext for a ware: Wool
         purpose = { pgettext(warectxt, "Wool is the hair of sheep. Weaving mills use it to make cloth.") }
      }
   },
   workers = {
      empire_armorsmith = {
         -- TRANSLATORS: Helptext for an empire worker: Armorsmith
         purpose = { pgettext(workerctxt, "Produces armor for the soldiers.") }
      },
   }
}
pop_textdomain()
return result
