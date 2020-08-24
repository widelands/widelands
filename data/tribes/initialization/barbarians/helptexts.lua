-- Barbarian helptexts

-- For formatting time strings
include "tribes/scripting/help/time_strings.lua"


local buildingctxt = "barbarians_building"
local warectxt = "barbarians_ware"
local workerctxt = "barbarians_worker"

push_textdomain("tribes_encyclopedia")

local result = {
   buildings = {
      constructionsite = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: Construction Site
            pgettext("building", "‘Don’t swear at the builder who is short of building materials.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: Construction Site
            pgettext("building", "Proverb widely used for impossible tasks of any kind")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: Construction Site
            pgettext("building", "A new building is being built at this construction site.")
         }
      },
      dismantlesite = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: Dismantle Site
            pgettext("building", "‘New paths will appear when you are willing to tear down the old.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: Dismantle Site
            pgettext("building", "Proverb")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: Dismantle Site
            pgettext("building", "A building is being dismantled at this dismantle site, returning some of the resources that were used during this building’s construction to your tribe’s stores.")
         }
      },
      barbarians_barrier = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian military site: Barrier
            pgettext(buildingctxt, "‘When we looked down to the valley from our newly established barrier, we felt that the spirit of our fathers was with us.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian military site: Barrier
            pgettext(buildingctxt, "Ballad ‘The Battle of Kal’mavrath’ by Hakhor the Bard")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian military site: Barrier
            pgettext("building", "Garrisons soldiers to expand your territory.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a barbarian military site: Barrier
            pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      barbarians_citadel = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian military site: Citadel
            pgettext(buildingctxt, [[‘The Citadel of Adlen surely is the finest masterpiece of Barbarian craftsmanship. ]] ..
               [[Nothing as strong and big and beautiful has ever been built in such a short time.’]])
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian military site: Citadel
            pgettext(buildingctxt, "Colintan, chief planner of the Citadel of Adlen,<br>at its opening ceremony")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian military site: Citadel
            pgettext("building", "Garrisons soldiers to expand your territory.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a barbarian military site: Citadel
            pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      barbarians_fortress = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian military site: Fortress
            pgettext(buildingctxt, "‘This stronghold made from blackwood and stones will be a hard nut to crack for them.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian military site: Fortress
            pgettext(buildingctxt, "Berthron,<br>chief military adviser of Chat’Karuth")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian military site: Fortress
            pgettext("building", "Garrisons soldiers to expand your territory.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a barbarian military site: Fortress
            pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      barbarians_sentry = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian military site: Sentry
            pgettext(buildingctxt, "‘The log cabin was so small that two men could hardly live there. But we were young and carefree. We just relished our freedom and the responsibility as an outpost.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian military site: Sentry
            pgettext(buildingctxt, "Boldreth,<br>about his time as young soldier")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian military site: Sentry
            pgettext("building", "Garrisons soldiers to expand your territory.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a barbarian military site: Sentry
            pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      barbarians_tower = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian military site: Tower
            pgettext(buildingctxt, "‘From the height of our tower we could see far into enemy territory. The enemy was well prepared, but we also noticed some weak points in their defense.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian military site: Tower
            pgettext(buildingctxt, "The Battle of Kal’mavrath’ by Hakhor the Bard")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian military site: Tower
            pgettext("building", "Garrisons soldiers to expand your territory.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a barbarian military site: Tower
            pgettext("building", "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      barbarians_ax_workshop = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Ax Workshop
            pgettext(buildingctxt, "‘A new warrior’s ax brings forth the best in its wielder – or the worst in its maker.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Ax Workshop
            pgettext(buildingctxt, "An old Barbarian proverb<br> meaning that you need to take some risks sometimes.")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Ax Workshop
            pgettext(buildingctxt, "Produces axes, sharp axes and broad axes.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a barbarian production site: Ax Workshop
            pgettext(buildingctxt, "The Barbarian ax workshop is the intermediate production site in a series of three buildings. It is enhanced from the metal workshop but doesn’t require additional qualification for the worker.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian production site: Ax Workshop
            pgettext(buildingctxt, "If all needed wares are delivered in time, this building can produce each type of ax in about %s on average."):bformat(ngettext("%d second", "%d seconds", 57):bformat(57)),
            -- TRANSLATORS: Performance helptext for a barbarian production site: Ax Workshop
            pgettext("barbarians_building", "All three weapons take the same time for making, but the required raw materials vary.")
         }
      },
      barbarians_bakery = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Bakery
            pgettext(buildingctxt, "‘He who has enough bread will never be too tired to dig the ore and wield the ax.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Bakery
            pgettext(buildingctxt, "Khantarakh, ‘The Modern Barbarian Economy’,<br>3ʳᵈ cowhide ‘Craftsmanship and Trade’")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Bakery
            pgettext(buildingctxt, "Bakes pitta bread for soldiers and miners alike.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian production site: Bakery
            pgettext(buildingctxt, "If all needed wares are delivered in time, this building can produce a pitta bread in %s on average."):bformat(ngettext("%d second", "%d seconds", 30):bformat(30))
         }
      },
      barbarians_barracks = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Barracks
            pgettext(buildingctxt, "‘Don't ask what your tribe can do for you, ask what you can do for your tribe!’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Barracks
            pgettext(buildingctxt, "Famous barbarian recruitment poster")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Barracks
            pgettext(buildingctxt, "Equips recruits and trains them as soldiers.")
         }
      },
      barbarians_big_inn = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Big Inn
            pgettext(buildingctxt, "‘Seasonal fish and game specialties served with pitta bread.<br>Choose from a prime selection of regional craft beer brands.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Big Inn
            pgettext(buildingctxt, "Menu of ‘The Laughing Barbarian’")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Big Inn
            pgettext(buildingctxt, "Prepares rations for scouts and rations, snacks and meals to feed all miners.")
         }
      },
      barbarians_brewery = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Brewery
            pgettext(buildingctxt, [[‘When the Barbarian was created,<br>]] ..
                                   [[this was the first thing that he stated:<br>]] ..
                                   [[“You want me fighting without fear?<br>]] ..
                                   [[Then you should create some beer!”’]])
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Brewery
            pgettext(buildingctxt, "First verse of the drinking song ‘Way of the Barbarian’")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Brewery
            pgettext("building", "Produces beer to keep the miners strong and happy.")
         }
      },
      barbarians_cattlefarm = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Cattle Farm
            pgettext(buildingctxt, "‘The smart leader builds roads, while the really wise leader breeds cattle.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Cattle Farm
            pgettext(buildingctxt, "Khantarakh, ‘The Modern Barbarian Economy’,<br> 5ᵗʰ cowhide ‘Traffic and Logistics’")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Cattle Farm
            pgettext(buildingctxt, "Breeds strong oxen for adding them to the transportation system.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian production site: Cattle Farm
            pgettext(buildingctxt, "If all needed wares are delivered in time, this building can produce an ox in %s on average."):bformat(ngettext("%d second", "%d seconds", 30):bformat(30))
         }
      },
      barbarians_charcoal_kiln = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Charcoal Kiln
            pgettext(buildingctxt, [[‘What’s the work of a charcoal burner?<br>]] ..
                                   [[He’s the tribe’s most plodding earner!<br>]] ..
                                   [[Logs on logs he piles up high,<br>]] ..
                                   [[Until the kiln will reach the sky.<br>]] ..
                                   [[He sets the fire, sees it smolder<br>]] ..
                                   [[The logs he carried on his shoulder.<br>]] ..
                                   [[Burnt down to just one single coal<br>]] ..
                                   [[This troubles much a burner’s soul.’]])
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Charcoal Kiln
            pgettext(buildingctxt, "Song of the charcoal burners")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Charcoal Kiln
            pgettext("building", "Burns logs into charcoal.")
         }
      },
      barbarians_coalmine = {
         lore = {
            -- TRANSLATORS: Lore helptext for production site: Coal Mine, part 1
            pgettext(buildingctxt, "Ages ago, the Barbarians learned to delve into mountainsides for that black material that feeds their furnaces."),
            -- TRANSLATORS: Lore helptext for production site: Coal Mine, part 2
            pgettext(buildingctxt, "Wood may serve for a household fire and to keep you warm, but when it comes to working with iron or gold, there is no way around coal.")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for production site: Coal Mine
            pgettext("building", "Digs coal out of the ground in mountain terrain.")
         },
         note = {
            -- TRANSLATORS: Note helptext for production site: Coal Mine
             pgettext("barbarians_building", "This mine exploits only %s of the resource. From there on out, it will only have a 5%% chance of finding any coal."):bformat("1/3")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for production site: Coal Mine
            pgettext(buildingctxt, "If the food supply is steady, this mine can produce coal in 32.5 seconds on average.")
         }
      },
      barbarians_coalmine_deep = {
         lore = {
            -- TRANSLATORS: Lore helptext for production site: Deep Coal Mine, part 1
            pgettext(buildingctxt, "Ages ago, the Barbarians learned to delve into mountainsides for that black material that feeds their furnaces."),
            -- TRANSLATORS: Lore helptext for production site: Deep Coal Mine
            pgettext(buildingctxt, "Wood may serve for a household fire and to keep you warm, but when it comes to working with iron or gold, there is no way around coal.")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for production site: Deep Coal Mine
            pgettext("building", "Digs coal out of the ground in mountain terrain.")
         },
         note = {
            -- TRANSLATORS: Note helptext for production site: Deep Coal Mine
            pgettext(buildingctxt, "This mine exploits only %s of the resource. From there on out, it will only have a 5%% chance of finding any coal."):bformat("2/3")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for production site: Deep Coal Mine
            pgettext(buildingctxt, "If the food supply is steady, this mine can produce coal in 19.5 seconds on average.")
         }
      },
      barbarians_coalmine_deeper = {
         lore = {
            -- TRANSLATORS: Lore helptext for production site: Deeper Coal Mine, part 1
            pgettext("barbarians_building", "Ages ago, the Barbarians learned to delve into mountainsides for that black material that feeds their furnaces."),
            -- TRANSLATORS: Lore helptext for production site: Deeper Coal Mine, part 2
            pgettext("barbarians_building", "Wood may serve for a household fire and to keep you warm, but when it comes to working with iron or gold, there is no way around coal.")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for production site: Deeper Coal Mine
            pgettext("building", "Digs coal out of the ground in mountain terrain.")
         },
         note = {
            -- TRANSLATORS: Note helptext for production site: Deeper Coal Mine
            pgettext(buildingctxt, "This mine exploits all of the resource down to the deepest level. But even after having done so, it will still have a %s chance of finding some more coal."):bformat("10%")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for production site: Deeper Coal Mine
            pgettext(buildingctxt, "If the food supply is steady, this mine can produce coal in 14.4 seconds on average.")
         }
      },
      barbarians_farm = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Farm
            pgettext(buildingctxt, [[‘See the crop fields from here to the horizons. They are a huge, heaving, golden sea.<br>]] ..
               [[Oh wheat, source of wealth, soul of beer, strength of our warriors!’]])
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Farm
            pgettext(buildingctxt, "Line from the harvesting song ‘The Wealth of the Fields’")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for production site: Farm
            pgettext("building", "Sows and harvests wheat.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for production site: Farm
            pgettext(buildingctxt, "The farmer needs %1% on average to sow and harvest a sheaf of wheat."):bformat(format_minutes_seconds(1, 40))
         }
      },
      barbarians_ferry_yard = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for production site: Ferry Yard
            pgettext("building", "Builds ferries.")
         },
         note = {
            -- TRANSLATORS: Note helptext for production site: Ferry Yard
            pgettext("building", "Needs water nearby.")
         }
      },
      barbarians_fishers_hut = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Fisher's Hut
            pgettext(buildingctxt, "‘What do you mean, my fish ain’t fresh?!’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Fisher's Hut
            pgettext(buildingctxt, "Frequent response of a barbarian fisherman,<br>often followed by an enjoyable brawl")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for production site: Fisher's Hut
            pgettext(buildingctxt, "Fishes on the coast near the fisher’s hut.")
         },
         note = {
            -- TRANSLATORS: Note helptext for production site: Fisher's Hut
            pgettext(buildingctxt, "The fisher’s hut needs water full of fish within the work area.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for production site: Fisher's Hut
            pgettext(buildingctxt, "The fisher pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 18):bformat(18))
         }
      },
      barbarians_gamekeepers_hut = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Gamekeeper's Hut
            pgettext(buildingctxt, "‘He loves the animals and to breed them<br>as we love to cook and eat them.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Gamekeeper's Hut
            pgettext(buildingctxt, "Barbarian nursery rhyme")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for production site: Gamekeeper's Hut
            pgettext(buildingctxt, "Releases animals into the wild to steady the meat production.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for production site: Gamekeeper's Hut
            pgettext(buildingctxt, "The gamekeeper pauses 52.5 seconds before going to work again.")
         }
      },
      barbarians_goldmine = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Gold Mine
            pgettext(buildingctxt, "‘Soft and supple.<br> And yet untouched by time and weather.<br> Rays of sun, wrought into eternity…’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Gold Mine
            pgettext(buildingctxt, "Excerpt from ‘Our Treasures Underground’,<br> a traditional Barbarian song.")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for production site: Gold Mine
            pgettext("building", "Digs gold ore out of the ground in mountain terrain.")
         },
         note = {
            -- TRANSLATORS: Note helptext for production site: Gold Mine
            pgettext(buildingctxt, "This mine exploits only %s of the resource. From there on out, it will only have a 5%% chance of finding any gold ore."):bformat("1/3")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for production site: Gold Mine
            pgettext(buildingctxt, "If the food supply is steady, this mine can produce gold ore in %s on average."):bformat(ngettext("%d second", "%d seconds", 65):bformat(65))
         }
      },
      barbarians_goldmine_deep = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Deep Gold Mine
            pgettext(buildingctxt, "‘Soft and supple.<br> And yet untouched by time and weather.<br> Rays of sun, wrought into eternity…’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Deep Gold Mine
            pgettext(buildingctxt, "Excerpt from ‘Our Treasures Underground’,<br> a traditional Barbarian song.")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for production site: Deep Gold Mine
            pgettext("building", "Digs gold ore out of the ground in mountain terrain.")
         },
         note = {
            -- TRANSLATORS: Note helptext for production site: Deep Gold Mine
            pgettext(buildingctxt, "This mine exploits only %s of the resource. From there on out, it will only have a 5%% chance of finding any gold ore."):bformat("2/3")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for production site: Deep Gold Mine
            pgettext(buildingctxt, "If the food supply is steady, this mine can produce gold ore in 19.5 seconds on average.")
         }
      },
      barbarians_goldmine_deeper = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Deeper Gold Mine
            pgettext(buildingctxt, "‘Soft and supple.<br> And yet untouched by time and weather.<br> Rays of sun, wrought into eternity…’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Deeper Gold Mine
            pgettext(buildingctxt, "Excerpt from ‘Our Treasures Underground’,<br> a traditional Barbarian song.")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for production site: Deeper Gold Mine
            pgettext("building", "Digs gold ore out of the ground in mountain terrain.")
         },
         note = {
            -- TRANSLATORS: Note helptext for production site: Deeper Gold Mine
            pgettext(buildingctxt, "This mine exploits all of the resource down to the deepest level. But even after having done so, it will still have a %s chance of finding some more gold ore."):bformat("10%")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for production site: Deeper Gold Mine
            pgettext(buildingctxt, "If the food supply is steady, this mine can produce gold ore in 18.5 seconds on average.")
         }
      },
      barbarians_granitemine = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Granite Mine
            pgettext(buildingctxt, "‘I can handle tons of granite, man, but no more of your vain prattle.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Granite Mine, part 1
            pgettext(buildingctxt, "This phrase was the reply Rimbert the miner – later known as Rimbert the loner – gave, when he was asked to remain seated on an emergency meeting at Stonford in the year of the great flood."),
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Granite Mine, part 2
            pgettext(buildingctxt, "The same man had all the 244 granite blocks ready only a week later, and they still fortify the city’s levee.")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Granite Mine
            pgettext(buildingctxt, "Carves granite out of the rock in mountain terrain.")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian production site: Granite Mine
            pgettext(buildingctxt,"This mine exploits all of the resource down to the deepest level. But even after having done so, it will still have a %s chance of finding some more granite."):bformat("5%"),
            -- TRANSLATORS: 'It' is a mine
            pgettext(buildingctxt, "It cannot be enhanced.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian production site: Granite Mine
            pgettext(buildingctxt, "If the food supply is steady, this mine can produce granite in %s on average."):bformat(ngettext("%d second", "%d seconds", 20):bformat(20))
         }
      },
      barbarians_helmsmithy = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Helm Smithy
            pgettext(buildingctxt, "‘The helmets forgéd by this smithy<br>Are widely known for looking pithy.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Helm Smithy
            pgettext(buildingctxt, "Slogan of ‘Harrath’s Handsome Helmets’ in Al’thunran")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Helm Smithy
            pgettext(buildingctxt, "Forges helmets that are used for training soldiers’ health in the training camp.")
         }
      },
      barbarians_hunters_hut = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Hunter's Hut
            pgettext(buildingctxt, "‘As silent as a panther,<br> as deft as a weasel,<br> as swift as an arrow,<br> as deadly as a viper.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Hunter's Hut
            pgettext(buildingctxt, "‘The Art of Hunting’")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Hunter's Hut
            pgettext("building", "Hunts animals to produce meat.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a barbarian production site: Hunter's Hut
            pgettext(buildingctxt, "The hunter’s hut needs animals to hunt within the work area.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian production site: Hunter's Hut
            pgettext(buildingctxt, "The hunter pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 35):bformat(35))
         }
      },
      barbarians_inn = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Inn
            pgettext(buildingctxt, "‘Miner’s Delight: Pitta bread with fish or meat, comes with one beer.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Inn
            pgettext(buildingctxt, "Chef’s recommendation in ‘The Singing Master Miner’")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Inn
            pgettext("building", "Prepares rations for scouts and rations and snacks to feed the miners in the basic and deep mines.")
         }
      },
      barbarians_ironmine = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Iron Mine
            pgettext(buildingctxt, "‘I look at my own pick wearing away day by day and I realize why my work is important.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Iron Mine
            pgettext(buildingctxt, "Quote from an anonymous miner.")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Iron Mine
            pgettext("building", "Digs iron ore out of the ground in mountain terrain.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a barbarian production site: Iron Mine
            pgettext(buildingctxt, "This mine exploits only %s of the resource. From there on out, it will only have a 5%% chance of finding any iron ore."):bformat("1/3")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian production site: Iron Mine
            pgettext(buildingctxt, "If the food supply is steady, this mine can produce iron ore in %s on average."):bformat(ngettext("%d second", "%d seconds", 65):bformat(65))
         }
      },
      barbarians_ironmine_deep = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Deep Iron Mine
            pgettext(buildingctxt, "‘I look at my own pick wearing away day by day and I realize why my work is important.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Deep Iron Mine
            pgettext(buildingctxt, "Quote from an anonymous miner.")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Deep Iron Mine
            pgettext("building", "Digs iron ore out of the ground in mountain terrain.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a barbarian production site: Deep Iron Mine
            pgettext(buildingctxt, "This mine exploits only %s of the resource. From there on out, it will only have a 5%% chance of finding any iron ore."):bformat("2/3")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian production site: Deep Iron Mine
            pgettext(buildingctxt, "If the food supply is steady, this mine can produce iron ore in 39.5 seconds on average.")
         }
      },
      barbarians_ironmine_deeper = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Deeper Iron Mine
            pgettext(buildingctxt, "‘I look at my own pick wearing away day by day and I realize why my work is important.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Deeper Iron Mine
            pgettext(buildingctxt, "Quote from an anonymous miner.")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Deeper Iron Mine
            pgettext("building", "Digs iron ore out of the ground in mountain terrain.")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian production site: Deeper Iron Mine
            pgettext(buildingctxt, "This mine exploits all of the resource down to the deepest level. But even after having done so, it will still have a %s chance of finding some more iron ore."):bformat("10%")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian production site: Deeper Iron Mine
            pgettext(buildingctxt, "If the food supply is steady, this mine can produce iron ore in 17.6 seconds on average.")
         }
      },
      barbarians_lime_kiln = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Lime Kiln
            pgettext(buildingctxt, "‘Forming new stone from old with fire and water.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Lime Kiln
            pgettext(buildingctxt, "Ragnald the Child’s answer to the question, what he’s doing in the dirt. His ‘new stone’ now is an important building material.")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Lime Kiln
            pgettext(buildingctxt, "Processes granite to make ‘grout’, a substance that solidifies and so reinforces masonry.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a barbarian production site: Lime Kiln
            pgettext(buildingctxt, "The lime kiln’s output will only go to construction sites that need it. Those are predominantly houses that work with fire, and some military sites.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian production site: Lime Kiln
            pgettext(buildingctxt, "If all needed wares are delivered in time, this building can produce grout in about %s on average."):bformat(ngettext("%d second", "%d seconds", 41):bformat(41))
         }
      },
      barbarians_lumberjacks_hut = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Lumberjack_s Hut
            pgettext(buildingctxt, "‘Take 200 hits to fell a tree and you’re a baby. Take 100 and you’re a soldier. Take 50 and you’re a hero. Take 20 and soon you will be a honorable lumberjack.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Lumberjack_s Hut
            pgettext(buildingctxt, "Krumta, carpenter of Chat’Karuth")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Lumberjack_s Hut
            pgettext("building", "Fells trees in the surrounding area and processes them into logs.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a barbarian production site: Lumberjack_s Hut
            pgettext(buildingctxt, "The lumberjack’s hut needs trees to fell within the work area.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian production site: Lumberjack_s Hut
            pgettext(buildingctxt, "The lumberjack needs %s to fell a tree, not counting the time he needs to reach the destination and go home again."):bformat(ngettext("%d second", "%d seconds", 12):bformat(12))
         }
      },
      barbarians_metal_workshop = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Metal Workshop
            pgettext(buildingctxt, "‘We make it work!’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Metal Workshop
            pgettext(buildingctxt, "Inscription on the threshold of the now ruined Olde Forge at Harradsheim, the eldest known smithy.")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Metal Workshop
            pgettext("building", "Forges all the tools that your workers need.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a barbarian production site: Metal Workshop
            pgettext(buildingctxt, "The Barbarian metal workshop is the basic production site in a series of three buildings and creates all the tools that Barbarians need. The others are for weapons.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian production site: Metal Workshop
            pgettext(buildingctxt, "If all needed wares are delivered in time, this building can produce each tool in about %s on average."):bformat(ngettext("%d second", "%d seconds", 67):bformat(67))
         }
      },
      barbarians_micro_brewery = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Micro Brewery
            pgettext(buildingctxt, "‘Let the first one drive away the hunger, the second one put you at ease; when you have swallowed up your third one, it’s time for the next shift!’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Micro Brewery
            pgettext(buildingctxt, "Widespread toast among miners")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Micro Brewery
            pgettext(buildingctxt, "Produces beer of the lower grade. This beer is a vital component of the snacks that inns and big inns prepare for miners in deep mines.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian production site: Micro Brewery
            pgettext(buildingctxt, "If all needed wares are delivered in time, this building can produce beer in about %s on average."):bformat(ngettext("%d second", "%d seconds", 60):bformat(60))
         }
      },
      barbarians_quarry = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Quarry
            pgettext(buildingctxt, "‘We open up roads and make houses from mountains.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Quarry
            pgettext(buildingctxt, "Slogan of the stonemasons’ guild")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Quarry
            pgettext(buildingctxt, "Cuts raw pieces of granite out of rocks in the vicinity.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a barbarian production site: Quarry
            pgettext(buildingctxt, "The quarry needs rocks to cut within the work area.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian production site: Quarry
            pgettext(buildingctxt, "The stonemason pauses %s before going back to work again."):bformat(ngettext("%d second", "%d seconds", 65):bformat(65))
         }
      },
      barbarians_rangers_hut = {
         lore = {
            --"Wer zwei Halme dort wachsen lässt, wo sonst nur einer wächst, der ist größer als der größte Feldherr!" – Friedrich der Große
            -- TRANSLATORS: Lore helptext for a barbarian production site: Ranger's Hut
            pgettext(buildingctxt, "‘He who can grow two trees where normally only one will grow exceeds the most important general!’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Ranger's Hut
            pgettext(buildingctxt, "Chat’Karuth in a conversation with a Ranger")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Ranger's Hut
            pgettext("building", "Plants trees in the surrounding area.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a barbarian production site: Ranger's Hut
            pgettext(buildingctxt, "The ranger’s hut needs free space within the work area to plant the trees.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian production site: Ranger's Hut
            pgettext(buildingctxt, "The ranger needs %s to plant a tree, not counting the time he needs to reach the destination and go home again."):bformat(ngettext("%d second", "%d seconds", 5):bformat(5))
         }
      },
      barbarians_reed_yard = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Reed Yard
            pgettext(buildingctxt, "‘We grow roofs’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Reed Yard
            pgettext(buildingctxt, "Slogan of the Guild of Gardeners")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Reed Yard
            pgettext(buildingctxt, "Cultivates reed that serves two different purposes for the Barbarian tribe.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a barbarian production site: Reed Yard
            pgettext(buildingctxt, "Reed is the traditional material for roofing, and it is woven into the extremely durable cloth that they use for their ships’ sails.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian production site: Reed Yard
            pgettext(buildingctxt, "A reed yard can produce a sheaf of reed in about %s on average."):bformat(ngettext("%d second", "%d seconds", 65):bformat(65))
         }
      },
      barbarians_scouts_hut = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Scout's Hut
            pgettext(buildingctxt, "‘Behind the next hill there might be wealth and happiness but also hostility and doom.<br>He who will not explore it commits the crime of stupidity.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Scout's Hut
            pgettext(buildingctxt, "Chat’Karuth<br>at the oath taking ceremony of the first scout troupe")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Scout's Hut
            pgettext("building", "Explores unknown territory.")
         }
      },
      barbarians_shipyard = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Shipyard
            pgettext(buildingctxt, [[‘When I saw the Saxnot for the first time, her majestic dragon head already looked up to the skies and the master was about to install the square sail.<br>]] ..
               [[It was the most noble ship I ever saw.’]])
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Shipyard
            pgettext(buildingctxt, "Captain Thanlas the Elder,<br>Explorer")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Shipyard
            pgettext("building", "Constructs ships that are used for overseas colonization and for trading between ports.")
         }
      },
      barbarians_smelting_works = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Smelting Works
            pgettext(buildingctxt, [[‘From inferior looking rubble they form shiny gold and gleaming iron.<br>]] ..
               [[The smelters are real magicians among our craftsmen.’]])
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Smelting Works
            pgettext(buildingctxt, "Chat’Karuth<br>at the 5ᵗʰ conference of master craftsmen")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Smelting Works
            pgettext("building", "Smelts iron ore into iron and gold ore into gold.")
         }
      },
      barbarians_tavern = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Tavern
            pgettext(buildingctxt, "‘Fresh from the countryside: Bread, fish or game to go.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Tavern
            pgettext(buildingctxt, "Menu of the tavern ‘Ye Ol’ Bronze Axe’")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Tavern
            pgettext("building", "Prepares rations to feed the scouts and miners.")
         }
      },
      barbarians_warmill = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Warmill
            pgettext(buildingctxt, "‘A new warrior’s ax brings forth the best in its wielder – or the worst in its maker.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Warmill
            pgettext(buildingctxt, "An old Barbarian proverb<br> meaning that you need to take some risks sometimes.]])")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Warmill
            pgettext(buildingctxt, "Produces all the axes that the Barbarians use for warfare.")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian production site: Warmill
            pgettext(buildingctxt, "The Barbarian war mill is their most advanced production site for weapons. As such you will need to enhance an ax workshop in order to get one.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian production site: Warmill, part 1
            pgettext(buildingctxt, "If all needed wares are delivered in time, this building can produce each type of ax in about %s on average."):bformat(ngettext("%d second", "%d seconds", 57):bformat(57)),
            -- TRANSLATORS: Performance helptext for a barbarian production site: Warmill, part 2
            pgettext(buildingctxt, "All weapons require the same time for making, but the consumption of the same raw materials increases from step to step. The last two need gold.")
         }
      },
      barbarians_weaving_mill = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Weaving Mill
            pgettext(buildingctxt, [[‘Fine cloth and jewelry mean nothing to a true Barbarian.<br>]] ..
            [[But a noble sail in the wind will please his eyes.’]])
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Weaving Mill
            pgettext(buildingctxt, "Khantarakh, ‘The Modern Barbarian Economy’,<br>9ᵗʰ cowhide ‘Seafaring and Conquest’")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Weaving Mill
            pgettext(buildingctxt, "Weaves cloth out of reed.")
         }
      },
      barbarians_well = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Well
            pgettext(buildingctxt, [[‘Oh how sweet is the source of life,<br> that comes down from the sky <br> and lets the earth drink.’]])
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Well
            pgettext(buildingctxt, "Song written by Sigurd the Bard when the first rain fell after the Great Drought in the 21ˢᵗ year of Chat’Karuth’s reign.")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Well
            pgettext("building", "Draws water out of the deep.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian production site: Well
            pgettext(buildingctxt, "The carrier needs %s to get one bucket full of water."):bformat(ngettext("%d second", "%d seconds", 40):bformat(40))
         }
      },
      barbarians_wood_hardener = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Wood Hardener
            pgettext(buildingctxt, [[‘Then he hit the door with his ax and, behold,<br>]] ..
                                   [[the door only trembled, but the shaft of the ax burst into a thousand pieces.’]])
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Wood Hardener
            pgettext(buildingctxt, "Legend about a quarrel between the brothers Harthor and Manthor,<br>Manthor is said to be the inventor of blackwood")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Wood Hardener
            pgettext(buildingctxt, "Hardens logs by fire into blackwood, which is used to construct robust buildings.")
         }
      },
      barbarians_battlearena = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian training site: Battle Arena
            pgettext(buildingctxt, "‘No better friend you have in battle than the enemy’s blow that misses.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian training site: Battle Arena
            pgettext(buildingctxt, "Said to originate from Neidhardt, the famous trainer.")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian training site: Battle Arena, part 1
            pgettext(buildingctxt, "Trains soldiers in ‘Evade’."),
            -- TRANSLATORS: Purpose helptext for a barbarian training site: Battle Arena, part 2
            pgettext(buildingctxt, "‘Evade’ increases the soldier’s chance not to be hit by the enemy and so to remain totally unaffected.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a barbarian training site: Battle Arena
            pgettext(buildingctxt, "Barbarian soldiers cannot be trained in ‘Defense’ and will remain at their initial level.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian training site: Battle Arena
            pgettext(buildingctxt, "If all needed wares are delivered in time, a battle arena can train evade for one soldier from 0 to the highest level in %1$s and %2$s on average."):bformat(ngettext("%d minute", "%d minutes", 1):bformat(1), ngettext("%d second", "%d seconds", 10):bformat(10))
         }
      },
      barbarians_trainingcamp = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian training site: Training Camp
            pgettext(buildingctxt, "‘He who is strong shall neither forgive nor forget, but revenge injustice suffered – in the past and for all future.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian training site: Training Camp
            pgettext(buildingctxt, "Chief Chat’Karuth in a speech to his army.")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian training site: Training Camp, part 1
            pgettext(buildingctxt, "Trains soldiers in ‘Attack’ and in ‘Health’."),
            -- TRANSLATORS: Purpose helptext for a barbarian training site: Training Camp, part 2
            pgettext(buildingctxt, "Equips the soldiers with all necessary weapons and armor parts.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: Training Camp
            pgettext(buildingctxt, "Barbarian soldiers cannot be trained in ‘Defense’ and will remain at their initial level.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian training site: Training Camp
            pgettext(buildingctxt, "If all needed wares are delivered in time, a training camp can train one new soldier in attack and health to the final level in %1$s and %2$s on average."):bformat(ngettext("%d minute", "%d minutes", 4):bformat(4), ngettext("%d second", "%d seconds", 40):bformat(40))
         }
      },
      barbarians_headquarters = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian warehouse: Headquarters
            pgettext(buildingctxt, "‘Steep like the slopes of Kal’mavrath, shiny like the most delicate armor and strong like our ancestors, that’s how the headquarters of Chat’Karuth presented itself to us.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian warehouse: Headquarters
            pgettext(buildingctxt, "Ballad ‘The Battle of Kal’mavrath’ by Hakhor the Bard")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian warehouse: Headquarters
            pgettext(buildingctxt, "Accommodation for your people. Also stores your wares and tools.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a barbarian warehouse: Headquarters
            pgettext(buildingctxt, "The headquarters is your main building.")
         }
      },
      barbarians_headquarters_interim = {
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian warehouse: Headquarters
            pgettext(buildingctxt, "Accommodation for your people. Also stores your wares and tools.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a barbarian warehouse: Headquarters
            pgettext(buildingctxt, "The headquarters is your main building.")
         }
      },
      barbarians_port = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian warehouse: Port
            pgettext(buildingctxt, "‘I prefer the planks of a ship to any fortress, no matter how strong it is.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian warehouse: Port
            pgettext(buildingctxt, "Captain Thanlas the Elder,<br>Explorer")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian warehouse: Port
            pgettext(buildingctxt, "Serves as a base for overseas colonization and trade. Also stores your soldiers, wares and tools.")
         }
      },
      barbarians_warehouse = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian warehouse: Warehouse
            pgettext(buildingctxt, "‘Who still owns a warehouse is not yet defeated!’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian warehouse: Warehouse
            pgettext(buildingctxt, "Berthron, chief military adviser of Chat’Karuth,<br>when they lost the headquarters in the battle around the heights of Kal’Megarath")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian warehouse: Warehouse
            pgettext("building", "Your workers and soldiers will find shelter here. Also stores your wares and tools.")
         }
      }
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
      barbarians_resi_coal_1 = {
         purpose = {
            -- TRANSLATORS: Helptext for a resource indicator: Coal, part 1
            _("Coal veins contain coal that can be dug up by coal mines."),
            -- TRANSLATORS: Helptext for a resource indicator: Coal part 2
            _("There is only a little bit of coal here.")
         }
      },
      barbarians_resi_coal_2 = {
         purpose = {
            -- TRANSLATORS: Helptext for a resource indicator: Coal, part 1
            _("Coal veins contain coal that can be dug up by coal mines."),
            -- TRANSLATORS: Helptext for a resource indicator: Coal, part 2
            _("There is a lot of coal here.")
         }
      },
      barbarians_resi_gold_1 = {
         purpose = {
            -- TRANSLATORS: Helptext for a resource indicator: Gold, part 1
            _("Gold veins contain gold ore that can be dug up by gold mines."),
            -- TRANSLATORS: Helptext for a resource indicator: Gold, part 2
            _("There is only a little bit of gold here.")
         }
      },
      barbarians_resi_gold_2 = {
         purpose = {
            -- TRANSLATORS: Helptext for a resource indicator: Gold, part 1
            _("Gold veins contain gold ore that can be dug up by gold mines."),
            -- TRANSLATORS: Helptext for a resource indicator: Gold, part 2
            _("There is a lot of gold here.")
         }
      },
      barbarians_resi_iron_1 = {
         purpose = {
            -- TRANSLATORS: Helptext for a resource indicator: Iron, part 1
            _("Iron veins contain iron ore that can be dug up by iron mines."),
            -- TRANSLATORS: Helptext for a resource indicator: Iron, part 2
            _("There is only a little bit of iron here.")
         }
      },
      barbarians_resi_iron_2 = {
         purpose = {
            -- TRANSLATORS: Helptext for a resource indicator: Iron, part 1
            _("Iron veins contain iron ore that can be dug up by iron mines."),
            -- TRANSLATORS: Helptext for a resource indicator: Iron, part 2
            _("There is a lot of iron here.")
         }
      },
      barbarians_resi_none = {
         -- TRANSLATORS: Helptext for a resource indicator: No resources
         purpose = { _("There are no resources in the ground here.") }
      },
      barbarians_resi_water = {
         -- TRANSLATORS: Helptext for a resource indicator: Water
         purpose = { _("There is water in the ground here that can be pulled up by a well.") }
      },
      barbarians_resi_stones_1 = {
         purpose = {
            -- TRANSLATORS: Helptext for a Barbarian resource indicator: Stones, part 1
            _("Granite is a basic building material and can be dug up by a granite mine."),
            -- TRANSLATORS: Helptext for a Barbarian resource indicator: Stones, part 2
            _("There is only a little bit of granite here."),
         }
      },
      barbarians_resi_stones_2 = {
         purpose = {
            -- TRANSLATORS: Helptext for a Barbarian resource indicator: Stones, part 1
            _("Granite is a basic building material and can be dug up by a granite mine."),
            -- TRANSLATORS: Helptext for a Barbarian resource indicator: Stones, part 2
            _("There is a lot of granite here.")
         }
      },
      barbarians_shipconstruction = {
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
      ax = {
         -- TRANSLATORS: Helptext for a ware: Ax
         purpose = { pgettext(warectxt, "The ax is the basic weapon of the Barbarians. All young soldiers are equipped with it.") }
      },
      ax_battle = {
         -- TRANSLATORS: Helptext for a ware: Battle Ax
         purpose = { pgettext(warectxt, "This is a dangerous weapon the Barbarians are able to produce. It is produced in the war mill. Only trained soldiers are able to wield such a weapon. It is used – together with food – in the training camp to train soldiers from attack level 3 to 4.") }
      },
      ax_broad = {
         -- TRANSLATORS: Helptext for a ware: Broad Ax
         purpose = { pgettext(warectxt, "The broad ax is produced by the ax workshop and the war mill. It is used in the training camp – together with food – to train soldiers with a fundamental fighting knowledge from attack level 1 to attack level 2.") }
      },
      ax_bronze = {
         -- TRANSLATORS: Helptext for a ware: Bronze Ax
         purpose = { pgettext(warectxt, "The bronze ax is considered a weapon that is hard to handle. Only skilled soldiers can use it. It is produced at the war mill and used in the training camp – together with food – to train soldiers from attack level 2 to level 3.") }
      },
      ax_sharp = {
         -- TRANSLATORS: Helptext for a ware: Sharp Ax
         purpose = { pgettext(warectxt, "Young soldiers are proud to learn to fight with this powerful weapon. It is heavier and sharper than the ordinary ax. It is produced in ax workshops and war mills. In training camps, it is used – together with food – to train soldiers from attack level 0 to level 1.") }
      },
      ax_warriors = {
         -- TRANSLATORS: Helptext for a ware: Warrior’s Ax
         purpose = { pgettext(warectxt, "The warrior’s ax is the most dangerous of all Barbarian weapons. Only a few soldiers ever were able to handle this huge and powerful ax. It is produced in a war mill and used – together with food – in a training camp to train soldiers from attack level 4 to level 5.") }
      },
      barbarians_bread = {
         -- TRANSLATORS: Helptext for a ware: Pitta Bread
         purpose = { pgettext(warectxt, "The Barbarian bakers are best in making this flat and tasty pitta bread. It is made out of wheat and water following a secret recipe. Pitta bread is used in the taverns, inns and big inns to prepare rations, snacks and meals. It is also consumed at training sites (training camp and battle arena).") }
      },
      beer = {
         -- TRANSLATORS: Helptext for a ware: Beer
         purpose = { pgettext(warectxt, "Beer is produced in micro breweries and used in inns and big inns to produce snacks.") }
      },
      beer_strong = {
         -- TRANSLATORS: Helptext for a ware: Strong Beer
         purpose = { pgettext(warectxt, "Only this beer is acceptable for the soldiers in a battle arena. Some say that the whole power of the Barbarians lies in this ale. It helps to train the soldiers’ evade level from 0 to 1 to 2. Strong beer is also used in big inns to prepare meals.") }
      },
      blackwood = {
         -- TRANSLATORS: Helptext for a ware: Blackwood
         purpose = { pgettext(warectxt, "This fire-hardened wood is as hard as iron and it is used for several buildings. It is produced out of logs in the wood hardener.") }
      },
      bread_paddle = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Bread Paddle, part 1
            pgettext("ware", "The bread paddle is the tool of the baker, each baker needs one."),
            -- TRANSLATORS: Helptext for a ware: Bread Paddle, part 2
            pgettext(warectxt, "Bread paddles are produced in the metal workshop like all other tools (but cease to be produced by the building if it is enhanced to an ax workshop and war mill).")
         }
      },
      cloth = {
         -- TRANSLATORS: Helptext for a ware: Cloth
         purpose = { pgettext(warectxt, "Cloth is needed for Barbarian ships. It is produced out of reed.") }
      },
      coal = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Coal, part 1
            pgettext("ware", "Coal is mined in coal mines or produced out of logs by a charcoal kiln."),
            -- TRANSLATORS: Helptext for a ware: Coal, part 2
            pgettext(warectxt, "The fires of the Barbarians are usually fed with coal. Consumers are several buildings: lime kiln, smelting works, ax workshop, war mill, and helm smithy.")
         }
      },
      felling_ax = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Felling Ax, part1
            pgettext("ware", "The felling ax is the tool to chop down trees."),
            -- TRANSLATORS: Helptext for a ware: Felling Ax, part 2
            pgettext(warectxt, "Felling axes are used by lumberjacks and produced in the metal workshop (but cease to be produced by the building if it is enhanced to an ax workshop and war mill).")
         }
      },
      fire_tongs = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Fire Tongs, part 1
            pgettext("ware", "Fire tongs are the tools for smelting ores."),
            -- TRANSLATORS: Helptext for a ware: Fire Tongs part 2
            pgettext(warectxt, "They are used in the smelting works and produced by the metal workshop (but they cease to be produced by the building if it is enhanced to an ax workshop and war mill).")
         }
      },
      fish = {
         -- TRANSLATORS: Helptext for a ware: Fish
         purpose = { pgettext(warectxt, "Besides pitta bread and meat, fish is also a foodstuff for the Barbarians. It is used in the taverns, inns and big inns and at the training sites (training camp and battle arena).") }
      },
      fishing_rod = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Fishing Rod, part 1
            pgettext("ware", "Fishing rods are needed by fishers to catch fish."),
            -- TRANSLATORS: Helptext for a ware: Fishing Rod, part 2
            pgettext(warectxt, "They are one of the basic tools produced in a metal workshop (but cease to be produced by the building if it is enhanced to an ax workshop and war mill).")
         }
      },
      gold = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Gold, part 1
            pgettext("ware", "Gold is the most valuable of all metals, and it is smelted out of gold ore."),
            -- TRANSLATORS: Helptext for a ware: Gold, part 2
            pgettext(warectxt, "Only very important things are embellished with gold. It is produced by the smelting works and used as a precious building material and to produce different axes (in the war mill) and different parts of armor (in the helm smithy).")
         }
      },
      gold_ore = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Gold Ore, part 1
            pgettext("ware", "Gold ore is mined in a gold mine."),
            -- TRANSLATORS: Helptext for a ware: Gold Ore, part 2
            pgettext(warectxt, "Smelted in a smelting works, it turns into gold which is used as a precious building material and to produce weapons and armor.")
         }
      },
      granite = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Granite, part 1
            pgettext("ware", "Granite is a basic building material."),
            -- TRANSLATORS: Helptext for a ware: Granite, part 2
            pgettext(warectxt, "The Barbarians produce granite blocks in quarries and granite mines.")
         }
      },
      grout = {
         -- TRANSLATORS: Helptext for a ware: Grout
         purpose = { pgettext(warectxt, "Granite can be processed into grout which provides a solid, non burning building material. Grout is produced in a lime kiln.") }
      },
      hammer = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Hammer, part 1
            pgettext("ware", "The hammer is an essential tool."),
            -- TRANSLATORS: Helptext for a ware: Hammer, part 2
            pgettext(warectxt, "Geologists, builders, blacksmiths and helmsmiths all need a hammer. Make sure you’ve always got some in reserve! They are one of the basic tools produced at the metal workshop (but cease to be produced by the building if it is enhanced to an ax workshop and war mill).")
         }
      },
      helmet = {
         -- TRANSLATORS: Helptext for a ware: Helmet
         purpose = { pgettext(warectxt, "A helmet is a basic tool to protect soldiers. It is produced in the helm smithy and used in the training camp – together with food – to train soldiers from health level 0 to level 1.") }
      },
      helmet_mask = {
         -- TRANSLATORS: Helptext for a ware: Mask
         purpose = { pgettext(warectxt, "A mask is an enhanced armor for Barbarian soldiers. It is produced in the helm smithy and used in the training camp – together with food – to train soldiers from health level 1 to level 2.") }
      },
      helmet_warhelm = {
         -- TRANSLATORS: Helptext for a ware: Warhelm
         purpose = { pgettext(warectxt, "This is the most enhanced Barbarian armor. It is produced in a helm smithy and used in a training camp – together with food – to train soldiers from health level 2 to level 3.") }
      },
      hunting_spear = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Hunting Spear, part 1
            pgettext("ware", "This spear is light enough to be thrown, but heavy enough to kill any animal in one blow. It is only used by hunters."),
            -- TRANSLATORS: Helptext for a ware: Hunting Spear, part 2
            pgettext(warectxt, "Hunting spears are produced in the metal workshop (but cease to be produced by the building if it is enhanced to an ax workshop and war mill).")
         }
      },
      iron = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Iron, part 1
            pgettext("ware", "Iron is smelted out of iron ores."),
            -- TRANSLATORS: Helptext for a ware: Iron, part 2
            pgettext(warectxt, "It is produced by the smelting works and used to produce weapons and tools in the metal workshop, ax workshop, war mill and helm smithy.")
         }
      },
      iron_ore = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Iron Ore, part 1
            pgettext("default_ware", "Iron ore is mined in iron mines."),
            -- TRANSLATORS: Helptext for a ware: Iron Ore, part 2
            pgettext(warectxt, "It is smelted in a smelting works to retrieve the iron.")
         }
      },
      kitchen_tools = {
         -- TRANSLATORS: Helptext for a ware: Kitchen Tools
         purpose = { pgettext(warectxt, "Kitchen tools are needed for preparing rations, snacks and meals. Be sure to have a metal workshop to produce this basic tool (but it ceases to be produced by the building if it is enhanced to an ax workshop and war mill).") }
      },
      log = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Log, part 1
            pgettext("ware", "Logs are an important basic building material. They are produced by felling trees."),
            -- TRANSLATORS: Helptext for a ware: Log, part 2
            pgettext(warectxt, "Barbarian lumberjacks fell the trees; rangers take care of the supply of trees. Logs are also used in the metal workshop to build basic tools, and in the charcoal kiln for the production of coal. The wood hardener refines logs into blackwood by hardening them with fire.")
         }
      },
      meal = {
         -- TRANSLATORS: Helptext for a ware: Meal
         purpose = { pgettext(warectxt, "A meal is made out of pitta bread, strong beer and fish/meat in a big inn. This substantial food is exactly what workers in a deeper mine need.") }
      },
      meat = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Meat, part 1
            pgettext("ware", "Meat contains a lot of energy, and it is obtained from wild game taken by hunters."),
            -- TRANSLATORS: Helptext for a ware: Meat, part 2
            pgettext(warectxt, "Meat is used in the taverns, inns and big inns to prepare rations, snacks and meals for the miners. It is also consumed at the training sites (training camp and battle arena).")
         }
      },
      pick = {
         -- TRANSLATORS: Helptext for a ware: Pick
         purpose = { pgettext(warectxt, "Picks are used by stonemasons and miners. They are produced in the metal workshop (but cease to be produced by the building if it is enhanced to an ax workshop and war mill).") }
      },
      ration = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Ration, part 1
            pgettext("ware", "A small bite to keep miners strong and working. The scout also consumes rations on his scouting trips."),
            -- TRANSLATORS: Helptext for a ware: Ration, part 2
            pgettext(warectxt, "Rations are produced in a tavern, an inn or a big inn out of fish or meat or pitta bread.")
         }
      },
      reed = {
         -- TRANSLATORS: Helptext for a ware: Reed
         purpose = { pgettext(warectxt, "Reed is produced in a reed yard and used to make the roofs of buildings waterproof.") }
      },
      scythe = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Scythe, part 1
            pgettext("ware", "The scythe is the tool of the farmers."),
            -- TRANSLATORS: Helptext for a ware: Scythe, part 2
            pgettext(warectxt, "Scythes are produced by the metal workshop (but cease to be produced by the building if it is enhanced to an ax workshop and war mill).")
         }
      },
      shovel = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Shovel, part 1
            pgettext("ware", "Shovels are needed for the proper handling of plants."),
            -- TRANSLATORS: Helptext for a ware: Shovel, part 2
            pgettext(warectxt, "Therefore the gardener and the ranger use them. Produced at the metal workshop (but cease to be produced by the building if it is enhanced to an ax workshop and war mill).")
         }
      },
      snack = {
         -- TRANSLATORS: Helptext for a ware: Snack
         purpose = { pgettext(warectxt, "A bigger morsel than the ration to provide miners in deep mines. It is produced in an inn or a big inn out of fish/meat, pitta bread and beer.") }
      },
      water = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Water, part 1
            pgettext("ware", "Water is the essence of life!"),
            -- TRANSLATORS: Helptext for a ware: Water, part 2
            pgettext(warectxt, "Water is used in the bakery, the micro brewery and the brewery. The lime kiln and the cattle farm also need to be supplied with water.")
         }
      },
      wheat = {
         purpose = {
            -- TRANSLATORS: Helptext for a ware: Wheat, part 1
            pgettext("ware", "Wheat is essential for survival."),
            -- TRANSLATORS: Helptext for a ware: Wheat, part 2
            pgettext(warectxt, "Wheat is produced by farms and consumed by bakeries, micro breweries and breweries. Cattle farms also need to be supplied with wheat.")
         }
      }
   },
   workers = {
      barbarians_baker = {
         -- TRANSLATORS: Helptext for a barbarian worker: Baker
         purpose = { pgettext(workerctxt, "Bakes pitta bread for the miners, soldiers and scouts.") }
      },
      barbarians_blacksmith = {
         -- TRANSLATORS: Helptext for a barbarian worker: Blacksmith
         purpose = { pgettext(workerctxt, "Produces weapons for soldiers and tools for workers.") }
      },
      barbarians_blacksmith_master = {
         -- TRANSLATORS: Helptext for a barbarian worker: Master Blacksmith
         purpose = { pgettext(workerctxt, "Produces weapons for soldiers and tools for workers.") }
      },
      barbarians_brewer = {
         -- TRANSLATORS: Helptext for a barbarian worker: Brewer
         purpose = { pgettext(workerctxt, "Produces beer to keep the miners strong and happy.") }
      },
      barbarians_brewer_master = {
         -- TRANSLATORS: Helptext for a barbarian worker: Master Brewer
         purpose = { pgettext(workerctxt, "Produces the finest ales to keep soldiers strong and happy in training.") }
      },
      barbarians_builder = {
         -- TRANSLATORS: Helptext for a barbarian worker: Builder
         purpose = { pgettext(workerctxt, "Works at construction sites to raise new buildings.") }
      },
      barbarians_carrier = {
         -- TRANSLATORS: Helptext for a barbarian worker: Carrier
         purpose = { pgettext(workerctxt, "Carries items along your roads.") }
      },
      barbarians_cattlebreeder = {
         -- TRANSLATORS: Helptext for a barbarian worker: Cattle Breeder
         purpose = { pgettext(workerctxt, "Breeds strong oxen for adding them to the transportation system.") }
      },
      barbarians_charcoal_burner = {
         -- TRANSLATORS: Helptext for a barbarian worker: Charcoal Burner
         purpose = { pgettext(workerctxt, "Burns coal.") }
      },
      barbarians_farmer = {
         -- TRANSLATORS: Helptext for a barbarian worker: Farmer
         purpose = { pgettext(workerctxt, "Plants fields.") }
      },
      barbarians_ferry = {
         -- TRANSLATORS: Helptext for a barbarian worker: Ferry
         purpose = { pgettext(workerctxt, "Ships wares across narrow rivers.") }
      },
      barbarians_fisher = {
         -- TRANSLATORS: Helptext for a barbarian worker: Fisher
         purpose = { pgettext(workerctxt, "Catches fish in the sea.") }
      },
      barbarians_gamekeeper = {
         -- TRANSLATORS: Helptext for a barbarian worker: Gamekeeper
         purpose = { pgettext(workerctxt, "The gamekeeper makes sure that the natural population of animals never drops too low.") }
      },
      barbarians_gardener = {
         -- TRANSLATORS: Helptext for a barbarian worker: Gardener
         purpose = { pgettext(workerctxt, "Plants and harvests reed fields.") }
      },
      barbarians_geologist = {
         -- TRANSLATORS: Helptext for a barbarian worker: Geologist
         purpose = { pgettext(workerctxt, "Discovers resources for mining.") }
      },
      barbarians_helmsmith = {
         -- TRANSLATORS: Helptext for a barbarian worker: Helmsmith
         purpose = { pgettext(workerctxt, "Forges helmets for soldiers.") }
      },
      barbarians_hunter = {
         -- TRANSLATORS: Helptext for a barbarian worker: Hunter
         purpose = { pgettext(workerctxt, "The hunter brings fresh, raw meat to the colonists.") }
      },
      barbarians_innkeeper = {
         -- TRANSLATORS: Helptext for a barbarian worker: Innkeeper
         purpose = { pgettext(workerctxt, "Produces food for miners.") }
      },
      barbarians_lime_burner = {
         -- TRANSLATORS: Helptext for a barbarian worker: Lime-Burner
         purpose = { pgettext(workerctxt, "Mixes grout out of granite, water and coal in the lime kiln.") }
      },
      barbarians_lumberjack = {
         -- TRANSLATORS: Helptext for a barbarian worker: Helmsmith
         purpose = { pgettext(workerctxt, "Fells trees.") }
      },
      barbarians_miner = {
         -- TRANSLATORS: Helptext for a barbarian worker: Miner
         purpose = { pgettext(workerctxt, "Works deep in the mines to obtain coal, iron, gold or granite.") }
      },
      barbarians_miner_chief = {
         -- TRANSLATORS: Helptext for a barbarian worker: Chief Miner
         purpose = { pgettext(workerctxt, "Works deep in the mines to obtain coal, iron, gold or granite.") }
      },
      barbarians_miner_master = {
         -- TRANSLATORS: Helptext for a barbarian worker: Master Miner
         purpose = { pgettext(workerctxt, "Works deep in the mines to obtain coal, iron, gold or granite.") }
      },
      barbarians_ox = {
         -- TRANSLATORS: Helptext for a barbarian worker: Ox
         purpose = { pgettext(workerctxt, "Oxen help to carry items along busy roads. They are reared in a cattle farm.") }
      },
      barbarians_ranger = {
         -- TRANSLATORS: Helptext for a barbarian worker: Ranger
         purpose = { pgettext(workerctxt, "Plants trees.") }
      },
      barbarians_recruit = {
         -- TRANSLATORS: Helptext for a barbarian worker: Recruit
         purpose = { pgettext(workerctxt, "Eager to become a soldier and defend his tribe!") }
      },
      barbarians_scout = {
         -- TRANSLATORS: Helptext for a barbarian worker: Scout
         purpose = { pgettext(workerctxt, "Scouts like Scotty the scout scouting unscouted areas in a scouty fashion.") }
         -- (c) WiHack Team 02.01.2010
      },
      barbarians_shipwright = {
         -- TRANSLATORS: Helptext for a barbarian worker: Shipwright
         purpose = { pgettext(workerctxt, "Works at the shipyard and constructs new ships.") }
      },
      barbarians_smelter = {
         -- TRANSLATORS: Helptext for a barbarian worker: Smelter
         purpose = { pgettext(workerctxt, "Smelts ores into metal.") }
      },
      barbarians_soldier = {
         -- TRANSLATORS: Helptext for a barbarian worker: Soldier
         purpose = { pgettext(workerctxt, "Defend and Conquer!") }
      },
      barbarians_stonemason = {
         -- TRANSLATORS: Helptext for a barbarian worker: Stonemason
         purpose = { pgettext(workerctxt, "Cuts raw pieces of granite out of rocks in the vicinity.") }
      },
      barbarians_trainer = {
         -- TRANSLATORS: Helptext for a barbarian worker: Trainer
         purpose = { pgettext(workerctxt, "Trains the soldiers.") }
      },
      barbarians_weaver = {
         -- TRANSLATORS: Helptext for a barbarian worker: Weaver
         purpose = { pgettext(workerctxt, "Produces cloth for ships’ sails.") }
      }
   }
}
pop_textdomain()
return result
