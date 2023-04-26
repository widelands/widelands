-- =======================================================================
--                 Texts for the Atlantean tutorial mission
-- =======================================================================

function jundlina(title, text)
   return speech("map:princess.png", styles.color("campaign_atl_jundlina"), title, text)
end
function loftomor(text)
   return speech("map:loftomor.png", styles.color("campaign_atl_loftomor"), "Loftomor", text)
end
function sidolus(text)
   return speech("map:sidolus.png", styles.color("campaign_atl_sidolus"), "Sidolus", text)
end
function colionder(text)
   return speech("map:colionder.png", styles.color("campaign_atl_colionder"), "Colionder", text)
end
function opol(text)
   return speech("map:opol.png", styles.color("campaign_atl_opol"), "Opol", text)
end
function ostur(text)
   return speech("map:ostur.png", styles.color("campaign_atl_ostur"), "Ostur", text)
end

-- =======================================================================
--                                Objectives
-- =======================================================================
obj_ensure_build_wares_production = {
   name = "obj_ensure_build_wares_production",
   title = _("Ensure the supply of building wares"),
   number = 6,
   body = objective_text(_("Supply Building Wares"),
      p(_([[Supply basic building material for your economy:]])) ..
      li(_([[Build a quarry]])) ..
      li(_([[Build two woodcutter’s houses]])) ..
      li(_([[Build two forester’s houses]])) ..
      li(_([[Build a sawmill]]))
   ),
}

obj_expand = {
   name = "obj_expand",
   title = _("Expand your territory and explore"),
   number = 1,
   body = objective_text(_("Expand and Explore"),
      p(_([[The island is huge and as long as we are not sure that we are alone here, we cannot relax. Explore and conquer it, this is the only way to protect us from threats on the island and from Atlantis.]])) ..
      li(_([[Build military sites to expand your territory]]))
   ),
}

obj_make_food_infrastructure = {
   name = "obj_make_food_infrastructure",
   title = _("Establish a solid food production"),
   number = 10,
   body = objective_text(_("Food Production"),
      p(_([[Food is very important for mines and military training areas. Establish a well working food environment by building at least one farm, one blackroot farm and a mill. The two kinds of flour together with water from a well will be baked into bread in a bakery, so build a bakery and a well, too.]])) ..
      p(_([[The other two important food wares are smoked fish and smoked meat. Raw meat is delivered from a hunter. A fisher gets the fish out of the sea while a fish breeder makes sure that a school of fish does not go extinct by breeding more. Make sure that there are always fish left, otherwise the fish breeder won’t be able to breed new ones. The smoking happens in a smokery, you will need at least two of those.]])) ..
      li(_([[Build a farm and a blackroot farm]])) ..
      li(_([[Build a mill to make cornmeal and blackroot flour]])) ..
      li(_([[Build a well]])) ..
      li(_([[Build a bakery to bake bread from cornmeal, blackroot flour and water]])) ..
      li(_([[Build a hunter’s house to get raw meat]])) ..
      li(_([[Build a fisher’s house close to water to get raw fish]])) ..
      li(_([[Build a fish breeder’s house close to the fisher to make sure the fish do not die out]])) ..
      li(_([[Build two smokeries to smoke raw meat and fish]]))
   )
}

obj_spidercloth_production = {
   name = "obj_spidercloth_production",
   title = _("Build a spider farm and a weaving mill"),
   number = 3,
   body = objective_text(_("Spidercloth Production"),
      p(_([[The weavers produce spidercloth and tabards in the weaving mill. Spidercloth is needed for the construction of some buildings and clothing, while tabards are the uniforms of soldiers. The weaving mill needs gold thread and spider silk as raw material. Spider silk is produced by the spider farm, while gold thread is produced by the gold spinning mill out of gold.]])) ..
      li(_([[Build a weaving mill]])) ..
      li(_([[Build a spider farm]])) ..
      li(_([[Build a gold spinning mill]]))
   )
}

obj_make_heavy_industry_and_mining = {
   name = "obj_make_heavy_industry_and_mining",
   title = _("Build industry and mines"),
   number = 5,
   body = objective_text(_("Industry and Mines"),
      p(_([[Iron ore, gold ore and coal are mined in the respective mines. The crystal mine will dig for quartz and diamonds – all of them are precious materials and very rarely found. It will produce a lot of granite while searching for them.]])) ..
      li_arrow(_([[The ores have to be smelted at a smelting works before they can be used. The refined materials are then used in the weapon smithy, the armor smithy and the toolsmithy.]])) ..
      li(_([[Build a mine of each type. Make sure to send geologists to the mountain first.]])) ..
      li(_([[Build a smelting works]])) ..
      li(_([[Build an armor smithy and a weapon smithy]])) ..
      li(_([[Build a toolsmithy]]))
   )
}

