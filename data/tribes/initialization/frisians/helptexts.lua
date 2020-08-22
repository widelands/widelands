-- Frisian ware helptexts

-- For formatting time strings
include "tribes/scripting/help/global_helptexts.lua"

local buildingctxt = "frisians_building"
local warectxt = "frisians_ware"
local workerctxt = "frisians_worker"

push_textdomain("tribes_encyclopedia")

local result = {
   buildings = {
      constructionsite = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian building: Construction Site
            pgettext("building", "‘Don’t swear at the builder who is short of building materials.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian building: Construction Site
            pgettext("building", "Proverb widely used for impossible tasks of any kind")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian building: Construction Site
            pgettext("building", "A new building is being built at this construction site.")
         }
      },
      dismantlesite = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian building: Dismantle Site
            pgettext("building", "‘New paths will appear when you are willing to tear down the old.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian building: Dismantle Site
            pgettext("building", "Proverb")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian building: Dismantle Site
            pgettext("building", "A building is being dismantled at this dismantle site, returning some of the resources that were used during this building’s construction to your tribe’s stores.")
         }
      },
      frisians_fortress = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian military site: Fortress
            pgettext("building", "Garrisons soldiers to expand your territory.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian military site: Fortress
            pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      frisians_outpost = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian military site: Outpost
            pgettext("building", "Garrisons soldiers to expand your territory.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian military site: Outpost
            pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      frisians_sentinel = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian military site: Sentinel
            pgettext("building", "Garrisons soldiers to expand your territory.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian military site: Sentinel
            pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      frisians_tower = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian military site: Tower
            pgettext("building", "Garrisons soldiers to expand your territory.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian military site: Tower
            pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      frisians_wooden_tower = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian military site: Wooden Tower
            pgettext("building", "Garrisons soldiers to expand your territory.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian military site: Wooden Tower
            pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      frisians_wooden_tower_high = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian military site: High Wooden Tower
            pgettext("building", "Garrisons soldiers to expand your territory.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian military site: High Wooden Tower
            pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      frisians_aqua_farm = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Aqua Farm
            pgettext(buildingctxt, "Why on earth shouldn’t we be able to catch fish even in the desert?")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Aqua Farm
            pgettext(buildingctxt, "The fisherman who invented aqua farming")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Aqua Farm
            pgettext("building", "Breeds fish as food for soldiers and miners.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian production site: Aqua Farm
            pgettext("building", "The aqua farm needs holes in the ground that were dug by a clay pit’s worker nearby to use as fishing ponds.")
         }
      },
      frisians_armor_smithy_large = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Large Armor Smithy
            pgettext(buildingctxt, "Of course I could still forge short and long swords, but it is beneath my honor to bother with such basic equipment now.")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Large Armor Smithy
            pgettext(buildingctxt, "A master blacksmith refusing to forge anything but the most sophisticated helmets and weapons")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Large Armor Smithy
            pgettext("building", "Produces advanced weapons and golden helmets for the soldiers.")
         }
      },
      frisians_armor_smithy_small = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Small Armor Smithy
            pgettext(buildingctxt, "I don’t forge swords because soldiers need ’em, but soldiers get ’em because I forge ’em.")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Small Armor Smithy
            pgettext(buildingctxt, "A blacksmith pointing out his influence on soldier training")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Small Armor Smithy
            pgettext("building", "Produces basic weapons and helmets for the soldiers.")
         }
      },
      frisians_bakery = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Bakery
            pgettext(buildingctxt, "Why shouldn’t my bread taste good? It’s only barley and water!")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Bakery
            pgettext(buildingctxt, "A baker")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Bakery
            pgettext("building", "Bakes bread out of barley and water to feed miners and soldiers.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a frisian production site: Bakery
            pgettext(buildingctxt, "The bakery needs %s on average to produce one loaf of bread."):bformat(ngettext("%d second", "%d seconds", 40):bformat(40))
         }
      },
      frisians_barracks = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Barracks
            pgettext(buildingctxt, "You have thirty seconds to learn the basics of swordfighting and how to stay alive in battle. A third of that time has gone by for the introduction alone! You’d better pay close attention to me in order to make the most of it. Now here is your new short sword, forged just for you by our best blacksmiths. Time’s up everyone, now go occupy your sentinels!")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Barracks
            pgettext(buildingctxt, "A trainer greeting the new recruits")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Barracks
            pgettext(buildingctxt, "Equips recruits and trains them as soldiers.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a frisian production site: Barracks
            pgettext(buildingctxt, "The barracks needs %s on average to recruit one soldier."):bformat(ngettext("%d second", "%d seconds", 30):bformat(30))
         }
      },
      frisians_beekeepers_house = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Beekeeper's House
            pgettext(buildingctxt, "If my honey tastes bitter, I must have left some bee stings in it. There are never any bee stings in my honey, therefore, it is not bitter.")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Beekeeper's House
            pgettext(buildingctxt, "A beekeeper ignoring a customer’s complaint")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Beekeeper's House
            pgettext(buildingctxt, "Keeps bees and lets them swarm over flowering fields to produce honey.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian production site: Beekeeper's House
            pgettext(buildingctxt, "Needs medium-sized fields (barley, wheat, reed, corn or blackroot) or bushes (berry bushes or grapevines) nearby.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a frisian production site: Beekeeper's House
            pgettext(buildingctxt, "The beekeeper pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 45):bformat(45))
         }
      },
      frisians_berry_farm = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Berry Farm
            pgettext(buildingctxt, "My bushes may not be as tall as your trees, but I don’t know anybody who likes to eat bark!")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Berry Farm
            pgettext(buildingctxt, "A berry farmer to a forester")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Berry Farm
            pgettext("building", "Plants berry bushes in the surrounding area.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian production site: Berry Farm
            pgettext(buildingctxt, "The berry farm needs free space within the work area to plant the bushes.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a frisian production site: Berry Farm
            pgettext(buildingctxt, "The berry farmer pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 21):bformat(21))
         }
      },
      frisians_blacksmithy = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Blacksmithy
            pgettext(buildingctxt, "If you don’t have iron, change your name from blacksmith to lacksmith!")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Blacksmithy
            pgettext(buildingctxt, "Irritated chieftain during a metal shortage")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Blacksmithy
            pgettext("building", "Forges tools to equip new workers.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a frisian production site: Blacksmithy
            pgettext(buildingctxt, "The blacksmith needs %s on average to produce one tool."):bformat(ngettext("%d second", "%d seconds", 67):bformat(67))
         }
      },
      frisians_brewery = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Brewery
            pgettext(buildingctxt, "I know no single master miner who’ll ever work without a nice pint of beer!")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Brewery
            pgettext(buildingctxt, "A brewer boasting about the importance of his profession")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Brewery
            pgettext("building", "Brews beer for miners and soldier training.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a frisian production site: Brewery
            pgettext(buildingctxt, "The brewery needs %s on average to brew one mug of beer."):bformat(ngettext("%d second", "%d seconds", 60):bformat(60))
         }
      },
      frisians_brick_kiln = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Brick Kiln
            pgettext(buildingctxt, "If there is not enough coal, only the most foolish of leaders would deprive his brick kilns of it first.")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Brick Kiln
            pgettext(buildingctxt, "A brickmaker arguing with his chieftain who was doing just that")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Brick Kiln
            pgettext("building", "Burns bricks using granite and clay, and coal as fuel. Bricks are the most important building material.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a frisian production site: Brick Kiln
            pgettext(buildingctxt, "The brick kiln needs %s on average to produce three bricks."):bformat(ngettext("%d second", "%d seconds", 84):bformat(84))
         }
      },
      frisians_charcoal_burners_house = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Charcoal Burner's House
            pgettext(buildingctxt, "No other tribe has ever mastered the art of charcoal burning as we have!")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Charcoal Burner's House
            pgettext(buildingctxt, "The inventor of the Frisian charcoal kiln")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Charcoal Burner's House
            pgettext("building", "Burns logs into charcoal.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian production site: Charcoal Burner's House
            pgettext("building", "The charcoal burner's house needs holes in the ground that were dug by a clay pit’s worker nearby to erect charcoal stacks in them.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a frisian production site: Charcoal Burner's House
            pgettext(buildingctxt, "The charcoal burner's house needs %s on average to produce one coal."):bformat(ngettext("%d second", "%d seconds", 80):bformat(80))
         }
      },
      frisians_charcoal_kiln = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Charcoal Kiln
            pgettext(buildingctxt, "No other tribe has ever mastered the art of charcoal burning as we have!")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Charcoal Kiln
            pgettext(buildingctxt, "The inventor of the Frisian charcoal kiln")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Charcoal Kiln
            pgettext("building", "Burns logs into charcoal.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a frisian production site: Charcoal Kiln
            pgettext(buildingctxt, "The charcoal kiln needs %s on average to produce one coal."):bformat(ngettext("%d second", "%d seconds", 60):bformat(60))
         }
      },
      frisians_clay_pit = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Clay Pit
            pgettext(buildingctxt, "You think you can build a house without my help?")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Clay Pit
            pgettext(buildingctxt, "A clay digger arguing with a builder")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Clay Pit
            pgettext("building", "Digs up mud from the ground and uses water to turn it into clay. Clay is used to make bricks, reinforce the charcoal kiln and to build ships.")
         }
      },
      frisians_coalmine = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Coal Mine
            pgettext("building", "Digs coal out of the ground in mountain terrain.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian production site: Coal Mine
            pgettext(buildingctxt, "This mine exploits only %s of the resource. From there on out, it will only have a 5%% chance of finding any coal."):bformat("1/2")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a frisian production site: Coal Mine
            pgettext(buildingctxt, "If the food supply is steady, the coal mine can produce two pieces of coal in %s on average."):bformat(ngettext("%d second", "%d seconds", 85):bformat(85))
         }
      },
      frisians_coalmine_deep = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Deep Coal Mine
            pgettext("building", "Digs coal out of the ground in mountain terrain.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian production site: Deep Coal Mine
            pgettext(buildingctxt, "This mine exploits all of the resource down to the deepest level. But even after having done so, it will still have a %s chance of finding some more coal."):bformat("10%")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a frisian production site: Deep Coal Mine
            pgettext(buildingctxt, "If the food supply is steady, the deep coal mine can produce four pieces of coal in %s on average."):bformat(ngettext("%d second", "%d seconds", 76):bformat(76))
         }
      },
      frisians_collectors_house = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Fruit Collector's House
            pgettext(buildingctxt, "Meat doesn’t grow on bushes. Fruit does.")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Fruit Collector's House
            pgettext(buildingctxt, "A fruit collector advertising his harvest to a landlady")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Fruit Collector's House
            pgettext("building", "Collects berries from nearby bushes.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian production site: Fruit Collector's House
            pgettext(buildingctxt, "The fruit collector needs bushes full of berries within the work area.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a frisian production site: Fruit Collector's House
            pgettext(buildingctxt, "The fruit collector pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 21):bformat(21))
         }
      },
      frisians_drinking_hall = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Drinking Hall
            pgettext(buildingctxt, "All I need to be happy is a slice of honey bread with some smoked meat and a beer.")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Drinking Hall
            pgettext(buildingctxt, "A master miner to the landlady")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Drinking Hall
            pgettext(buildingctxt, "Prepares rations for scouts and rations and meals to feed the miners in all mines.")
         }
      },
      frisians_farm = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Farm
            pgettext(buildingctxt, "No frost, no heat; no rain, no drought; no rats, no locusts; naught can destroy my harvest.")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Farm
            pgettext(buildingctxt, "A farmer’s reply when asked by his chieftain why he was planting such a slow-growing grain.")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Farm
            pgettext("building", "Sows and harvests barley.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian production site: Farm
            pgettext(buildingctxt, "The farm needs free space within the work area to plant seeds.")
         }
      },
      frisians_ferry_yard = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Ferry Yard
            pgettext("building", "Builds ferries.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian production site: Ferry Yard
            pgettext("building", "Needs water nearby.")
         }
      },
      frisians_fishers_house = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Fisher's House
            pgettext(buildingctxt, "Hunters can’t sell anything on a Friday, but fishers don’t have such problems.")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Fisher's House
            pgettext(buildingctxt, "A hunter admiring a fisher")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Fisher's House
            pgettext(buildingctxt, "Fishes on the coast near the fisher’s house.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian production site: Fisher's House
            pgettext(buildingctxt, "The fisher’s house needs water full of fish within the work area.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a frisian production site: Fisher's House
            pgettext(buildingctxt, "The fisher pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 16):bformat(16))
         }
      },
      frisians_foresters_house = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Forester's House
            pgettext(buildingctxt, "What can ever be more beautiful than the brilliant sun’s beams shining through the glistering canopy of leaves?")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Forester's House
            pgettext(buildingctxt, "A forester explaining his choice of profession")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Forester's House
            pgettext("building", "Plants trees in the surrounding area.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian production site: Forester's House
            pgettext(buildingctxt, "The forester’s house needs free space within the work area to plant the trees.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a frisian production site: Forester's House
            pgettext(buildingctxt, "The forester pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 12):bformat(12))
         }
      },
      frisians_furnace = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Furnace
            pgettext(buildingctxt, "Miners get ores from the depths of the hills; but without our work, their labour is in vain.")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Furnace
            pgettext(buildingctxt, "Slogan of the Smelters’ Guild")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Furnace
            pgettext("building", "Smelts iron ore and gold ore into iron and gold ingots using coal.")
         }
      },
      frisians_goldmine = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Gold Mine
            pgettext("building", "Digs gold ore out of the ground in mountain terrain.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian production site: Gold Mine
            pgettext(buildingctxt, "This mine exploits only %s of the resource. From there on out, it will only have a 5%% chance of finding any gold ore."):bformat("1/2")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a frisian production site: Gold Mine
            pgettext(buildingctxt, "If the food supply is steady, the gold mine can produce one piece of gold ore in %s on average."):bformat(ngettext("%d second", "%d seconds", 65):bformat(65))
         }
      },
      frisians_goldmine_deep = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Deep Gold Mine
            pgettext("building", "Digs gold ore out of the ground in mountain terrain.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian production site: Deep Gold Mine
            pgettext(buildingctxt, "This mine exploits all of the resource down to the deepest level. But even after having done so, it will still have a %s chance of finding some more gold ore."):bformat("10%")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a frisian production site: Deep Gold Mine
            pgettext(buildingctxt, "If the food supply is steady, the deep gold mine can produce two pieces of gold ore in %s on average."):bformat(ngettext("%d second", "%d seconds", 76):bformat(76))
         }
      },
      frisians_honey_bread_bakery = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Honey Bread Bakery
            pgettext(buildingctxt, "Rookies will say that vengeance is sweet. Heroes will say that honey bread is sweeter.")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Honey Bread Bakery
            pgettext(buildingctxt, "A trainer in conversation with a baker")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Honey Bread Bakery
            pgettext("building", "Bakes honey bread out of barley, water and honey to feed miners in deep mines and soldiers in advanced training.")
         }
      },
      frisians_hunters_house = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Hunter's House
            pgettext("building", "Hunts animals to produce meat.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian production site: Hunter's House
            pgettext(buildingctxt, "The hunter’s house needs animals to hunt within the work area.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a frisian production site: Hunter's House
            pgettext(buildingctxt, "The hunter pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 35):bformat(35))
         }
      },
      frisians_ironmine = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Iron Mine
            pgettext("building", "Digs iron ore out of the ground in mountain terrain.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian production site: Iron Mine
            pgettext(buildingctxt, "This mine exploits only %s of the resource. From there on out, it will only have a 5%% chance of finding any iron ore."):bformat("1/2")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a frisian production site: Iron Mine
            pgettext(buildingctxt, "If the food supply is steady, the iron mine can produce one piece of iron ore in %s on average."):bformat(ngettext("%d second", "%d seconds", 65):bformat(65))
         }
      },
      frisians_ironmine_deep = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Deep Iron Mine
            pgettext("building", "Digs iron ore out of the ground in mountain terrain.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian production site: Deep Iron Mine
            pgettext(buildingctxt, "This mine exploits all of the resource down to the deepest level. But even after having done so, it will still have a %s chance of finding some more iron ore."):bformat("10%")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a frisian production site: Deep Iron Mine
            pgettext(buildingctxt, "If the food supply is steady, the deep iron mine can produce two pieces of iron ore in %s on average."):bformat(ngettext("%d second", "%d seconds", 76):bformat(76))
         }
      },
      frisians_mead_brewery = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Mead Brewery
            pgettext(buildingctxt, "If you like beer, you’ve never tasted mead.")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Mead Brewery
            pgettext(buildingctxt, "Slogan over a mead brewery")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Mead Brewery
            pgettext("building", "Brews beer out of barley and water. It also brews mead, which is beer refined with honey. Mead is consumed by experienced soldiers.")
         }
      },
      frisians_quarry = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Quarry
            pgettext(buildingctxt, "When I swing my pick, whole mountains fall before me!")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Quarry
            pgettext(buildingctxt, "A stonemason")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Quarry
            pgettext(buildingctxt, "Cuts raw pieces of granite out of rocks in the vicinity.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian production site: Quarry
            pgettext(buildingctxt, "The quarry needs rocks to cut within the work area.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a frisian production site: Quarry
            pgettext(buildingctxt, "The stonemason pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 25):bformat(25))
         }
      },
      frisians_recycling_center = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Recycling Center
            pgettext(buildingctxt, "Of course these weapons could be used by other soldiers again without being smelted down first! The only drawback is that they’d break in two at the first blow.")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Recycling Center
            pgettext(buildingctxt, "A smelter explaining the need for recycling to his impatient chieftain")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Recycling Center
            pgettext(buildingctxt, "Recycles old armor and weapon parts that have been discarded by training sites into fur, iron and gold.")
         }
      },
      frisians_reed_farm = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Reed Farm
            pgettext(buildingctxt, "No worse fortune can befall a reed farmer than to see his roof leaking.")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Reed Farm
            pgettext(buildingctxt, "Anonymous reed farmer")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Reed Farm
            pgettext(buildingctxt, "Cultivates reed that serves three different purposes for the Frisians.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian production site: Reed Farm
            pgettext(buildingctxt, "Reed is the traditional material for roofing. It is also needed for producing buckets and fishing nets, and it is woven – together with reindeer fur – into the cloth used for ships’ sails.")
         }
      },
      frisians_reindeer_farm = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Reindeer Farm
            pgettext(buildingctxt, "Who says a beast of burden cannot be useful for other things than transport?")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Reindeer Farm
            pgettext(buildingctxt, "The reindeer breeder who first proposed using reindeer fur for clothing")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Reindeer Farm
            pgettext(buildingctxt, "Breeds strong reindeer for adding them to the transportation system. Also keeps them for their fur, which is turned into armor and cloth.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian production site: Reindeer Farm
            pgettext(buildingctxt, "If the supply is steady, the reindeer farm produces one meat after producing three pieces of fur.")
         }
      },
      frisians_rockmine = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Rock Mine
            pgettext("building", "Digs granite out of the ground in mountain terrain.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian production site: Rock Mine
            pgettext(buildingctxt, "This mine exploits only %s of the resource. From there on out, it will only have a 5%% chance of finding any granite."):bformat("1/2")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a frisian production site: Rock Mine
            pgettext(buildingctxt, "If the food supply is steady, the rock mine can produce two blocks of granite in %s on average."):bformat(ngettext("%d second", "%d seconds", 85):bformat(85))
         }
      },
      frisians_rockmine_deep = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Deep Rock Mine
            pgettext("building", "Digs granite out of the ground in mountain terrain.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian production site: Deep Rock Mine
            pgettext(buildingctxt, "This mine exploits all of the resource down to the deepest level. But even after having done so, it will still have a %s chance of finding some more granite."):bformat("10%")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a frisian production site: Deep Rock Mine
            pgettext(buildingctxt, "If the food supply is steady, the deep rock mine can produce three blocks of granite in %s on average."):bformat(ngettext("%d second", "%d seconds", 76):bformat(76))
         }
      },
      frisians_scouts_house = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Scout's House
            pgettext(buildingctxt, "Everyone has their own ideas on how exactly we should explore the enemy’s territory… One more ‘improvement’ suggestion and we’ll demand meals instead of rations!")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Scout's House
            pgettext(buildingctxt, "The spokesman of the scouts’ labor union")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Scout's House
            pgettext("building", "Explores unknown territory.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a frisian production site: Scout's House
            pgettext(buildingctxt, "The scout pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 30):bformat(30))
         }
      },
      frisians_sewing_room = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Sewing Room
            pgettext(buildingctxt, "You soldiers think a good sword is everything, but where would you be if you had no garments?")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Sewing Room
            pgettext(buildingctxt, "A seamstress scolding a soldier for disrespecting her profession")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Sewing Room
            pgettext("building", "Sews fur garments out of reindeer fur.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a frisian production site: Sewing Room
            pgettext(buildingctxt, "The sewing room needs %s on average to produce one fur garment."):bformat(ngettext("%d second", "%d seconds", 45):bformat(45))
         }
      },
      frisians_shipyard = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Shipyard
            pgettext(buildingctxt, "This house may be called a shipyard, but my ships are rather longer than one yard!")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Shipyard
            pgettext(buildingctxt, "A shipwright who only constructed toy ships after being chid that his ships were too small")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Shipyard
            pgettext("building", "Constructs ships that are used for overseas colonization and for trading between ports.")
         }
      },
      frisians_smokery = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Smokery
            pgettext(buildingctxt, "Miners and soldiers are so picky… But who am I to complain, as I make my living from it?")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Smokery
            pgettext(buildingctxt, "A smoker explaining his profession")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Smokery
            pgettext("building", "Smokes fish and meat using logs. Only smoked meat and fish are good enough to be eaten by miners and soldiers.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a frisian production site: Smokery
            pgettext(buildingctxt, "The smokery needs %s on average to smoke two fish or two meat."):bformat(ngettext("%d second", "%d seconds", 46):bformat(46))
         }
      },
      frisians_tailors_shop = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Tailor's Shop
            pgettext(buildingctxt, "Don’t complain if these garments are too heavy – they’re not supposed to be light but to keep you alive a bit longer!")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Tailor's Shop
            pgettext(buildingctxt, "A trainer scolding a soldier")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Tailor's Shop
            pgettext("building", "Equips fur garments with iron or gold to produce good armor.")
         }
      },
      frisians_tavern = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Tavern
            pgettext(buildingctxt, "Nothing but fruit all day… Couldn’t you hurry up a bit?")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Tavern
            pgettext(buildingctxt, "Hungry customers in times of a shortage of smoked fish and meat")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Tavern
            pgettext("building", "Prepares rations to feed the scouts and miners.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a frisian production site: Tavern
            pgettext(buildingctxt, "The tavern can produce one ration in %s on average if the supply is steady; otherwise, it will take 50%% longer."):bformat(ngettext("%d second", "%d seconds", 33):bformat(33))
         }
      },
      frisians_weaving_mill = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Weaving Mill
            pgettext(buildingctxt, "Reindeer’s fur and roofing reed<br>These items two make up the seed<br>For ships of wood to chain the gales<br>In sturdy, beautious, blowing sails!")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Weaving Mill
            pgettext(buildingctxt, "A seamstress’ work song")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Weaving Mill
            pgettext("building", "Sews cloth for ship sails out of reed and reindeer fur.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a frisian production site: Weaving Mill
            pgettext(buildingctxt, "The weaving mill needs %s on average to produce one piece of cloth."):bformat(ngettext("%d second", "%d seconds", 45):bformat(45))
         }
      },
      frisians_well = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Well
            pgettext(buildingctxt, "We love the sea so much that we don’t want to drink it empty!")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Well
            pgettext(buildingctxt, "Chieftain Arldor’s retort when he was asked why his tribe can’t drink salt water")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Well
            pgettext("building", "Draws water out of the deep.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a frisian production site: Well
            pgettext(buildingctxt, "The well needs %s on average to produce one bucket of water."):bformat(ngettext("%d second", "%d seconds", 40):bformat(40))
         }
      },
      frisians_woodcutters_house = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian production site: Woodcutter's House
            pgettext(buildingctxt, "I cannot see a tree without imagining what it would look like in terms of furniture.")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian production site: Woodcutter's House
            pgettext(buildingctxt, "An over-enthusiastic woodcutter")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian production site: Woodcutter's House
            pgettext("building", "Fells trees in the surrounding area and processes them into logs.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian production site: Woodcutter's House
            pgettext(buildingctxt, "The woodcutter’s house needs trees to fell within the work area.")
         }
      },
      frisians_training_camp = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian training site: Training Camp
            pgettext(buildingctxt, "Just be quiet, listen carefully, and do try not to stab yourself until I’ve explained to you how to hold a broadsword.")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian training site: Training Camp
            pgettext(buildingctxt, "A trainer training a soldier")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian training site: Training Camp
            pgettext(buildingctxt, "Trains soldiers in Attack up to level 3 as well as in Defense and Health to level 1. Equips the soldiers with all necessary weapons and armor parts.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian training site: Training Camp
            pgettext(buildingctxt, "Frisian soldiers cannot train in Evade and will remain at their initial level.")
         }
      },
      frisians_training_arena = {
         lore = {
            -- TRANSLATORS: Lore helptext for a frisian training site: Training Arena
            pgettext(buildingctxt, "Now that you have two swords, there’s more of a risk you’ll accidentally stab yourself, but if you got this far, you’ll likely master this challenge as well.")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a frisian training site: Training Arena
            pgettext(buildingctxt, "A trainer training a soldier")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian training site: Training Arena
            pgettext(buildingctxt, "Trains soldiers in Attack, Defense and Health to the final level. Equips the soldiers with all necessary weapons and armor parts.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian training site: Training Arena
            pgettext(buildingctxt, "Trains only soldiers who have been trained to the maximum level by the Training Camp.")
         }
      },
      frisians_headquarters = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian warehouse: Headquarters
            pgettext(buildingctxt, "Accommodation for your people. Also stores your wares and tools.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a frisian warehouse: Headquarters
            pgettext(buildingctxt, "The headquarters is your main building.")
         }
      },
      frisians_port = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian warehouse: Port
            pgettext(buildingctxt, "Serves as a base for overseas colonization and trade. Also stores your soldiers, wares and tools.")
         }
      },
      frisians_warehouse = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for a frisian warehouse: Warehouse
            pgettext("building", "Your workers and soldiers will find shelter here. Also stores your wares and tools.")
         }
      },
   },
   immovables = {
      ashes = {
         -- TRANSLATORS: Helptext for a tribe immovable: Ashes
         purpose = { _("The remains of a destroyed building.") }
      },
      barleyfield_harvested = {
         -- TRANSLATORS: Helptext for a tribe immovable: Barley Field
         purpose = { _("This field has been harvested.") }
      },
      barleyfield_medium = {
         -- TRANSLATORS: Helptext for a tribe immovable: Barley Field
         purpose = { _("This field is growing.") }
      },
      barleyfield_ripe = {
         -- TRANSLATORS: Helptext for a tribe immovable: Barley Field
         purpose = { _("This field is ready for harvesting.") }
      },
      barleyfield_small = {
         -- TRANSLATORS: Helptext for a tribe immovable: Barley Field
         purpose = { _("This field is growing.") }
      },
      barleyfield_tiny = {
         -- TRANSLATORS: Helptext for a tribe immovable: Barley Field
         purpose = { _("This field has just been planted.") }
      },
      blackrootfield_medium = {
         -- TRANSLATORS: Helptext for a tribe immovable: Blackroot Field
         purpose = { _("This field is growing.") }
      },
      cornfield_harvested = {
         -- TRANSLATORS: Helptext for a tribe immovable: Corn Field
         purpose = { _("This field has been harvested.") }
      },
      cornfield_medium = {
         -- TRANSLATORS: Helptext for a tribe immovable: Corn Field
         purpose = { _("This field is growing.") }
      },
      cornfield_ripe = {
         -- TRANSLATORS: Helptext for a tribe immovable: Corn Field
         purpose = { _("This field is ready for harvesting.") }
      },
      cornfield_small = {
         -- TRANSLATORS: Helptext for a tribe immovable: Corn Field
         purpose = { _("This field is growing.") }
      },
      cornfield_tiny = {
         -- TRANSLATORS: Helptext for a tribe immovable: Corn Field
         purpose = { _("This field has just been planted.") }
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
      reedfield_medium = {
         -- TRANSLATORS: Helptext for a tribe immovable: Reed Field
         purpose = { _("This reed field is growing.") }
      },
      reedfield_ripe = {
         -- TRANSLATORS: Helptext for a tribe immovable: Reed Field
         purpose = { _("This reed field is ready for harvesting.") }
      },
      reedfield_small = {
         -- TRANSLATORS: Helptext for a tribe immovable: Reed Field
         purpose = { _("This reed field is growing.") }
      },
      reedfield_tiny = {
         -- TRANSLATORS: Helptext for a tribe immovable: Reed Field
         purpose = { _("This reed field has just been planted.") }
      },
      frisians_resi_coal_1 = {
         purpose = {
            -- TRANSLATORS: Helptext for a resource indicator: Coal
            _("Coal veins contain coal that can be dug up by coal mines."),
            -- TRANSLATORS: Helptext for a resource indicator: Coal
            _("There is only a little bit of coal here.")
         }
      },
      frisians_resi_coal_2 = {
         purpose = {
            -- TRANSLATORS: Helptext for a resource indicator: Coal
            _("Coal veins contain coal that can be dug up by coal mines."),
            -- TRANSLATORS: Helptext for a resource indicator: Coal
            _("There is a lot of coal here.")
         }
      },
      frisians_resi_gold_1 = {
         purpose = {
            -- TRANSLATORS: Helptext for a resource indicator: Gold
            _("Gold veins contain gold ore that can be dug up by gold mines."),
            -- TRANSLATORS: Helptext for a resource indicator: Gold
            _("There is only a little bit of gold here.")
         }
      },
      frisians_resi_gold_2 = {
         purpose = {
            -- TRANSLATORS: Helptext for a resource indicator: Gold
            _("Gold veins contain gold ore that can be dug up by gold mines."),
            -- TRANSLATORS: Helptext for a resource indicator: Gold
            _("There is a lot of gold here.")
         }
      },
      frisians_resi_iron_1 = {
         purpose = {
            -- TRANSLATORS: Helptext for a resource indicator: Iron
            _("Iron veins contain iron ore that can be dug up by iron mines."),
            -- TRANSLATORS: Helptext for a resource indicator: Iron
            _("There is only a little bit of iron here.")
         }
      },
      frisians_resi_iron_2 = {
         purpose = {
            -- TRANSLATORS: Helptext for a resource indicator: Iron
            _("Iron veins contain iron ore that can be dug up by iron mines."),
            -- TRANSLATORS: Helptext for a resource indicator: Iron
            _("There is a lot of iron here.")
         }
      },
      frisians_resi_none = {
         -- TRANSLATORS: Helptext for a resource indicator: No resources
         purpose = { _("There are no resources in the ground here.") }
      },
      frisians_resi_water = {
         -- TRANSLATORS: Helptext for a resource indicator: Water
         purpose = { _("There is water in the ground here that can be pulled up by a well.") }
      },
      frisians_resi_stones_1 = {
         purpose = {
            -- TRANSLATORS: Helptext for a Frisian resource indicator: Stones
            _("Granite is a basic building material and can be dug up by a rock mine."),
            -- TRANSLATORS: Helptext for a Frisian resource indicator: Stones
            _("There is only a little bit of granite here.")
         }
      },
      frisians_resi_stones_2 = {
         purpose = {
            -- TRANSLATORS: Helptext for a Frisian resource indicator: Stones
            _("Granite is a basic building material and can be dug up by a rock mine."),
            -- TRANSLATORS: Helptext for a Frisian resource indicator: Stones
            _("There is a lot of granite here.")
         }
      },
      frisians_shipconstruction = {
         -- TRANSLATORS: Helptext for a tribe immovable: Ship Under Construction
         purpose = { _("A ship is being constructed at this site.") }
      },
      wheatfield_medium = {
         -- TRANSLATORS: Helptext for a tribe immovable: Wheat field
         purpose = { _("This field is growing.") }
      },
   },
   wares = {
      barley = {
         -- TRANSLATORS: Helptext for a ware: Barley
         purpose = { pgettext(warectxt, "Barley is a slow-growing grain that is used for baking bread and brewing beer. It is also eaten by reindeer.") }
      },
      basket = {
         -- TRANSLATORS: Helptext for a ware: Basket
         purpose = { pgettext(warectxt, "Baskets are needed by the fruit collector to gather berries. They are woven from reed and wood by the blacksmith.") }
      },
      beer = {
         -- TRANSLATORS: Helptext for a ware: Beer
         purpose = { pgettext(warectxt, "Beer is produced in breweries and used in drinking halls to produce meals. Soldiers drink beer while receiving basic training.") }
      },
      bread_frisians = {
         -- TRANSLATORS: Helptext for a ware: Bread
         purpose = { pgettext(warectxt, "Bread is made out of barley and water and is used in the taverns to prepare rations. It is also consumed by training soldiers.") }
      },
      bread_paddle = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Bread Paddle
            pgettext("ware", "The bread paddle is the tool of the baker, each baker needs one."),
            -- TRANSLATORS: Helptext for a ware: Bread Paddle
            pgettext(warectxt, "Bread paddles are produced by the blacksmithy.")
         }
      },
      brick = {
         -- TRANSLATORS: Helptext for a ware: Brick
         purpose = { pgettext(warectxt, "Bricks are the best and most important building material. They are made out of a mix of clay and granite dried in a coal fire.") }
      },
      clay = {
         -- TRANSLATORS: Helptext for a ware: Clay
         purpose = { pgettext(warectxt, "Clay is made out of water and mud to be turned into bricks, used in ship construction and to improve the charcoal kiln.") }
      },
      cloth = {
         -- TRANSLATORS: Helptext for a ware: Cloth
         purpose = { pgettext(warectxt, "Cloth is needed for ships. It is produced out of reindeer fur and reed.") }
      },
      coal = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Coal
            pgettext("ware", "Coal is mined in coal mines or produced out of logs by a charcoal kiln."),
            -- TRANSLATORS: Helptext for a ware: Coal
            pgettext(warectxt, "The fires of the brick kilns, furnaces and armor smithies are fed with coal.")
         }
      },
      felling_ax = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Felling Ax
            pgettext("ware", "The felling ax is the tool to chop down trees."),
            -- TRANSLATORS: Helptext for a ware: Felling Ax
            pgettext(warectxt, "Felling axes are used by woodcutters and produced by the blacksmithy.")
         }
      },
      fire_tongs = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Fire Tongs
            pgettext("ware", "Fire tongs are the tools for smelting ores."),
            -- TRANSLATORS: Helptext for a ware: Fire Tongs
            pgettext(warectxt, "They are used in the furnace and the brick kiln and produced by the blacksmithy.")
         }
      },
      fish = {
         -- TRANSLATORS: Helptext for a ware: Fish
         purpose = { pgettext(warectxt, "Fish is a very important food resource for the Frisians. It is fished from the shore or reared in aqua farms." ) }
      },
      fishing_net = {
         -- TRANSLATORS: Helptext for a ware: Fishing Net
         purpose = { pgettext(warectxt, "Fishing nets are the tool used by fishers." ) }
      },
      fruit = {
         -- TRANSLATORS: Helptext for a ware: Fruit
         purpose = { pgettext(warectxt, "Fruit are berries gathered from berry bushes by a fruit collector. They are used for rations and for feeding the fish at the aqua farms.") }
      },
      fur = {
         -- TRANSLATORS: Helptext for a ware: Fur
         purpose = { pgettext(warectxt, "Fur is won from reindeer in a reindeer farm. It is woven into cloth or turned into fur garments for soldiers.") }
      },
      fur_garment = {
         -- TRANSLATORS: Helptext for a ware: Fur Garment
         purpose = { pgettext(warectxt, "Fur can be sewn into garments. They are used as basic armor. All new soldiers are clothed in a fur garment.") }
      },
      fur_garment_golden = {
         -- TRANSLATORS: Helptext for a ware: Golden Fur Garment
         purpose = { pgettext(warectxt, "Ordinary fur garments can be decorated with iron and gold. Such clothes are the best armor.") }
      },
      fur_garment_old = {
         -- TRANSLATORS: Helptext for a ware: Old Fur Garment
         purpose = { pgettext(warectxt, "Old garments can be turned into fur in a recycling center.") }
      },
      fur_garment_studded = {
         -- TRANSLATORS: Helptext for a ware: Studded Fur Garment
         purpose = { pgettext(warectxt, "Ordinary fur garments can be decorated with iron to give them a silvery shine. These clothes make good armor.") }
      },
      gold = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Gold
            pgettext("ware", "Gold is the most valuable of all metals, and it is smelted out of gold ore."),
            -- TRANSLATORS: Helptext for a ware: Gold
            pgettext(warectxt, "Only very important things are embellished with gold. It is produced by the furnace and is used to produce better swords and the best helmets. The best armor is also decorated with gold.")
         }
      },
      gold_ore = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Gold Ore
            pgettext("ware", "Gold ore is mined in a gold mine."),
            -- TRANSLATORS: Helptext for a ware: Gold Ore
            pgettext(warectxt, "Smelted in a furnace, it turns into gold which is used as a precious building material and to produce weapons and armor.")
         }
      },
      granite = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Granite
            pgettext("ware", "Granite is a basic building material."),
            -- TRANSLATORS: Helptext for a ware: Granite
            pgettext(warectxt, "The Frisians produce granite blocks in quarries and rock mines. They can be refined in a brick kiln.")
         }
      },
      hammer = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Hammer
            pgettext("ware", "The hammer is an essential tool."),
            -- TRANSLATORS: Helptext for a ware: Hammer
            pgettext(warectxt, "Geologists, builders and blacksmiths all need a hammer. Make sure you’ve always got some in reserve! They are produced by the blacksmithy.")
         }
      },
      helmet = {
         -- TRANSLATORS: Helptext for a ware: Helmet
         purpose = { pgettext(warectxt, "A helmet is a basic tool to protect soldiers. It is produced in the small armor smithy and used to train soldiers from health level 0 to level 1.") }
      },
      helmet_golden = {
         -- TRANSLATORS: Helptext for a ware: Golden Helmet
         purpose = { pgettext(warectxt, "A golden helmet protects soldiers. It is produced in the large armor smithy and used to train soldiers from health level 1 to level 2.") }
      },
      honey = {
         -- TRANSLATORS: Helptext for a ware: Honey
         purpose = { pgettext(warectxt, "Honey is produced by bees belonging to a beekeeper. It is used to bake honey bread and brew mead.") }
      },
      honey_bread = {
         -- TRANSLATORS: Helptext for a ware: Honey Bread
         purpose = { pgettext(warectxt, "This bread is sweetened with honey. It is consumed by the most experienced miners and in advanced soldier training.") }
      },
      hunting_spear = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Hunting Spear
            pgettext("ware", "This spear is light enough to be thrown, but heavy enough to kill any animal in one blow. It is only used by hunters."),
            -- TRANSLATORS: Helptext for a ware: Hunting Spear
            pgettext(warectxt, "Hunting spears are produced by the blacksmithy")
         }
      },
      iron = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Iron
            pgettext("ware", "Iron is smelted out of iron ores."),
            -- TRANSLATORS: Helptext for a ware: Iron
            pgettext(warectxt, "It is produced by the furnace. Tools and weapons are made of iron. It is also used as jewellery for fur garment armor to give it a silver shine.")
         }
      },
      iron_ore = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Iron Ore
            pgettext("default_ware", "Iron ore is mined in iron mines."),
            -- TRANSLATORS: Helptext for a ware: Iron Ore
            pgettext(warectxt, "It is smelted in a furnace to retrieve the iron.")
         }
      },
      kitchen_tools = {
         -- TRANSLATORS: Helptext for a ware: Kitchen Tools
         purpose = { pgettext(warectxt, "Kitchen tools are needed for preparing rations and meals. The smoker also needs them.") }
      },
      log = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Log
            pgettext("ware", "Logs are an important basic building material. They are produced by felling trees."),
            -- TRANSLATORS: Helptext for a ware: Log
            pgettext(warectxt, "Woodcutters fell the trees; foresters take care of the supply of trees. Logs are also used in the blacksmithy to build basic tools, and in the charcoal kiln for the production of coal. Smokeries use logs as fuel for smoking meat and fish.")
         }
      },
      mead = {
         -- TRANSLATORS: Helptext for a ware: Mead
         purpose = { pgettext(warectxt, "Mead is produced by mead breweries. Soldiers drink mead during advanced training.") }
      },
      meal = {
         -- TRANSLATORS: Helptext for a ware: Meal
         purpose = { pgettext(warectxt, "A meal is made out of honey bread and beer and either smoked fish or meat. It is consumed by miners in deep mines.") }
      },
      meat = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Meat
            pgettext("ware", "Meat contains a lot of energy, and it is obtained from wild game taken by hunters."),
            -- TRANSLATORS: Helptext for a ware: Meat
            pgettext(warectxt, "Meat has to be smoked in a smokery before being delivered to taverns, drinking halls and training sites.")
         }
      },
      needles = {
         -- TRANSLATORS: Helptext for a ware: Needles
         purpose = { pgettext(warectxt, "Needles are used by seamstresses to sew cloth and fur garments.") }
      },
      pick = {
         -- TRANSLATORS: Helptext for a ware: Pick
         purpose = { pgettext(warectxt, "Picks are used by stonemasons and miners.") }
      },
      ration = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Ration
            pgettext("ware", "A small bite to keep miners strong and working. The scout also consumes rations on his scouting trips."),
            -- TRANSLATORS: Helptext for a ware: Ration
            pgettext(warectxt, "Rations are produced in taverns and drinking halls out of something to eat: Fruit, bread or smoked meat or fish.")
         }
      },
      reed = {
         -- TRANSLATORS: Helptext for a ware: Reed
         purpose = { pgettext(warectxt, "Reed is grown in a reed farm. Nothing is better suited to make roofs waterproof. It is also used to make buckets and fishing nets as well as cloth." ) }
      },
      scrap_iron = {
         -- TRANSLATORS: Helptext for a ware: Scrap Iron
         purpose = { pgettext(warectxt, "Discarded weapons and armor can be recycled in a recycling center to produce new tools, weapon and armor.") }
      },
      scrap_metal_mixed = {
         -- TRANSLATORS: Helptext for a ware: Scrap metal (mixed)
         purpose = { pgettext(warectxt, "Discarded weapons and armor can be recycled in a recycling center to produce new tools, weapon and armor.") }
      },
      scythe = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Scythe
            pgettext("ware", "The scythe is the tool of the farmers."),
            -- TRANSLATORS: Helptext for a ware: Scythe
            pgettext(warectxt, "Scythes are produced by the blacksmithy.")
         }
      },
      shovel = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Shovel
            pgettext("ware", "Shovels are needed for the proper handling of plants."),
            -- TRANSLATORS: Helptext for a ware: Shovel
            pgettext(warectxt, "They are used by berry and reed farmers as well as foresters. Clay diggers also need them to dig mud out of hard soil.")
         }
      },
      smoked_fish = {
         -- TRANSLATORS: Helptext for a ware: Smoked Fish
         purpose = { pgettext(warectxt, "Fish is smoked in a smokery. Smoked fish is then consumed by soldiers in training or turned into rations and meals for miners and scouts.") }
      },
      smoked_meat = {
         -- TRANSLATORS: Helptext for a ware: Smoked Meat
         purpose = { pgettext(warectxt, "Meat is smoked in a smokery. Smoked meat is then consumed by soldiers in training or turned into rations and meals for miners and scouts.") }
      },
      sword_broad = {
         -- TRANSLATORS: Helptext for a ware: Broadsword
         purpose = { pgettext(warectxt, "The broadsword is the weapon used by level 2 soldiers. Level 5 soldiers are equipped with a broadsword and a double-edged sword.") }
      },
      sword_double = {
         -- TRANSLATORS: Helptext for a ware: Double-edged Sword
         purpose = { pgettext(warectxt, "The double-edged sword is the weapon used by level 3 soldiers. Level 6 soldiers are equipped with two of these ferocious swords.") }
      },
      sword_long = {
         -- TRANSLATORS: Helptext for a ware: Long Sword
         purpose = { pgettext(warectxt, "The long sword is the weapon used by level 1 soldiers. Level 4 soldiers are equipped with a long and a double-edged sword.") }
      },
      sword_short = {
         -- TRANSLATORS: Helptext for a ware: Short sword
         purpose = { pgettext(warectxt, "This is the basic weapon of the Frisian soldiers. Together with a fur garment, it makes up the equipment of young soldiers. Short swords are produced by the small armor smithy.") }
      },
      water = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Water
            pgettext("ware", "Water is the essence of life!"),
            -- TRANSLATORS: Helptext for a ware: Water
            pgettext(warectxt, "Water is used to bake bread and brew beer. Reindeer farms and aqua farms also consume it.")
         }
      }
   },
   workers = {
      frisians_baker = {
         -- TRANSLATORS: Helptext for a frisian worker: Baker
         purpose = { pgettext(workerctxt, "Bakes bread for miners and soldiers.") }
      },
   }
}
pop_textdomain()
return result
