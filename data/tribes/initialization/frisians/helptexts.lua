-- Frisian ware helptexts

local buildingctxt = "frisians_building"
local immctxt = "frisians_immovable"
local warectxt = "frisians_ware"

return {
   buildings = {
      constructionsite = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: Construction Site
            pgettext(buildingctxt, "‘Don’t swear at the builder who is short of building materials.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: Construction Site
            pgettext(buildingctxt, "Proverb widely used for impossible tasks of any kind")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: Construction Site
            pgettext(buildingctxt, "A new building is being built at this construction site.")
         }
      },
      dismantlesite = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: Dismantle Site
            pgettext(buildingctxt, "‘New paths will appear when you are willing to tear down the old.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: Dismantle Site
            pgettext(buildingctxt, "Proverb")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: Dismantle Site
            pgettext(buildingctxt, "A building is being dismantled at this dismantle site, returning some of the resources that were used during this building’s construction to your tribe’s stores.")
         }
      },
   },
   immovables = {
      ashes = {
         -- TRANSLATORS: Helptext for a tribe immovable: Ashes
         pgettext(immctxt, "The remains of a destroyed building.")
      },
      barleyfield_harvested = {
         -- TRANSLATORS: Helptext for a tribe immovable: Barley Field
         pgettext(immctxt, "This field has been harvested.")
      },
      barleyfield_medium = {
         -- TRANSLATORS: Helptext for a tribe immovable: Barley Field
         pgettext(immctxt, "This field is growing.")
      },
      barleyfield_ripe = {
         -- TRANSLATORS: Helptext for a tribe immovable: Barley Field
         pgettext(immctxt, "This field is ready for harvesting.")
      },
      barleyfield_small = {
         -- TRANSLATORS: Helptext for a tribe immovable: Barley Field
         pgettext(immctxt, "This field is growing.")
      },
      barleyfield_tiny = {
         -- TRANSLATORS: Helptext for a tribe immovable: Barley Field
         pgettext(immctxt, "This field has just been planted.")
      },
      destroyed_building = {
         -- TRANSLATORS: Helptext for a tribe immovable: Destroyed Building
         pgettext(immctxt, "The remains of a destroyed building.")
      },
      reedfield_medium = {
         -- TRANSLATORS: Helptext for a tribe immovable: Reed Field
         pgettext(immctxt, "This reed field is growing.")
      },
      reedfield_ripe = {
         -- TRANSLATORS: Helptext for a tribe immovable: Reed Field
         pgettext(immctxt, "This reed field is ready for harvesting.")
      },
      reedfield_small = {
         -- TRANSLATORS: Helptext for a tribe immovable: Reed Field
         pgettext(immctxt, "This reed field is growing.")
      },
      reedfield_tiny = {
         -- TRANSLATORS: Helptext for a tribe immovable: Reed Field
         pgettext(immctxt, "This reed field has just been planted.")
      },
      frisians_resi_coal_1 = {
         -- TRANSLATORS: Helptext for a resource indicator: Coal
         pgettext(immctxt, "Coal veins contain coal that can be dug up by coal mines."),
         -- TRANSLATORS: Helptext for a resource indicator: Coal
         pgettext(immctxt, "There is only a little bit of coal here.")
      },
      frisians_resi_coal_2 = {
         -- TRANSLATORS: Helptext for a resource indicator: Coal
         pgettext(immctxt, "Coal veins contain coal that can be dug up by coal mines."),
         -- TRANSLATORS: Helptext for a resource indicator: Coal
         pgettext(immctxt, "There is a lot of coal here.")
      },
      frisians_resi_gold_1 = {
         -- TRANSLATORS: Helptext for a resource indicator: Gold
         pgettext(immctxt, "Gold veins contain gold ore that can be dug up by gold mines."),
         -- TRANSLATORS: Helptext for a resource indicator: Gold
         pgettext(immctxt, "There is only a little bit of gold here.")
      },
      frisians_resi_gold_2 = {
         -- TRANSLATORS: Helptext for a resource indicator: Gold
         pgettext(immctxt, "Gold veins contain gold ore that can be dug up by gold mines."),
         -- TRANSLATORS: Helptext for a resource indicator: Gold
         pgettext(immctxt, "There is a lot of gold here.")
      },
      frisians_resi_iron_1 = {
         -- TRANSLATORS: Helptext for a resource indicator: Iron
         pgettext(immctxt, "Iron veins contain iron ore that can be dug up by iron mines."),
         -- TRANSLATORS: Helptext for a resource indicator: Iron
         pgettext(immctxt, "There is only a little bit of iron here.")
      },
      frisians_resi_iron_2 = {
         -- TRANSLATORS: Helptext for a resource indicator: Iron
         pgettext(immctxt, "Iron veins contain iron ore that can be dug up by iron mines."),
         -- TRANSLATORS: Helptext for a resource indicator: Iron
         pgettext(immctxt, "There is a lot of iron here.")
      },
      frisians_resi_none = {
         -- TRANSLATORS: Helptext for a resource indicator: No resources
         pgettext(immctxt, "There are no resources in the ground here.")
      },
      frisians_resi_water = {
         -- TRANSLATORS: Helptext for a resource indicator: Water
         pgettext(immctxt, "There is water in the ground here that can be pulled up by a well.")
      },
      frisians_resi_stones_1 = {
         -- TRANSLATORS: Helptext for a Frisian resource indicator: Stones
         pgettext(immctxt, "Granite is a basic building material and can be dug up by a rock mine."),
         -- TRANSLATORS: Helptext for a Frisian resource indicator: Stones
         pgettext(immctxt, "There is only a little bit of granite here.")
      },
      frisians_resi_stones_2 = {
         -- TRANSLATORS: Helptext for a Frisian resource indicator: Stones
         pgettext(immctxt, "Granite is a basic building material and can be dug up by a rock mine."),
         -- TRANSLATORS: Helptext for a Frisian resource indicator: Stones
         pgettext(immctxt, "There is a lot of granite here.")
      },
      frisians_shipconstruction = {
         -- TRANSLATORS: Helptext for a tribe immovable: Ship Under Construction
         pgettext(immctxt, "A ship is being constructed at this site.")
      }
   },
   wares = {
      barley = {
         -- TRANSLATORS: Helptext for a ware: Barley
         pgettext(warectxt, "Barley is a slow-growing grain that is used for baking bread and brewing beer. It is also eaten by reindeer.")
      },
      basket = {
         -- TRANSLATORS: Helptext for a ware: Basket
         pgettext(warectxt, "Baskets are needed by the fruit collector to gather berries. They are woven from reed and wood by the blacksmith.")
      },
      beer = {
         -- TRANSLATORS: Helptext for a ware: Beer
         pgettext(warectxt, "Beer is produced in breweries and used in drinking halls to produce meals. Soldiers drink beer while receiving basic training.")
      },
      bread_frisians = {
         -- TRANSLATORS: Helptext for a ware: Bread
         pgettext(warectxt, "Bread is made out of barley and water and is used in the taverns to prepare rations. It is also consumed by training soldiers.")
      },
      bread_paddle = {
         -- TRANSLATORS: Helptext for a ware: Bread Paddle
         pgettext(warectxt, "The bread paddle is the tool of the baker, each baker needs one."),
         -- TRANSLATORS: Helptext for a ware: Bread Paddle
         pgettext(warectxt, "Bread paddles are produced by the blacksmithy.")
      },
      brick = {
         -- TRANSLATORS: Helptext for a ware: Brick
         pgettext(warectxt, "Bricks are the best and most important building material. They are made out of a mix of clay and granite dried in a coal fire.")
      },
      clay = {
         -- TRANSLATORS: Helptext for a ware: Clay
         pgettext(warectxt, "Clay is made out of water and mud to be turned into bricks, used in ship construction and to improve the charcoal kiln.")
      },
      cloth = {
         -- TRANSLATORS: Helptext for a ware: Cloth
         pgettext(warectxt, "Cloth is needed for ships. It is produced out of reindeer fur and reed.")
      },
      coal = {
         -- TRANSLATORS: Helptext for a ware: Coal
         pgettext(warectxt, "Coal is mined in coal mines or produced out of logs by a charcoal kiln."),
         -- TRANSLATORS: Helptext for a ware: Coal
         pgettext(warectxt, "The fires of the brick kilns, furnaces and armor smithies are fed with coal.")
      },
      felling_ax = {
         -- TRANSLATORS: Helptext for a ware: Felling Ax
         pgettext("ware", "The felling ax is the tool to chop down trees."),
         -- TRANSLATORS: Helptext for a ware: Felling Ax
         pgettext(warectxt, "Felling axes are used by woodcutters and produced by the blacksmithy.")
      },
      fire_tongs = {
         -- TRANSLATORS: Helptext for a ware: Fire Tongs
         pgettext(warectxt, "Fire tongs are the tools for smelting ores."),
         -- TRANSLATORS: Helptext for a ware: Fire Tongs
         pgettext(warectxt, "They are used in the furnace and the brick kiln and produced by the blacksmithy.")
      },
      fish = {
         -- TRANSLATORS: Helptext for a ware: Fish
         pgettext(warectxt, "Fish is a very important food resource for the Frisians. It is fished from the shore or reared in aqua farms." )
      },
      fishing_net = {
         -- TRANSLATORS: Helptext for a ware: Fishing Net
         pgettext(warectxt, "Fishing nets are the tool used by fishers." )
      },
      fruit = {
         -- TRANSLATORS: Helptext for a ware: Fruit
         pgettext(warectxt, "Fruit are berries gathered from berry bushes by a fruit collector. They are used for rations and for feeding the fish at the aqua farms.")
      },
      fur = {
         -- TRANSLATORS: Helptext for a ware: Fur
         pgettext(warectxt, "Fur is won from reindeer in a reindeer farm. It is woven into cloth or turned into fur garments for soldiers.")
      },
      fur_garment = {
         -- TRANSLATORS: Helptext for a ware: Fur Garment
         pgettext(warectxt, "Fur can be sewn into garments. They are used as basic armor. All new soldiers are clothed in a fur garment.")
      },
      fur_garment_golden = {
         -- TRANSLATORS: Helptext for a ware: Golden Fur Garment
         pgettext(warectxt, "Ordinary fur garments can be decorated with iron and gold. Such clothes are the best armor.")
      },
      fur_garment_old = {
         -- TRANSLATORS: Helptext for a ware: Old Fur Garment
         pgettext(warectxt, "Old garments can be turned into fur in a recycling center.")
      },
      fur_garment_studded = {
         -- TRANSLATORS: Helptext for a ware: Studded Fur Garment
         pgettext(warectxt, "Ordinary fur garments can be decorated with iron to give them a silvery shine. These clothes make good armor.")
      },
      gold = {
         -- TRANSLATORS: Helptext for a ware: Gold
         pgettext(warectxt, "Gold is the most valuable of all metals, and it is smelted out of gold ore."),
         -- TRANSLATORS: Helptext for a ware: Gold
         pgettext(warectxt, "Only very important things are embellished with gold. It is produced by the furnace and is used to produce better swords and the best helmets. The best armor is also decorated with gold.")
      },
      gold_ore = {
         -- TRANSLATORS: Helptext for a ware: Gold Ore
         pgettext(warectxt, "Gold ore is mined in a gold mine."),
         -- TRANSLATORS: Helptext for a ware: Gold Ore
         pgettext(warectxt, "Smelted in a furnace, it turns into gold which is used as a precious building material and to produce weapons and armor.")
      },
      granite = {
         -- TRANSLATORS: Helptext for a ware: Granite
         pgettext(warectxt, "Granite is a basic building material."),
         -- TRANSLATORS: Helptext for a ware: Granite
         pgettext(warectxt, "The Frisians produce granite blocks in quarries and rock mines. They can be refined in a brick kiln.")
      },
      hammer = {
      -- TRANSLATORS: Helptext for a ware: Hammer
         pgettext(warectxt, "The hammer is an essential tool."),
         -- TRANSLATORS: Helptext for a ware: Hammer
         pgettext(warectxt, "Geologists, builders and blacksmiths all need a hammer. Make sure you’ve always got some in reserve! They are produced by the blacksmithy.")
      },
      helmet = {
         -- TRANSLATORS: Helptext for a ware: Helmet
         pgettext(warectxt, "A helmet is a basic tool to protect soldiers. It is produced in the small armor smithy and used to train soldiers from health level 0 to level 1.")
      },
      helmet_golden = {
         -- TRANSLATORS: Helptext for a ware: Golden Helmet
         pgettext(warectxt, "A golden helmet protects soldiers. It is produced in the large armor smithy and used to train soldiers from health level 1 to level 2.")
      },
      honey = {
         -- TRANSLATORS: Helptext for a ware: Honey
         pgettext(warectxt, "Honey is produced by bees belonging to a beekeeper. It is used to bake honey bread and brew mead.")
      },
      honey_bread = {
         -- TRANSLATORS: Helptext for a ware: Honey Bread
         pgettext(warectxt, "This bread is sweetened with honey. It is consumed by the most experienced miners and in advanced soldier training.")
      },
      hunting_spear = {
         -- TRANSLATORS: Helptext for a ware: Hunting Spear
         pgettext(warectxt, "This spear is light enough to be thrown, but heavy enough to kill any animal in one blow. It is only used by hunters."),
         -- TRANSLATORS: Helptext for a ware: Hunting Spear
         pgettext(warectxt, "Hunting spears are produced by the blacksmithy")
      },
      iron = {
         -- TRANSLATORS: Helptext for a ware: Iron
         pgettext(warectxt, "Iron is smelted out of iron ores."),
         -- TRANSLATORS: Helptext for a ware: Iron
         pgettext(warectxt, "It is produced by the furnace. Tools and weapons are made of iron. It is also used as jewellery for fur garment armor to give it a silver shine.")
      },
      iron_ore = {
         -- TRANSLATORS: Helptext for a ware: Iron Ore
         pgettext(warectxt, "Iron ore is mined in iron mines."),
         -- TRANSLATORS: Helptext for a ware: Iron Ore
         pgettext(warectxt, "It is smelted in a furnace to retrieve the iron.")
      },
      kitchen_tools = {
         -- TRANSLATORS: Helptext for a ware: Kitchen Tools
         pgettext(warectxt, "Kitchen tools are needed for preparing rations and meals. The smoker also needs them.")
      },
      log = {
         -- TRANSLATORS: Helptext for a ware: Log
         pgettext(warectxt, "Logs are an important basic building material. They are produced by felling trees."),
         -- TRANSLATORS: Helptext for a ware: Log
         pgettext(warectxt, "Woodcutters fell the trees; foresters take care of the supply of trees. Logs are also used in the blacksmithy to build basic tools, and in the charcoal kiln for the production of coal. Smokeries use logs as fuel for smoking meat and fish.")
      },
      mead = {
         -- TRANSLATORS: Helptext for a ware: Mead
         pgettext(warectxt, "Mead is produced by mead breweries. Soldiers drink mead during advanced training.")
      },
      meal = {
         -- TRANSLATORS: Helptext for a ware: Meal
         pgettext(warectxt, "A meal is made out of honey bread and beer and either smoked fish or meat. It is consumed by miners in deep mines.")
      },
      meat = {
         -- TRANSLATORS: Helptext for a ware: Meat
         pgettext(warectxt, "Meat contains a lot of energy, and it is obtained from wild game taken by hunters."),
         -- TRANSLATORS: Helptext for a ware: Meat
         pgettext(warectxt, "Meat has to be smoked in a smokery before being delivered to taverns, drinking halls and training sites.")
      },
      needles = {
         -- TRANSLATORS: Helptext for a ware: Needles
         pgettext(warectxt, "Needles are used by seamstresses to sew cloth and fur garments.")
      },
      pick = {
         -- TRANSLATORS: Helptext for a ware: Pick
         pgettext(warectxt, "Picks are used by stonemasons and miners.")
      },
      ration = {
         -- TRANSLATORS: Helptext for a ware: Ration
         pgettext(warectxt, "A small bite to keep miners strong and working. The scout also consumes rations on his scouting trips."),
         -- TRANSLATORS: Helptext for a ware: Ration
         pgettext(warectxt, "Rations are produced in taverns and drinking halls out of something to eat: Fruit, bread or smoked meat or fish.")
      },
      reed = {
         -- TRANSLATORS: Helptext for a ware: Reed
         pgettext(warectxt, "Reed is grown in a reed farm. Nothing is better suited to make roofs waterproof. It is also used to make buckets and fishing nets as well as cloth." )
      },
      scrap_iron = {
         -- TRANSLATORS: Helptext for a ware: Scrap Iron
         pgettext(warectxt, "Discarded weapons and armor can be recycled in a recycling center to produce new tools, weapon and armor.")
      },
      scrap_metal_mixed = {
         -- TRANSLATORS: Helptext for a ware: Scrap metal (mixed)
         pgettext(warectxt, "Discarded weapons and armor can be recycled in a recycling center to produce new tools, weapon and armor.")
      },
      scythe = {
         -- TRANSLATORS: Helptext for a ware: Scythe
         pgettext(warectxt, "The scythe is the tool of the farmers."),
         -- TRANSLATORS: Helptext for a ware: Scythe
         pgettext(warectxt, "Scythes are produced by the blacksmithy.")
      },
      shovel = {
         -- TRANSLATORS: Helptext for a ware: Shovel
         pgettext(warectxt, "Shovels are needed for the proper handling of plants."),
         -- TRANSLATORS: Helptext for a ware: Shovel
         pgettext(warectxt, "They are used by berry and reed farmers as well as foresters. Clay diggers also need them to dig mud out of hard soil.")
      },
      smoked_fish = {
         -- TRANSLATORS: Helptext for a ware: Smoked Fish
         pgettext(warectxt, "Fish is smoked in a smokery. Smoked fish is then consumed by soldiers in training or turned into rations and meals for miners and scouts.")
      },
      smoked_meat = {
         -- TRANSLATORS: Helptext for a ware: Smoked Meat
         pgettext(warectxt, "Meat is smoked in a smokery. Smoked meat is then consumed by soldiers in training or turned into rations and meals for miners and scouts.")
      },
      sword_broad = {
         -- TRANSLATORS: Helptext for a ware: Broadsword
         pgettext(warectxt, "The broadsword is the weapon used by level 2 soldiers. Level 5 soldiers are equipped with a broadsword and a double-edged sword.")
      },
      sword_double = {
         -- TRANSLATORS: Helptext for a ware: Double-edged Sword
         pgettext(warectxt, "The double-edged sword is the weapon used by level 3 soldiers. Level 6 soldiers are equipped with two of these ferocious swords.")
      },
      sword_long = {
         -- TRANSLATORS: Helptext for a ware: Long Sword
         pgettext(warectxt, "The long sword is the weapon used by level 1 soldiers. Level 4 soldiers are equipped with a long and a double-edged sword.")
      },
      sword_short = {
         -- TRANSLATORS: Helptext for a ware: Short sword
         pgettext(warectxt, "This is the basic weapon of the Frisian soldiers. Together with a fur garment, it makes up the equipment of young soldiers. Short swords are produced by the small armor smithy.")
      },
      water = {
         -- TRANSLATORS: Helptext for a ware: Water
         pgettext(warectxt, "Water is the essence of life!"),
         -- TRANSLATORS: Helptext for a ware: Water
         pgettext(warectxt, "Water is used to bake bread and brew beer. Reindeer farms and aqua farms also consume it.")
      }
    }
}
