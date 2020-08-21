-- Barbarian ware helptexts

local buildingctxt = "barbarians_building"
local immctxt = "barbarians_immovable"
local warectxt = "barbarians_ware"

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
            pgettext(buildingctxt, "Garrisons soldiers to expand your territory.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a barbarian military site: Barrier
            pgettext(buildingctxt, "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         }
      },
      barbarians_citadel = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian military site: Citadel
            pgettext(buildingctxt, "The Citadel of Adlen surely is the finest masterpiece of Barbarian craftsmanship. Nothing as strong and big and beautiful has ever been built in such a short time.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian military site: Citadel
            pgettext(buildingctxt, "Colintan, chief planner of the Citadel of Adlen,<br>at its opening ceremony")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian military site: Citadel
            pgettext(buildingctxt, "Garrisons soldiers to expand your territory.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a barbarian military site: Citadel
            pgettext(buildingctxt, "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
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
            pgettext(buildingctxt, "Garrisons soldiers to expand your territory.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a barbarian military site: Fortress
            pgettext(buildingctxt, "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
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
            pgettext(buildingctxt, "Garrisons soldiers to expand your territory.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a barbarian military site: Sentry
            pgettext(buildingctxt, "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
         },
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
            pgettext(buildingctxt, "Garrisons soldiers to expand your territory.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a barbarian military site: Tower
            pgettext(buildingctxt, "If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.")
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
            -- NOCOM this will break, we need fo fix push/pop_textdomain for this
            -- TRANSLATORS: Performance helptext for a barbarian production site: Ax Workshop
            pgettext(buildingctxt, "If all needed wares are delivered in time, this building can produce each type of ax in about %s on average."):bformat(ngettext("%d second", "%d seconds", 57):bformat(57)),
            -- TRANSLATORS: Performance helptext for a barbarian production site: Ax Workshop
            pgettext("barbarians_building", "All three weapons take the same time for making, but the required raw materials vary.")
         },
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
            pgettext(buildingctxt, "Produces beer to keep the miners strong and happy.")
         }
      },
      barbarians_cattlefarm = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian production site: Cattle Farm
            pgettext(buildingctxt, "‘The smart leader builds roads, while the really wise leader breeds cattle.’")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian production site: Cattle Farm
            pgettext(buildingctxt, "‘The Modern Barbarian Economy’,<br> 5ᵗʰ cowhide ‘Traffic and Logistics’")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian production site: Cattle Farm
            pgettext(buildingctxt, "Breeds strong oxen for adding them to the transportation system.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian production site: Cattle Farm
            pgettext(buildingctxt, "If all needed wares are delivered in time, this building can produce an ox in %s on average."):bformat(ngettext("%d second", "%d seconds", 30):bformat(30))
         },
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
            pgettext(buildingctxt, "Burns logs into charcoal.")
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
            pgettext(buildingctxt, "Digs coal out of the ground in mountain terrain.")
         },
         note = {
            -- TRANSLATORS: Note helptext for production site: Coal Mine
             pgettext("barbarians_building", "This mine exploits only %s of the resource. From there on out, it will only have a 5%% chance of finding any coal."):bformat("1/3")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for production site: Coal Mine
            pgettext(buildingctxt, "If the food supply is steady, this mine can produce coal in 32.5 seconds on average.")
         },
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
            pgettext(buildingctxt, "Digs coal out of the ground in mountain terrain.")
         },
         note = {
            -- TRANSLATORS: Note helptext for production site: Deep Coal Mine
            pgettext(buildingctxt, "This mine exploits only %s of the resource. From there on out, it will only have a 5%% chance of finding any coal."):bformat("2/3")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for production site: Deep Coal Mine
            pgettext(buildingctxt, "If the food supply is steady, this mine can produce coal in 19.5 seconds on average.")
         },
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
            pgettext(buildingctxt, "Digs coal out of the ground in mountain terrain.")
         },
         note = {
            -- TRANSLATORS: Note helptext for production site: Deeper Coal Mine
            pgettext(buildingctxt, "This mine exploits all of the resource down to the deepest level. But even after having done so, it will still have a %s chance of finding some more coal."):bformat("10%")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for production site: Deeper Coal Mine
            pgettext(buildingctxt, "If the food supply is steady, this mine can produce coal in 14.4 seconds on average.")
         },
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
         },
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
            pgettext(buildingctxt, "Hunts animals to produce meat.")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian production site: Hunter's Hut
            pgettext(buildingctxt, "The hunter’s hut needs animals to hunt within the work area.")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian production site: Hunter's Hut
            pgettext(buildingctxt, "The hunter pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 35):bformat(35))
         },
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
            pgettext(buildingctxt, "Prepares rations for scouts and rations and snacks to feed the miners in the basic and deep mines.")
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
            pgettext(buildingctxt, "Digs iron ore out of the ground in mountain terrain.")
         },
         note = {
            -- TRANSLATORS: Note helptext for a barbarian production site: Iron Mine
            pgettext(buildingctxt, "This mine exploits only %s of the resource. From there on out, it will only have a 5%% chance of finding any iron ore."):bformat("1/3")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian production site: Iron Mine
            pgettext(buildingctxt, "If the food supply is steady, this mine can produce iron ore in %s on average."):bformat(ngettext("%d second", "%d seconds", 65):bformat(65))
         },
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
            pgettext(buildingctxt, "Digs iron ore out of the ground in mountain terrain.")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian production site: Deep Iron Mine
            pgettext(buildingctxt, "This mine exploits only %s of the resource. From there on out, it will only have a 5%% chance of finding any iron ore."):bformat("2/3")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian production site: Deep Iron Mine
            pgettext(buildingctxt, "If the food supply is steady, this mine can produce iron ore in 39.5 seconds on average.")
         },
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
            pgettext(buildingctxt, "Digs iron ore out of the ground in mountain terrain.")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian production site: Deeper Iron Mine
            pgettext(buildingctxt, "This mine exploits all of the resource down to the deepest level. But even after having done so, it will still have a %s chance of finding some more iron ore."):bformat("10%")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian production site: Deeper Iron Mine
            pgettext(buildingctxt, "If the food supply is steady, this mine can produce iron ore in 17.6 seconds on average.")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },
      XXXX = {
         lore = {
            -- TRANSLATORS: Lore helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         lore_author = {
            -- TRANSLATORS: Lore author helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         purpose = {
            -- TRANSLATORS: Purpose helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },

         note = {
            -- TRANSLATORS: Note helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
         performance = {
            -- TRANSLATORS: Performance helptext for a barbarian building: XXXX
            pgettext(buildingctxt, "")
         },
      },

   },
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
