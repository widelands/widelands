-- =======================================================================
--                 Texts for the Atlantean Mission 2
-- =======================================================================

include "scripting/richtext_scenarios.lua"

function jundlina(title, text)
   return speech("map:princess.png", "2F9131", title, text)
end
function loftomor(text)
   return speech("map:loftomor.png", "FDD53D", "Loftomor", text)
end
function sidolus(text)
   return speech("map:sidolus.png", "FF1A30", "Sidolus", text)
end
function colionder(text)
   return speech("map:colionder.png", "33A9FD", "Colionder", text)
end
function opol(text)
   return speech("map:opol.png", "FF3FFB", "Opol", text)
end
function ostur(text)
   return speech("map:ostur.png", "375FFC", "Ostur", text)
end
function kalitath(text)
   return speech("map:kalitath.png", "FF0000", "Kalitath", text)
end


-- =======================================================================
--                                Objectives
-- =======================================================================

obj_find_port = {
   name = "obj_find_port",
   title = _ "Find a place to build a port",
   number = 1,
   body = objective_text(_"Find a portspace",
   p(_[[To set foot on this island, we must first find a building space where a port can be built. Explore the nearby coast line to find such a building plot.]]) ..
   p(_[[After the port is built we should cancel the other ship’s expedition, to unload its wares as well.]]))
}

obj_basic_infrastructure = {
   name = "obj_basic_infrastructure",
   title = _ "Ensure the supply of build wares",
   number = 4,
   body = objective_text(_"Establish Basic Production Chain",
   p(_[[We will need at least a quarry, a few woodcutters, and a sawmill. A forester might or might not be needed depending on how fast the trees are growing around you. Find the balance.]]))
}

obj_tools = {
   name = "obj_tools",
   title = _ "Build tools quickly",
   number = 1,
   body = objective_text(_"Make New Tools",
      p(_[[Most of our tools have been lost in the ship tragedy. Our people will not be able to fulfill their duties without proper tools. Luckily we still have some iron ore and plenty of coal. Build a production of tools, as it is a prerequisite for success on the island. Don’t forget to build a smelting works as well.]]))
}

obj_mining = {
   name = "obj_mining",
   title = _ "Search the mountains for resources",
   number = 3,
   body = objective_text(_"Build Mines",
      p(_[[As we are able to make tools again, our supply of mining resources needs to be replenished soon. Explore the mountains and build at least one mine of each type to mine coal, iron and gold.]]))
}

obj_explore = {
   name = "obj_explore",
   title = _"Explore the surroundings",
   number = 1,
   body = objective_text(_"Explore Your Surroundings",
   p(_[[We are fairly sure that we are not alone on the island. We do not know if the others on this island are friends or foes, so we need to tread carefully. Explore the island by building at least one habitat for a scout.]]))
}

obj_expand = {
   name = "obj_explore",
   title = _"Expand your teritory",
   number = 1,
   body = objective_text(_"Expand Territory",
   p(_[[As our settlement is going to grow up, we will need more space for all buildings. Expand our territory by building military buildings. Build at least one tower for its larger view range.]]))
}

obj_spidercloth = {
   name = "obj_spidercloth_production",
   title = _"Build a spider farm and a weaving mill",
   number = 2,
   body = objective_text(_"Spidercloth Production",
      p(_[[We need some spidercloth urgently as we are running out of it. It is therefore essential to spare every piece we still have. Reduce the input of all buildings that consume spidercloth, not forgetting the construction sites.]]) ..
      p(_[[Afterwards we need to construct a farm under all circumstances, as it needs spidercloth to be built. Additionally, we need a spiderfarm and a weaving mill. Later on, a gold spinning mill would be helpful as well.]]))
}

obj_trading_post = {
   name = "obj_trading_post",
   title = _"Build a trading post",
   number = 1,
   body = objective_text(_"Upgrade Scout’s House To Trading Post",
   p(_[[To be able to pay the tributes to Kalitath, and to maintain contact with Jundlina, we need to enhance our scout’s house to a trading post.]]))
}