obj_build_barracks = {
   name = "obj_build_barracks",
   title = _("Build a barracks"),
   number = 1,
   body = objective_text(_("Recruiting new soldiers"),
      p(_([[Now that weapons are being forged, new soldiers can be recruited. For this, we need to deliver light tridents and tabards to the barracks. Recruits will arrive from the warehouse and receive one of each and some basic training to become a soldier.]])) ..
      li(_([[Build a barracks]]))
   )
}

obj_make_training_buildings = {
   name = "obj_make_training_buildings",
   title = _("Build training sites for soldiers"),
   number = 2,
   body = objective_text(_("Soldier Training Sites"),
      p(_([[Like all other tribes, the Atlanteans are also able to train soldiers: the dungeon trains attack – the major attribute of the Atlanteans – and the labyrinth trains evasion, health and defense. The items produced by the industry are used to train better soldiers in the two training sites.]])) ..
      li(_([[Build a dungeon and a labyrinth]]))
 )
}

obj_horsefarm_and_warehouse = {
   name = "obj_horsefarm_and_warehouse",
   title = _("Build a warehouse and a horse farm"),
   number = 2,
   body = objective_text(_("Warehouse and Horse Farm"),
      p(_([[As your road network gets longer and more complicated, you should employ horses to help out your carriers. Horses are bred at horse farms using water and corn. A warehouse will also help to ensure that your transportation system does not collapse.]])) ..
      li(_([[Build a warehouse]])) ..
      li(_([[Build a horse farm]]))
  )
}

obj_build_ships = {
   name = "obj_build_ships",
   title = _("Build 3 ships to escape from the island"),
   number = 1,
   body = objective_text(_("Escape From the Island"),
      p(_([[There is a lake at the top of the island. Build three ships in these waters and you might be able to rescue your people before the island is swallowed completely by the ocean.]])) ..
      li(_([[Build a shipyard close to the lake to start building ships]]))
   )
}

-- =======================================================================
--                                  Texts
-- =======================================================================

-- This is an array or message descriptions
initial_messages = {
   {
      title = _("Proud to the Death"),
      body = (
         h1(_("Favored by the God")) ..
         -- TRANSLATORS: Foreword
         p(_([[On the hidden and lost island of Atlantis, a proud tribe settled since the world was very young. Ruled by the bloodline of King Ajanthul – the first human to be empowered by the sea god Lutas to breathe above sea level – and the wise clerics, who provided the link to Lutas – they prospered and became civilized.]])) ..
         -- TRANSLATORS: Foreword
         p(_([[This story is taking place during the reign of King Askandor, the 43rd successor of King Ajanthul. He had been a good king, ruling Atlantis with wisdom and foresight. But with age, he became afraid of dying and so he began looking for a cure for death even though most clerics warned him against it. Some said that endless life was only for the gods and that to seek for it was forbidden.]]))
      )
   },
   {
      title = _("Disgraced Before the God"),
      body = (
         h1(_("The God’s Punishment")) ..
         -- TRANSLATORS: Foreword
         p(_([[But all seemed well. Only the horses seemed to feel something was wrong. During the nights, they went crazy and were full of fear. It was not long before the horse breeder Xydra figured out what was wrong with them: the sea level in front of their stable was rising with ever increasing speed.]])) ..
         -- TRANSLATORS: Foreword
         p(_([[The clerics retreated into meditation and the reason for the rising water was soon to be found: the god Lutas had lost faith in the Atlanteans because of the boldness of their king. He decided to withdraw the rights that were granted to King Ajanthul and his children. And so, he called them back below the sea again.]]))
      )
   },
   {
      title = _("Uproar and Confusion"),
      body = (
         h1(_("Emerging Chaos…")) ..
         -- TRANSLATORS: Foreword
         p(_([[Guilt-ridden, the king committed suicide. Without a monarch, the people turned to the clerics, but they had no substantial help to offer. Most accepted their fate while others tried to change the god’s mind by offering animals in his temple. But to no avail…]])) ..
         -- TRANSLATORS: Foreword
          p(_([[Jundlina, the late king’s daughter and the highest priestess of the god was the most determined cleric. As countless offerings didn’t change the situation, she convinced herself that to soothe the god, an offer of great personal value was needed. So she offered him her most beloved: her husband, the father of her only child.]])) ..
          -- TRANSLATORS: Foreword
          p(_([[But not even this changed the mind of the god. The water kept on rising. Nearly driven insane by guilt, pain and anger, Jundlina became a heretic: Secretly, she gathered people of the common folk who were not in line with the decision of the clerics to accept the god’s will. Together with them, she set the temple on fire and stole a ship to flee from the god’s influence over Atlantis. This small group started praying to Satul, the fire god and the worst enemy of Lutas.]])) ..
          -- TRANSLATORS: Foreword
          p(_([[Leaving the dying Atlantis and their past behind, they started on a quest to find a place sheltered by the fire and protected from the sea.]]))
      )
   },
} -- end of initial messages.

