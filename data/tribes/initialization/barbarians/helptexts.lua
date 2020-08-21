-- Barbarian ware helptexts

local warectxt = "barbarians_ware"
local immctxt = "barbarians_immovable"

return {
   immovables = {
      ashes = {
         -- TRANSLATORS: Helptext for a tribe immovable: Ashes
         pgettext(immctxt, "The remains of a destroyed building.")
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
      barbarians_resi_coal_1 = {
         -- TRANSLATORS: Helptext for a resource indicator: Coal
         pgettext(immctxt, "Coal veins contain coal that can be dug up by coal mines."),
         -- TRANSLATORS: Helptext for a resource indicator: Coal
         pgettext(immctxt, "There is only a little bit of coal here.")
      },
      barbarians_resi_coal_2 = {
         -- TRANSLATORS: Helptext for a resource indicator: Coal
         pgettext(immctxt, "Coal veins contain coal that can be dug up by coal mines."),
         -- TRANSLATORS: Helptext for a resource indicator: Coal
         pgettext(immctxt, "There is a lot of coal here.")
      },
      barbarians_resi_gold_1 = {
         -- TRANSLATORS: Helptext for a resource indicator: Gold
         pgettext(immctxt, "Gold veins contain gold ore that can be dug up by gold mines."),
         -- TRANSLATORS: Helptext for a resource indicator: Gold
         pgettext(immctxt, "There is only a little bit of gold here.")
      },
      barbarians_resi_gold_2 = {
         -- TRANSLATORS: Helptext for a resource indicator: Gold
         pgettext(immctxt, "Gold veins contain gold ore that can be dug up by gold mines."),
         -- TRANSLATORS: Helptext for a resource indicator: Gold
         pgettext(immctxt, "There is a lot of gold here.")
      },
      barbarians_resi_iron_1 = {
         -- TRANSLATORS: Helptext for a resource indicator: Iron
         pgettext(immctxt, "Iron veins contain iron ore that can be dug up by iron mines."),
         -- TRANSLATORS: Helptext for a resource indicator: Iron
         pgettext(immctxt, "There is only a little bit of iron here.")
      },
      barbarians_resi_iron_2 = {
         -- TRANSLATORS: Helptext for a resource indicator: Iron
         pgettext(immctxt, "Iron veins contain iron ore that can be dug up by iron mines."),
         -- TRANSLATORS: Helptext for a resource indicator: Iron
         pgettext(immctxt, "There is a lot of iron here.")
      },
      barbarians_resi_none = {
         -- TRANSLATORS: Helptext for a resource indicator: No resources
         pgettext(immctxt, "There are no resources in the ground here.")
      },
      barbarians_resi_water = {
         -- TRANSLATORS: Helptext for a resource indicator: Water
         pgettext(immctxt, "There is water in the ground here that can be pulled up by a well.")
      },
      barbarians_resi_stones_1 = {
         -- TRANSLATORS: Helptext for a Barbarian resource indicator: Stones
         pgettext(immctxt, "Granite is a basic building material and can be dug up by a granite mine."),
         -- TRANSLATORS: Helptext for a Barbarian resource indicator: Stones
         pgettext(immctxt, "There is only a little bit of granite here."),
      },
      barbarians_resi_stones_2 = {
         -- TRANSLATORS: Helptext for a Barbarian resource indicator: Stones
         pgettext(immctxt, "Granite is a basic building material and can be dug up by a granite mine."),
         -- TRANSLATORS: Helptext for a Barbarian resource indicator: Stones
         pgettext(immctxt, "There is a lot of granite here.")
      },
      barbarians_shipconstruction = {
         -- TRANSLATORS: Helptext for a tribe immovable: Ship Under Construction
         pgettext(immctxt, "A ship is being constructed at this site.")
      },
      wheatfield_harvested = {
         -- TRANSLATORS: Helptext for a tribe immovable: Wheat field
         pgettext(immctxt, "This field has been harvested.")
      },
      wheatfield_medium = {
         -- TRANSLATORS: Helptext for a tribe immovable: Wheat field
         pgettext(immctxt, "This field is growing.")
      },
      wheatfield_ripe = {
         -- TRANSLATORS: Helptext for a tribe immovable: Wheat field
         pgettext(immctxt, "This field is ready for harvesting.")
      },
      wheatfield_small = {
         -- TRANSLATORS: Helptext for a tribe immovable: Wheat field
         pgettext(immctxt, "This field is growing.")
      },
      wheatfield_tiny = {
         -- TRANSLATORS: Helptext for a tribe immovable: Wheat field
         pgettext(immctxt, "This field has just been planted.")
      }
   },
   wares = {
      ax = {
         -- TRANSLATORS: Helptext for a ware: Ax
         pgettext(warectxt, "The ax is the basic weapon of the Barbarians. All young soldiers are equipped with it.")
      },
      ax_battle = {
         -- TRANSLATORS: Helptext for a ware: Battle Ax
         pgettext(warectxt, "This is a dangerous weapon the Barbarians are able to produce. It is produced in the war mill. Only trained soldiers are able to wield such a weapon. It is used – together with food – in the training camp to train soldiers from attack level 3 to 4.")
      },
      ax_broad = {
         -- TRANSLATORS: Helptext for a ware: Broad Ax
         pgettext(warectxt, "The broad ax is produced by the ax workshop and the war mill. It is used in the training camp – together with food – to train soldiers with a fundamental fighting knowledge from attack level 1 to attack level 2.")
      },
      ax_bronze = {
         -- TRANSLATORS: Helptext for a ware: Bronze Ax
         pgettext(warectxt, "The bronze ax is considered a weapon that is hard to handle. Only skilled soldiers can use it. It is produced at the war mill and used in the training camp – together with food – to train soldiers from attack level 2 to level 3.")
      },
      ax_sharp = {
         -- TRANSLATORS: Helptext for a ware: Sharp Ax
         pgettext(warectxt, "Young soldiers are proud to learn to fight with this powerful weapon. It is heavier and sharper than the ordinary ax. It is produced in ax workshops and war mills. In training camps, it is used – together with food – to train soldiers from attack level 0 to level 1.")
      },
      ax_warriors = {
         -- TRANSLATORS: Helptext for a ware: Warrior’s Ax
         pgettext(warectxt, "The warrior’s ax is the most dangerous of all Barbarian weapons. Only a few soldiers ever were able to handle this huge and powerful ax. It is produced in a war mill and used – together with food – in a training camp to train soldiers from attack level 4 to level 5.")
      },
      barbarians_bread = {
         -- TRANSLATORS: Helptext for a ware: Pitta Bread
         pgettext(warectxt, "The Barbarian bakers are best in making this flat and tasty pitta bread. It is made out of wheat and water following a secret recipe. Pitta bread is used in the taverns, inns and big inns to prepare rations, snacks and meals. It is also consumed at training sites (training camp and battle arena).")
      },
      beer = {
         -- TRANSLATORS: Helptext for a ware: Beer
         pgettext(warectxt, "Beer is produced in micro breweries and used in inns and big inns to produce snacks.")
      },
      beer_strong = {
         -- TRANSLATORS: Helptext for a ware: Strong Beer
         pgettext(warectxt, "Only this beer is acceptable for the soldiers in a battle arena. Some say that the whole power of the Barbarians lies in this ale. It helps to train the soldiers’ evade level from 0 to 1 to 2. Strong beer is also used in big inns to prepare meals.")
      },
      blackwood = {
         -- TRANSLATORS: Helptext for a ware: Blackwood
         pgettext(warectxt, "This fire-hardened wood is as hard as iron and it is used for several buildings. It is produced out of logs in the wood hardener.")
      },
      bread_paddle = {
         -- TRANSLATORS: Helptext for a ware: Bread Paddle
         pgettext(warectxt, "The bread paddle is the tool of the baker, each baker needs one."),
         -- TRANSLATORS: Helptext for a ware: Bread Paddle
         pgettext(warectxt, "Bread paddles are produced in the metal workshop like all other tools (but cease to be produced by the building if it is enhanced to an ax workshop and war mill).")
      },
      cloth = {
         -- TRANSLATORS: Helptext for a ware: Cloth
         pgettext(warectxt, "Cloth is needed for Barbarian ships. It is produced out of reed.")
      },
      coal = {
         -- TRANSLATORS: Helptext for a ware: Coal
         pgettext(warectxt, "Coal is mined in coal mines or produced out of logs by a charcoal kiln."),
         -- TRANSLATORS: Helptext for a ware: Coal
         pgettext(warectxt, "The fires of the Barbarians are usually fed with coal. Consumers are several buildings: lime kiln, smelting works, ax workshop, war mill, and helm smithy.")
      },
      felling_ax = {
         -- TRANSLATORS: Helptext for a ware: Felling Ax
         pgettext(warectxt, "The felling ax is the tool to chop down trees."),
         -- TRANSLATORS: Helptext for a ware: Felling Ax
         pgettext(warectxt, "Felling axes are used by lumberjacks and produced in the metal workshop (but cease to be produced by the building if it is enhanced to an ax workshop and war mill).")
      },
      fire_tongs = {
         -- TRANSLATORS: Helptext for a ware: Fire Tongs
         pgettext(warectxt, "Fire tongs are the tools for smelting ores."),
         -- TRANSLATORS: Helptext for a ware: Fire Tongs
         pgettext(warectxt, "They are used in the smelting works and produced by the metal workshop (but they cease to be produced by the building if it is enhanced to an ax workshop and war mill).")
      },
      fish = {
         -- TRANSLATORS: Helptext for a ware: Fish
         pgettext(warectxt, "Besides pitta bread and meat, fish is also a foodstuff for the Barbarians. It is used in the taverns, inns and big inns and at the training sites (training camp and battle arena).")
      },
      fishing_rod = {
         -- TRANSLATORS: Helptext for a ware: Fishing Rod
         pgettext(warectxt, "Fishing rods are needed by fishers to catch fish."),
         -- TRANSLATORS: Helptext for a ware: Fishing Rod
         pgettext(warectxt, "They are one of the basic tools produced in a metal workshop (but cease to be produced by the building if it is enhanced to an ax workshop and war mill).")
      },
      gold = {
         -- TRANSLATORS: Helptext for a ware: Gold
         pgettext(warectxt, "Gold is the most valuable of all metals, and it is smelted out of gold ore."),
         -- TRANSLATORS: Helptext for a ware: Gold
         pgettext(warectxt, "Only very important things are embellished with gold. It is produced by the smelting works and used as a precious building material and to produce different axes (in the war mill) and different parts of armor (in the helm smithy).")
      },
      gold_ore = {
         -- TRANSLATORS: Helptext for a ware: Gold Ore
         pgettext(warectxt, "Gold ore is mined in a gold mine."),
         -- TRANSLATORS: Helptext for a ware: Gold Ore
         pgettext(warectxt, "Smelted in a smelting works, it turns into gold which is used as a precious building material and to produce weapons and armor.")
      },
      granite = {
         -- TRANSLATORS: Helptext for a ware: Granite
         pgettext(warectxt, "Granite is a basic building material."),
         -- TRANSLATORS: Helptext for a ware: Granite
         pgettext(warectxt, "The Barbarians produce granite blocks in quarries and granite mines.")
      },
      grout = {
         -- TRANSLATORS: Helptext for a ware: Grout
         pgettext(warectxt, "Granite can be processed into grout which provides a solid, non burning building material. Grout is produced in a lime kiln.")
      },
      hammer = {
         -- TRANSLATORS: Helptext for a ware: Hammer
         pgettext(warectxt, "The hammer is an essential tool."),
         -- TRANSLATORS: Helptext for a ware: Hammer
         pgettext(warectxt, "Geologists, builders, blacksmiths and helmsmiths all need a hammer. Make sure you’ve always got some in reserve! They are one of the basic tools produced at the metal workshop (but cease to be produced by the building if it is enhanced to an ax workshop and war mill).")
      },
      helmet = {
         -- TRANSLATORS: Helptext for a ware: Helmet
         pgettext(warectxt, "A helmet is a basic tool to protect soldiers. It is produced in the helm smithy and used in the training camp – together with food – to train soldiers from health level 0 to level 1.")
      },
      helmet_mask = {
         -- TRANSLATORS: Helptext for a ware: Mask
         pgettext(warectxt, "A mask is an enhanced armor for Barbarian soldiers. It is produced in the helm smithy and used in the training camp – together with food – to train soldiers from health level 1 to level 2.")
      },
      helmet_warhelm = {
         -- TRANSLATORS: Helptext for a ware: Warhelm
         pgettext(warectxt, "This is the most enhanced Barbarian armor. It is produced in a helm smithy and used in a training camp – together with food – to train soldiers from health level 2 to level 3.")
      },
      hunting_spear = {
         -- TRANSLATORS: Helptext for a ware: Hunting Spear
         pgettext(warectxt, "This spear is light enough to be thrown, but heavy enough to kill any animal in one blow. It is only used by hunters."),
         -- TRANSLATORS: Helptext for a ware: Hunting Spear
         pgettext(warectxt, "Hunting spears are produced in the metal workshop (but cease to be produced by the building if it is enhanced to an ax workshop and war mill).")
      },
      iron = {
         -- TRANSLATORS: Helptext for a ware: Iron
         pgettext(warectxt, "Iron is smelted out of iron ores."),
         -- TRANSLATORS: Helptext for a ware: Iron
         pgettext(warectxt, "It is produced by the smelting works and used to produce weapons and tools in the metal workshop, ax workshop, war mill and helm smithy.")
      },
      iron_ore = {
         -- TRANSLATORS: Helptext for a ware: Iron Ore
         pgettext(warectxt, "Iron ore is mined in iron mines."),
         -- TRANSLATORS: Helptext for a ware: Iron Ore
         pgettext(warectxt, "It is smelted in a smelting works to retrieve the iron.")
      },
      kitchen_tools = {
         -- TRANSLATORS: Helptext for a ware: Kitchen Tools
         pgettext(warectxt, "Kitchen tools are needed for preparing rations, snacks and meals. Be sure to have a metal workshop to produce this basic tool (but it ceases to be produced by the building if it is enhanced to an ax workshop and war mill).")
      },
      log = {
         -- TRANSLATORS: Helptext for a ware: Log
         pgettext(warectxt, "Logs are an important basic building material. They are produced by felling trees."),
         -- TRANSLATORS: Helptext for a ware: Log
         pgettext(warectxt, "Barbarian lumberjacks fell the trees; rangers take care of the supply of trees. Logs are also used in the metal workshop to build basic tools, and in the charcoal kiln for the production of coal. The wood hardener refines logs into blackwood by hardening them with fire.")
      },
      meal = {
         -- TRANSLATORS: Helptext for a ware: Meal
         pgettext(warectxt, "A meal is made out of pitta bread, strong beer and fish/meat in a big inn. This substantial food is exactly what workers in a deeper mine need.")
      },
      meat = {
         -- TRANSLATORS: Helptext for a ware: Meat
         pgettext(warectxt, "Meat contains a lot of energy, and it is obtained from wild game taken by hunters."),
         -- TRANSLATORS: Helptext for a ware: Meat
         pgettext(warectxt, "Meat is used in the taverns, inns and big inns to prepare rations, snacks and meals for the miners. It is also consumed at the training sites (training camp and battle arena).")
      },
      pick = {
         -- TRANSLATORS: Helptext for a ware: Pick
         pgettext(warectxt, "Picks are used by stonemasons and miners. They are produced in the metal workshop (but cease to be produced by the building if it is enhanced to an ax workshop and war mill).")
      },
      ration = {
         -- TRANSLATORS: Helptext for a ware: Ration
         pgettext(warectxt, "A small bite to keep miners strong and working. The scout also consumes rations on his scouting trips."),
         -- TRANSLATORS: Helptext for a ware: Ration
         pgettext(warectxt, "Rations are produced in a tavern, an inn or a big inn out of fish or meat or pitta bread.")
      },
      reed = {
         -- TRANSLATORS: Helptext for a ware: Reed
         pgettext(warectxt, "Reed is produced in a reed yard and used to make the roofs of buildings waterproof.")
      },
      scythe = {
         -- TRANSLATORS: Helptext for a ware: Scythe
         pgettext(warectxt, "The scythe is the tool of the farmers."),
         -- TRANSLATORS: Helptext for a ware: Scythe
         pgettext(warectxt, "Scythes are produced by the metal workshop (but cease to be produced by the building if it is enhanced to an ax workshop and war mill).")
      },
      shovel = {
         -- TRANSLATORS: Helptext for a ware: Shovel
         pgettext(warectxt, "Shovels are needed for the proper handling of plants."),
         -- TRANSLATORS: Helptext for a ware: Shovel
         pgettext(warectxt, "Therefore the gardener and the ranger use them. Produced at the metal workshop (but cease to be produced by the building if it is enhanced to an ax workshop and war mill).")
      },
      snack = {
         -- TRANSLATORS: Helptext for a ware: Snack
         pgettext(warectxt, "A bigger morsel than the ration to provide miners in deep mines. It is produced in an inn or a big inn out of fish/meat, pitta bread and beer.")
      },
      water = {
         -- TRANSLATORS: Helptext for a ware: Water
         pgettext(warectxt, "Water is the essence of life!"),
         -- TRANSLATORS: Helptext for a ware: Water
         pgettext(warectxt, "Water is used in the bakery, the micro brewery and the brewery. The lime kiln and the cattle farm also need to be supplied with water.")
      },
      wheat = {
         -- TRANSLATORS: Helptext for a ware: Wheat
         pgettext(warectxt, "Wheat is essential for survival."),
         -- TRANSLATORS: Helptext for a ware: Wheat
         pgettext(warectxt, "Wheat is produced by farms and consumed by bakeries, micro breweries and breweries. Cattle farms also need to be supplied with wheat.")
      }
   }
}