obj_tribute = {
   name = "obj_tribute",
   title = _"Pay all tributes to Kalitath",
   number = 1,
   body = objective_text(_"Send Trader To Kalitath",
   p(_[[We need to send the wares to Kalitath with our trader. He will pick them up as soon as a batch is ready in his trading post.]]) ..
   p(_[[The following batches need to be delivered:]]) ..
   li(_[[Five batches of five logs each.]]) ..
   li(_[[Four batches of four lumps of coal each.]]) ..
   li(_[[Three batches of three iron bars each.]]) ..
   li(_[[Two batches of two gold bars each.]]))
}

obj_tribute2 = {
   name = "obj_tribute2",
   title = _"Pay all tributes to Kalitath",
   number = 1,
   body = objective_text(_"Send Trader To Kalitath",
   p(_[[We need to send the wares to Kalitath with our trader. He will pick them up as soon as a batch is ready in his trading post.]]) ..
   p(_[[The following batches need to be delivered:]]) ..
   li(_[[Ten batches of five logs each.]]) ..
   li(_[[Eight batches of four lumps of coal each.]]) ..
   li(_[[Six batches of three iron bars each.]]) ..
   li(_[[Four batches of two gold bars each.]]))
}

obj_defeat_maletus = {
   name = "obj_defeat_maletus",
   title = _"Defeat Maletus",
   number = 1,
   body = objective_text(_"Defeat Imperial Leader Maletus",
   p(_[[We need to finish the Empire off to have a chance to build a new home.]]))
}

-- =======================================================================
--                                  Texts
-- =======================================================================

defeated = {
   {
      title =_"Defeated!",
      body = sidolus(_"Sidolus has bad news…",
         -- TRANSLATORS: Sidolus
         _([[Aaaargh, Jundlina. I don’t know how this could have happened, but Satul couldn’t protect us from the enemy. They have destroyed our last warehouse. Thus we have lost this battle and our people is doomed!]])),
      posy=1,
   }
}