first_briefing_messages = {
   {
      title = _("The Princess’ Memoir"),
      body = jundlina(_("Jundlina Writes Down Her Memories"),
         -- TRANSLATORS: Jundlina - Diary
         _([[We left Atlantis and sailed east. We entered the forbidden sea on the sixth day without noticing any pursuers from Atlantis and without Lutas having smashed our ship. Now, we are out of his reach. One day later, we sighted an island which seems to have one of these fire spitting mountains on it. I deemed this a sign from the fire god and we landed on its shore.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Jundlina - Diary
         _([[We spent the last week building two watchtowers on the mountains close to our landing area; and, of course, a hall for us all. We have very talented constructors in our group – still, the buildings do not match the art we had on Atlantis. I hope they will withstand the next rain. At least, the towers will warn us if a ship from Atlantis follows us and if the island is inhabited, we will see attackers a long time before they arrive.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Jundlina - Diary
         _([[We have established ourselves on this island. The next step is now to make it a home. I reckon we need to establish a sustainable economy and to explore our surroundings. I called for specialists and will follow their advice.]]))
   },
   {
      title = _("Loftomor and Sidolus Arrive"),
      body = jundlina( _("Jundlina"),
         -- TRANSLATORS: Jundlina
         _([[May Satul warm you both. Loftomor, you have been the island’s most renowned architect. Sidolus, you are a seasoned warrior and strategist. I have called you before me to seek your counsel: what needs to be done to make this our new home?]]))
   },
   {
      title = _("Loftomor Speaks"),
      body = loftomor(
         -- TRANSLATORS: Loftomor
         _([[May Satul warm you, Jundlina! The most important things for building a settlement are the building materials. There are some trees here, so we should build housings for some woodcutters and of course also for foresters, so we will not run out of trees. Oh, and we mustn’t forget the sawmill, for most buildings can’t be made out of logs alone. Stronger buildings also need granite, but there is plenty to the north-east of here; we just need to build a quarry, and my stonemasons will go to work promptly.]]))
         .. new_objectives(obj_ensure_build_wares_production)
   },
   {
      title = _("Sidolus Speaks"),
      body = sidolus(
         -- TRANSLATORS: Sidolus
         _([[May Satul warm you, Jundlina! I agree with what Loftomor proposes. We need a good supply of building materials for we have to expand our territory swiftly. I will not feel safe on this island as long as we have not seen all its shores. I brought plenty of good men from Atlantis. The military might is available. We only need some housing to live in.]]))
         ..  new_objectives(obj_expand)
   }
}

food_story_message = {
   {
      title = _("Jundlina is Satisfied"),
      body = jundlina(_("Jundlina’s Memoirs"),
         -- TRANSLATORS: Jundlina
         _([[Our building infrastructure is finished and I spent some days making sure that everybody is working well together. Now, there are other pressing matters. I called Colionder, my personal cook, before me to get his help with sorting some things out.]]))
   },
   {
      title = _("Colionder Arrives"),
      body = colionder(
         -- TRANSLATORS: Colionder
         _([[May Satul warm you, Jundlina. Can I be of any service to you or the god?]]))
   },
   {
      title = _("Jundlina Replies"),
      body = jundlina(_("Jundlina"),
         -- TRANSLATORS: Jundlina
         _([[May Satul warm you too, Colionder. Yes, I wondered if there is a way that we could centralize our food production. We lose a lot of productivity because our people are accustomed to making their own food. I feel if we could split responsibilities, it would be better for us all. I want your thoughts on this.]]))
   },
   {
      title = _("Colionder is in Thought"),
      body = colionder(
         -- TRANSLATORS: Colionder
         _([[Ahh, but I think this is impossible to change. The preparing and eating of food is something deeply ingrained in us Atlanteans – it is a ceremony that we just need for our well-being. So I guess we cannot take this away from the individuals completely.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Colionder
         _([[But we might find a compromise in between: for me, making bread is a troublesome task. Grinding the blackroots and corn to flour and then baking the bread is tedious and boring; I feel a more industrial approach would be helpful here. I for one would love to just have fresh bread delivered to my house every day.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Colionder
         _([[Oooh and even more important: the smoking of fish and meat to cleanse them and improve their taste is terrible. My house is full of smoke and stinks for weeks afterwards. Don’t you think that this could be done in a special building where the side effects do not matter? I think those two things would be accepted by the people and would reduce the cooking time without taking away the ritual.]]))
   },
   {
      title = _("Jundlina Agrees"),
      body = jundlina(_("Jundlina Agrees"),
         -- TRANSLATORS: Jundlina
         _([[Your words sound wise to me, Colionder. So be it then.]]))
         .. new_objectives(obj_make_food_infrastructure)
   },
}

food_story_ended_messages = {
   {
      title = _("Jundlina is Satisfied"),
      body = jundlina(_("Jundlina is Reviewing the Reports"),
         -- TRANSLATORS: Jundlina
         _([[Seems like our food production is finished and everybody is working together nicely. It is really starting to feel like home here.]]))
   }
}

spidercloth_messages = {
   {
      title = _("A Man Comes to Jundlina"),
      body = opol(
         -- TRANSLATORS: Opol
         _([[May Satul warm you, Jundlina. My name is Opol, and I am the highest weaver of the guild abandoning Atlantis and Lutas with you. I come with sad news indeed: we have no more spidercloth. Not a single piece is to be found in our warehouses. Could you not help the weavers’ guild by arranging the building of a weaving mill and a spider farm? The spiders deliver the finest silk and we will produce the finest spidercloth from it.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Opol
         _([[We offer to also produce the tabards for young soldiers and the golden tabards for officers for you in exchange. You will need them for sure as soon as you want to recruit new soldiers.]]))
   },
   {
       title = _("Jundlina Replies"),
       body = jundlina(_("Jundlina"),
         -- TRANSLATORS: Jundlina
         _([[May Satul warm you too, Opol. I wanted to delay production of spidercloth, but I understand your urgency. Your suggestion sounds fair to me, I will build your weaving mill and spider farm. I will also build a gold spinning mill so that the golden tabards you make will not be golden by name alone.]]))
         ..  new_objectives(obj_spidercloth_production)
   }
}

spidercloth_story_ended_messages = {
   {
      title = _("Opol Seeks Out Jundlina"),
      body = opol(
         -- TRANSLATORS: Opol
         _([[May Satul warm you, Jundlina! The weaving mill, gold spinning mill and spider farm buildings are complete and are starting their work at this very moment. The weavers’ guild is very grateful for your support and we will stand by our word and deliver the first tabard very soon.]]))
   },
   {
      title = _("Jundlina Replies"),
      body = jundlina(_("Jundlina"),
         -- TRANSLATORS: Jundlina
         _([[This is good news indeed, Opol. Our economy is developing very well. Go back to your work now, Opol, and may Satul warm you and all your guild members!]]))
   }
}


heavy_industry_story = {
   {
      title = _("Sidolus Seeks Out Jundlina"),
      body = sidolus(
         -- TRANSLATORS: Sidolus
         _([[May Satul warm you, Jundlina. Have you considered mining for resources in the mountains around us? We are running short of quartz and diamonds for our buildings. Also, we could use more iron, gold and especially coal. If we find an ample supply of those commodities, we could start producing weapons and tools; this would help enormously. Of course, we would need smelters, a weapon smithy, an armor smithy and a toolsmithy. And maybe even more.]]))
   },
   {
      title = _("Jundlina Replies"),
      body = jundlina(_("Jundlina Nods"),
         -- TRANSLATORS: Jundlina
         _([[I have considered this, Sidolus. I think it is about time. We will make this a priority for now. You shall get your industry soon!]]))
         .. new_objectives(obj_make_heavy_industry_and_mining)
   },
}

hint_for_toolsmith = {
   {
      title = _("Loftomor Reports"),
      body = loftomor(
         -- TRANSLATORS: Loftomor
         _([[May Satul warm you, Jundlina! We have completed a toolsmithy and now our toolsmiths will produce new tools in an instant.]]))
   },
   {
      title = _("Jundlina Reminds"),
      body = jundlina(_("Jundlina"),
         -- TRANSLATORS: Jundlina
         _([[May Satul warm you as well, Loftomor. That is good news to hear. However please remember that our toolsmiths need to be equipped with a saw. Making tools is an art for us, requiring a precise tool like a saw, not a brutal hammer like for the other tribes.]]))
   }
}

barracks_story = {
   {
      title = _("Jundlina Writes a Letter"),
      body = jundlina(_("Jundlina"),
         -- TRANSLATORS: Jundlina
         _([[I have received word that our mining industry started working. Now that we are forging weapons, I will send a letter to Sidolus informing him that it’s time to build a barracks and start recruiting soldiers.]]))
         .. new_objectives(obj_build_barracks)
   }
}

barracks_story_end = {
   {
      title = _("Jundlina is Satisfied"),
      body = jundlina(_("Jundlina"),
         -- TRANSLATORS: Jundlina
         _([[Today I saw the first recruits gathering in front of the new barracks. If everything goes well, they should receive their training soon.]]))
   }
}

training_story = {
   {
      title = _("Jundlina Summons Sidolus"),
      body = jundlina(_("Jundlina is Angry"),
         -- TRANSLATORS: Jundlina
         _([[What is this I hear, Sidolus? I received words about your soldiers misbehaving and bullying the common folk. What is this nonsense all about? They are servants for the common good and must not misuse their powers. This is inexcusable! Explain this, if you will!]]))
   },
   {
      title = _("Sidolus Seems Contrite"),
      body = sidolus(
         -- TRANSLATORS: Sidolus
         _([[I agree with you and I have punished the troublemakers severely. But it is very difficult to keep my men quiet: they are bored and have nothing to do. I would like to ramp up their drill. I think more routine in their life would keep them occupied and focused on their duties. I would need a dungeon and a labyrinth for them. I have the plans ready, but I guess they won’t come cheap.]]))
   },
   {
      title = _("Jundlina Replies"),
      body = jundlina(_("Jundlina"),
         -- TRANSLATORS: Jundlina
         _([[I do not fear the cost and labor as long as it keeps your people at bay. You shall get your buildings. But note that I will not tolerate any rogue soldier; if similar things happen again, I will make you responsible for them. Dismissed.]]))
         .. new_objectives(obj_make_training_buildings)
   }
}

training_story_end = {
   {
      title = _("Jundlina Received a Letter"),
      body = jundlina(_("Jundlina"),
         -- TRANSLATORS: Jundlina
         _([[I received an invitation from Sidolus. The first day of training in the newly finished labyrinth will be tomorrow. The dungeon is done as well and will start its work soon. I am looking forward to tomorrow; I am very interested in how our soldiers are trained, and I wonder if the additional workload will keep them from bullying the civilian population.]]))
   }
}

horsefarm_and_warehouse_story = {
   {
      title = _("Jundlina Considers the Economy"),
      body = jundlina(_("Jundlina"),
         -- TRANSLATORS: Jundlina
         _([[People are complaining about crowded streets and slow transportation. We need to help out our carriers on the roads. I have decided to build a horse farm so that the horses can help with the heavy wares.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Jundlina
         _([[Another way to take load from our roads is to build warehouses. We have claimed the mountain now, it seems a good idea to have a warehouse on the plateau to avoid having to transport everything up and down the slopes.]]))
         .. new_objectives(obj_horsefarm_and_warehouse)
   }
}

-- =======================================================================
--                         Leftover buildings found
-- =======================================================================
first_leftover_building_found = {
   {
      posx = 100000,
      posy = 0,
      title = _("A Dangerous Discovery"),
      body = jundlina(_("Jundlina is in Thought"),
         -- TRANSLATORS: Jundlina
         _([[We found an old building, destroyed and burned by flames. I am very worried about this discovery. The building is not of any kind I’ve ever seen. It is certainly not designed by any Atlantean architect I’ve ever heard about. The building is crude, the assemblage is sloppy. But the materials are very enduring: the wood used for it seemed to have been burned in a strange way before it was used as building material – it is hard as stone.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Jundlina
         _([[The implications frighten me. Are there others on this island? Where are they then? They work with fire, are they praying to Satul too? Are they friends or foe? And why is the building burned down? Has this island seen war? Is a war being waged on it right now?]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Jundlina
         _([[We must improve our military capabilities. If there are foes on the island, we have to be prepared when we meet them. We should also enforce the fortification of our borders.]]))
   }
}

second_leftover_building_found = {
   {
      posx = 100000,
      posy = 0,
      title = _("Another Building"),
      body = jundlina(_("Jundlina is Worried"),
         -- TRANSLATORS: Jundlina
         _([[We found one more building of this strange making. We have yet to see any of its builders. But obviously there has been a community on this island before and obviously it was militarized. I have to check on our military status with my generals. I will not allow that a potential enemy will find us unprepared.]]))
   }
}

third_leftover_building_found = {
   {
      posx = 100000,
      posy = 0,
      title = _("One More Building"),
      body = jundlina(_("Jundlina is in Thought"),
         -- TRANSLATORS: Jundlina
         _([[And yet another of these buildings. We know now that the tribe that built them once had complete control over the island. But I am no longer so scared of potential enemies. We know so much of this island already and there is no trace of this old economy to be found. I am starting to relax, but I will not let our guard down as we continue to explore this beautiful island. But without obvious dangers, I am feeling very much at home here already.]]))
   }
}

-- =======================
-- Flooding of the island
-- =======================
field_flooded_msg = {
   {
      posx = 100000,
      posy = 0,
      title = _("The Water is Rising!"),
      body = jundlina(_("Jundlina"),
         -- TRANSLATORS: Jundlina
         _([[May Satul save us! Lutas is still trying to get to us. A scout informed me that the ocean is rising quickly. We have to reach higher ground and evacuate everybody and as much wares as we can from the headquarters up the mountains. How can we escape?]]))
   },
   {
      title = _("A Young Man Approaches"),
      body = ostur(
         -- TRANSLATORS: Ostur
         _([[May Satul warm you, Jundlina. My name is Ostur and I construct ships. I have invented a new kind of ship: smaller than those we are used to, but much sturdier. If we build them correctly, I am confident that we can sail a much longer distance with them and maybe escape from Lutas’ influence.]]))
   },
   {
      title = _("Jundlina Replies"),
      body = jundlina(_("Jundlina"),
         -- TRANSLATORS: Jundlina
         _([[Ostur, we have no time. The water rises too fast, and if we build the ships on the shore they will vanish in the sea before they can float. I fear we are doomed!]]))
   },
   {
      posx = 100000,
      posy = 0,
      title = _("Ostur Seems Confident"),
      pre_func = function()
         local lake_field = map:get_field(75,80)
         p1:reveal_fields(lake_field:region(10))
         scroll_to_field(lake_field)
         sleep(200)
      end,
      body = ostur(
         -- TRANSLATORS: Ostur
         _([[A friend of mine explored the island on his own and he told me of a big lake at the top of the mountain. I suggest we build the ships in this lake and wait until the island drowns beneath us. Then we can sail away. It is a bold plan, but I think it could work.]]))
   },
   {
      title = _("Jundlina Replies"),
      body = jundlina(_("Jundlina"),
         -- TRANSLATORS: Jundlina
         _([[Your red hair is a signal from the fire god. I will trust you and support your plan. Our survival lies in your hands now, Ostur.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Jundlina
         _([[Swift now, we need a house for Ostur next to the lake, and we need planks, logs and spidercloth for the construction there. Forget about everything else, we need those wares on top of the mountain before our warehouses are all swallowed by the sea.]]))
         .. new_objectives(obj_build_ships)
   }
}

scenario_won = {
   {
      allow_next_scenario = true,
      title = _("Jundlina is Relieved"),
      body = jundlina(_("Jundlina"),
         -- TRANSLATORS: Jundlina
         _([[Praise Satul! Ostur, the young ship builder did it. We have three ships – never have I seen sturdier ones – with enough room to carry all of us and some wares too. And this rescue came just in time: Lutas is about to swallow the rest of this island, the water rises faster by the hour. But we can make our escape now and start over in some country farther away…]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Jundlina
         _([[I expect a long journey, but we will find the land of Satul in the end. This is what I promised my people. And myself.]]))
         .. objective_text(_("Congratulations"),
      _([[You have completed this mission. You may continue playing if you wish, otherwise move on to the next mission.]]))
   }
}

function building_lost(buildingname)
   push_textdomain("scenario_atl01.wmf")
   local r = {
      -- TRANSLATORS: Short message title. Translate as "Lost!" if you don't have enough space.
      title = pgettext("message_short_title", "Building lost!"),
      text = li_object(buildingname, _("We lost a building to the ocean!"))
   }
   pop_textdomain()
   return r
end