-- This is an array or message descriptions
initial_messages = {
   {
      title = _"The princess’s memoir",
      body = jundlina(_"Jundlina writes her diary",
      -- TRANSLATORS: Jundlina - Diary
      _([[Our escape from the cursed island was a close one: we managed to load three ships and make our escape, but the currents of the sinking island tested our new ships and their design. ]]) ..
      _([[And one failed the test: the last days of loading was done in chaos and without much planning and so one of our ships ended up carrying most of the heavy wares in our possession. ]]) ..
      _([[The heavy weight plus the strong current was too much for the ship and it was pulled below the sea level by Lutas forces and we lost most men and all wares it carried.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Jundlina - Diary
      _([[All of us were fiercely afraid. We expected the other ships to be drowned as well, but they persisted. We escaped the island and began our smooth sailing to other coasts. ]]) ..
      _([[Many weeks have passed and we did only see small islands not suitable for habitation and without any sign of being blessed by Satul, the fire god. ]]) ..
      _([[Last night however, we made out a new island. We saw smoke hovering over it. The black comes from the fiery mountain in the north of the island, the grey however comes from settlers.]])),
      h=500,
   },
   {
      title = _"Planning for the future",
      body = jundlina(_"Jundlina continues",
      -- TRANSLATORS: Jundlina - Diary
      _([[At least the east side of the island is inhabited, that much is certain. We do not know if the people there are friends or foes. We do not even know what tribe they might be from. ]]) ..
      _([[The only thing we know for sure is that they cannot be Atlanteans, for no ship of ours has sailed as far as we did for centuries. I can only hope they are friendly. ]]) ..
      _([[We have not much water left; our food is running low; we lost all our tools and most of our building materials with the third ship. We can’t afford to sail any further, and we need to try to settle on this island.]]))
      .. new_objectives(obj_find_port),
      h=500,
   },
} -- end of initial messages.

initial_messages1= {
   {
      title = _"Jundlina gives orders",
      body = jundlina(_"Jundlina decides",
      -- TRANSLATORS: Jundlina - Diary
      _([[As we founded a port here, we can now plan our settlement. We still have plenty of iron ore, so a toolsmithy and a smelting works will provide us with good tools in no time. However, we need to take care of our building materials: ]]) ..
      _([[The island is full of trees, we might not even need foresters. But we need stone, that much is certain!]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Jundlina - Diary
      _([[Let’s also explore this island and expand our territory. But we need to be careful. We should build at least one tower so that we can see potential enemies before they see us. ]]) ..
      _([[But we need to be careful with the stones and other precious material. When we no longer need a building, we will dismantle it instead of burning it down. ]]) ..
      _([[This will take more time, but we can reuse some of its materials. Also, I want a scout out and exploring at all times.]]))
      .. new_objectives(obj_tools)
      .. new_objectives(obj_basic_infrastructure)
      .. new_objectives(obj_explore)
      .. new_objectives(obj_expand),
      h=500,
   },
} -- end of initial messages.

spidercloth_1 = {
   {
      title = _"Opol visits Jundlina",
      body = opol(
         -- TRANSLATORS: Opol
         _([[May Satul warm you, Jundlina. As the highest weaver of the guild I have bad news for you. I just realized that we are running out of spidercloth. This is a most perilous case, for as you know, spidercloth is needed for many buildings.]])
         .. paragraphdivider() ..
         -- TRANSLATORS: Opol
         _([[We should build up a spidercloth production urgently.]]))
   },
   {
       title = _"Jundlina Replies",
       body = jundlina(_"Jundlina",
         -- TRANSLATORS: Jundlina
         _([[May Satul warm you too, Opol. I wanted to delay production of spidercloth, but I understand the urgency. We need to build a complete spidercloth production instantly.]]))
         .. new_objectives(obj_spidercloth_production)
   }
}

spidercloth_2 = {
   {
      title = _"Opol makes a point",
      body = opol(
         -- TRANSLATORS: Opol
         _([[Jundlina! It seems there is another problem even more urgent. I fear we haven’t built a farm to feed our spiders yet. As this is one of the buildings needing spidercloth for completion, not completing one in time will result in a crisis that can never be resolved.]]))
   },
   {
      title = _"Jundlina Acknowledges",
      body = jundlina(_"Jundlina",
         -- TRANSLATORS: Jundlina
         _([[I see the risk, Opol. Thank you for reporting. We will build a farm immediately now.]]))
   }
}

spidercloth_3 = {
   {
      title = _"Opol Seeks Out Jundlina",
      body = opol(
         -- TRANSLATORS: Opol
         _([[Jundlina, may Satul warm you! The weaving mill and spider farm buildings are complete and the weavers’ guild can start their work again. I promise we will deliver the finest cloth and some tabards soon. Maybe we can build a gold spinning mill later on to make better tabards as well.]]))
   },
   {
      title = _"Jundlina Cheers",
      body = jundlina(_"Jundlina",
         -- TRANSLATORS: Jundlina
         _([[This is good news indeed, Opol. You may go back to your work now, and may Satul warm you and the weavers’ guild!]]))
   }
}

enemy_1 = {
   {
      title = _"Sidolus Reports foreign inhabitants",
      body = sidolus(
         -- TRANSLATORS: Sidolus
         _([[May Satul warm you, Jundlina! I have some disturbing news. We have seen some foreign reigned land, But we don’t know what to expect from the foreigners.]]))
   },
   {
      title = _"Jundlina Cheers",
      body = jundlina(_"Jundlina",
         -- TRANSLATORS: Jundlina
         _([[Sidolus, I agree this is very concerning. We need to find out more. Hopefully our scout will get us some information soon.]]))
   }
}

enemy_1a = {
   {
      title = _"Sidolus Reports foreign inhabitants",
      body = sidolus(
         -- TRANSLATORS: Sidolus
         _([[May Satul warm you, Jundlina! I have some disturbing news. We have seen some foreign reigned land, But we don’t know what to expect from the foreigners.]]))
   },
   {
      title = _"Jundlina Cheers",
      body = jundlina(_"Jundlina",
         -- TRANSLATORS: Jundlina
         _([[Sidolus, I agree this is very concerning. We need to find out more. We definitly need to send out a scout to gather some information.]]))
   }
}

allies = {
   {
      title = _"Allies?",
      body = sidolus(
         -- TRANSLATORS: Sidolus
         _([[May Satul warm you, Jundlina! We have made contact with the foreigners. They are in big trouble as they are in war with a mighty tribe in the north. They ask us for support.]]))
   },
   {
      title = _"Jundlina Cheers",
      body = jundlina(_"Jundlina",
         -- TRANSLATORS: Jundlina
         _([[Sidolus, as we could need any ally we can get we should help them. Let’s see what we can do for them. However we should improve our own military strength to become independent of others.]]))
   }
}

infrastructure_1 = {
   {
      title = _"Building material ensured",
      body = loftomor(
         -- TRANSLATORS: loftomor
         _([[May Satul warm you, Jundlina! We have finished our supply of building materials.]]))
   },
   {
      title = _"Jundlina Cheers",
      body = jundlina(_"Jundlina",
         -- TRANSLATORS: Jundlina
         _([[This is good news indeed, Loftomor. Now we can think of constructing more sophisticated buildings to make our economy grow. Please advice if you feel we are missing necessary buildings.]]))
   }
}

tools_available = {
   {
      title = _"Tool Supply Ensured",
      body = loftomor(
         -- TRANSLATORS: loftomor
         _([[Jundlina, may Satul warm you! We have made some progress in building up our tool industry.]]))
   },
   {
      title = _"Jundlina Enchanted",
      body = jundlina(_"Jundlina",
         -- TRANSLATORS: Jundlina
         _([[This is good news indeed, Loftomor. This enables us to operate all the buildings we are going to build in our new home. But now we should really look to get some resources from the mountains.]]))
         .. new_objectives(obj_mining)
   }
}

mining_established_1 = {
   {
      title = _"Mining Established",
      body = loftomor(
         -- TRANSLATORS: loftomor
         _([[Jundlina, may Satul warm you! Another issue solved. We have found some resources and build a mine of each type.]]))
   },
   {
      title = _"Jundlina is Excited",
      body = jundlina(_"Jundlina",
         -- TRANSLATORS: Jundlina
         _([[Well done, Loftomor, well done. May Satul warm our strong and brave miners. But now we need to ensure we can feed them. We need to built up bakeries and smokeries and all buildings to supply them.]]))
   }
}

mining_established_2 = {
   {
      title = _"Mining Established",
      body = loftomor(
         -- TRANSLATORS: loftomor
         _([[Sidolus, may Satul warm you! Another issue solved. We have found some resources and build a mine of each type.]]))
   },
   {
      title = _"Sidolus is Excited",
      body = sidolus(
         -- TRANSLATORS: Jundlina
         _([[Well done, Loftomor, well done. May Satul warm our strong and brave miners. But now we need to ensure we can feed them. We need to built up bakeries and smokeries and all buildings to supply them.]]))
   }
}

maletus_defeated = {
   {
      title = _"Empire Defeated",
      body = sidolus(
         -- TRANSLATORS: Sidolus
         _([[May Satul warm you, Jundlina! We have defeated the troops of the Empire in the north. Now we should be able to establish a new home on this island.]]))
   },
   {
      title = _"Jundlina Cheers",
      body = jundlina(_"Jundlina",
         -- TRANSLATORS: Jundlina
         _([[Sidolus, this is very good news. I will talk immediately to our Babarian ally how divide the land we gained.]]))
   }
}

tribute = {
   {
      title = _"Tribute Demand",
      body = kalitath(
         -- TRANSLATORS: Kalitath
         _([[Strangers, we need to get some sign of your good intentions. So if you are really on our side you should send us some gold and a couple of other wares on a regular basis.]])..
         _([[At most we are interested in any wares that can be used to forge weapons. I mean real weapons like our axes, and not the sort of toothpicks you use. So feel free to send us 25 logs, 16 coal, 9 iron and 4 gold.]])
         .. paragraphdivider() ..
         _([[Oh I almost forgot this one. We demand to have your princess in our headquarters as well to coordinate our warplannings.]]))
   },
   {
      title = _"Pitfall!",
      body = sidolus(
         -- TRANSLATORS: Sidolus
         _([[May Satul warm you, Jundlina! This behaviour is truely babaric! I fear this is a trap to keep you as a hostage. And the demand itself is really an insult. I strongly recommend to decline this unsubstantiated claim.]]))
   },
   {
      title = _"Jundlina Concedes",
      body = jundlina(_"Jundlina",
         -- TRANSLATORS: Jundlina
         _([[Sidolus, I agree with you. However I think we have no choice so we need to obey to this demand. It is your obligation now to run our small people until I get back.]])
         .. paragraphdivider() ..
         _([[First you need to upgrade our scout’s house to a trading post. Then you need to send the demanded tributes of gold, iron, coal and logs.]]))
         .. new_objectives(obj_trading_post)
   }
}

trading = {
   {
      title = _"Trading post built",
      body = loftomor(
         -- TRANSLATORS: loftomor
         _([[Sidolus, may Satul warm you! We have finished the new trading post. Hopefully this will get us back Jundlina soon]]))
   },
   {
      title = _"Sidolus Agrees",
      body = sidolus(
         -- TRANSLATORS: sidolus
         _([[Well done, Loftomor, well done. May Satul warm our princess and our whole people. Now we need to begin sending goods to this barbarian.]])
         .. paragraphdivider() ..
         _([[I think we should start with the wood deliveries as we could afford them the easiest. But we need to find more metals.]]))
         .. new_objectives(obj_tribute)
   }
}

tribute_not_started = {
   {
      title = _"Urgent Tribute Demand",
      body = kalitath(
         -- TRANSLATORS: Kalitath
         _([[Strangers, we haven't received any help yet. So if you are really on our side you should start sending some wares urgently.]])..
         _([[As we are facing great losses due to your lazyness, you should now send us double the amount of each ware as before.]])
         .. paragraphdivider() ..
         _([[I am sure you won't loose our alliance, wouldn't you?]]))
   },
   {
      title = _"Blackmail!",
      body = sidolus(
         -- TRANSLATORS: Sidolus
         _([[May Satul warm us all. Our so-called ally is blackmailing us and we can't punish his insult as he has our princess at his hands.]])
         .. paragraphdivider() ..
         _([[Our only chance is to double our efforts and start delivering something soon.]]))
         .. new_objectives(obj_tribute2)
   },
}

tribute_started = {
   {
      title = _"Tribute Started",
      body = kalitath(
         -- TRANSLATORS: Kalitath
         _([[Strangers, we haven't received the first batch of wares. So we may finally trust you.]])..
         _([[However our enemy in the north is very strong. I am afraid without significant help from your side we might loose the war despite any ware help you might provide.]])
         .. paragraphdivider() ..
         _([[They name themselves an empire and their leader is named Maletus. I want you to train your soldiers and help us defeat his lousy empire.]]))
         .. new_objectives(obj_defeat_maletus)
   },
   {
      title = _"Be Vigilant!",
      body = jundlina(_"Jundlina",
         -- TRANSLATORS: Jundlina
         _([[May Satul warm you, Sidolus. Our ally now has gained some trust, but we shouldn't trust to much in them.]])
         .. paragraphdivider() ..
         _([[So I want you to defeat this Empire, but ensure that we will become and remain strong enough to enforce the respect we deserve.]]))
   },
}

alliance_broken = {
   {
      title = _"Traitors",
      body = kalitath(
         -- TRANSLATORS: Kalitath
         _([[Strangers, we still haven’t received any help. We are deeply insulted by your treachery.]])..
         _([[At least your princess will make a reasonable slave, if we survive the battle with our enemy.]]))
   },
   {
      title = _"Jundlinas Suicide",
      body = jundlina(_"Jundlina",
         -- TRANSLATORS: Jundlina
         _([[Barbarian, I am an Atlantean princess and I will never go into slavery. I have prepared myself for this eventuality and will rather die from the poison I carry with me then be your slave. May Satul avenge my sacrifice.]]))
   },
   {
      title = _"Defeated!",
      body = sidolus(
         -- TRANSLATORS: Sidolus
         _([[Aaaargh. I don’t know how this could have happened, but Satul couldn’t protect us from the enemy. We have lost our noble princess. Thus, we have lost this battle and our people is doomed!]]))
   },
}

kalitath_dead = {
   {
      title = _"Kalitath Defeated",
      body = sidolus(
         -- TRANSLATORS: Sidolus
         _([[Aaaargh. I don’t know how this could have happened, but Satul couldn’t protect our ally from the enemy. He lost his last warehouse and our noble princess perished with him. Thus, we have lost this battle and our people is doomed!]]))
   },
}
