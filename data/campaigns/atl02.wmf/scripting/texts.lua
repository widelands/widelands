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
   p(_[[To set foot on this island we first must find a building space where a port can be built. Explore the nearby coast line to find such a building plot.]]) ..
   p(_[[We should choose the ship to found the port with carefully to not waste time for unloading first. after the port is build we should cancel the remaining ships expedition, to unload its wares as well.]]))
}

obj_basic_infrastructure = {
   name = "obj_basic_infrastructure",
   title = _ "Ensure the supply of build wares",
   number = 4,
   body = objective_text(_"Establish a basic production",
   p(_[[We will need at least a quarry, a few lumberjacks and a sawmill. A forester might or might not be needed depending on how fast the trees are growing around you. Find the balance.]]))
}

obj_tools = {
   name = "obj_tools",
   title = _ "Build tools quickly",
   number = 1,
   body = objective_text(_"Make new tools",
      p(_[[Most of our tools have been lost in the ship tragedy. Our people will not be able to fulfill their duties without proper tools. Luckily we still have some iron ore and plenty of coal. Build a production of tools, it is a prerequisite for success on island. Don’t forget to build a smelter as well.]]))
}

obj_mining = {
   name = "obj_mining",
   title = _ "Explore the mountains for ressources",
   number = 3,
   body = objective_text(_"Build Mines",
      p(_[[As we are able to make tools again, our supply of mining ressources needs to be replenished soon. Explore the mountains and build at least one mine of each type to mine coal, iron and gold.]]))
}

obj_explore = {
   name = "obj_explore",
   title = _"Explore the surroundings",
   number = 1,
   body = objective_text(_"Explore your surroundings",
   p(_[[We are pretty sure that we are not alone on the island. We do not know if the others on this island are friends or foes so we need to treat carefully. Expand and explore the island by building at least one habitat for a scout and military buildings. Build at least one tower for his bigger view range.]]))
}

obj_spidercloth = {
   name = "obj_spidercloth_production",
   title = _"Build a spider farm and a weaving mill",
   number = 2,
   body = objective_text(_"Spidercloth Production",
      p(_[[We need some spidercloth urgently as we are running out of it. So it is essential to spare every piece we still have. Reduce the input of all buildings that consume spidercloth. Don’t forget the constructionsites.]]) ..
      p(_[[After that we need to have a farm under all circumstances as it needs spidercloth to be build. Additionally we need a spiderfarm and a weaving mill. Later on a gold spinning mill would be helpful as well.]]))
}

obj_trading_post = {
   name = "obj_trading_post",
   title = _"Build a trading post",
   number = 1,
   body = objective_text(_"Upgrade our scout’s house to a trading post",
   p(_[[To be able to pay the tributes to Kalitath, and keep contact with Jundlina. We need to enhance our scout’s house to a trading post.]]))
}

obj_tribute = {
   name = "obj_tribute",
   title = _"Pay all tributes to Kalitath",
   number = 1,
   body = objective_text(_"Send our trader to Kalitath to deliver the demanded goods",
   p(_[[We need to send the wares to Kalitath with our trader. He will pick them up as soon as a batch is ready in his trading post.]]) ..
   p(_[[The following batches need to be delivered:]]) ..
   li(_[[Five batches with five logs.]]) ..
   li(_[[Four batches with four coal.]]) ..
   li(_[[Three batches with three iron.]]) ..
   li(_[[Two batches with two gold.]]))
}

-- =======================================================================
--                                  Texts
-- =======================================================================

defeated = {
   {
      title =_"Defeated!",
      body = sidolus(_"Sidolus has bad news…",
         -- TRANSLATORS: Sidolus
         _([[Aaaargh, Jundlina. I don’t know how this could have happened, but Satul couldn’t protect us from the enemy. They have destroyed our last warehouse. So, we have lost this battle and our people is doomed!]])),
      posy=1,
   }
}

-- This is an array or message descriptions
initial_messages = {
   {
      title = _"The princess’ memoir",
      body = jundlina(_"Jundlina writes her diary",
      -- TRANSLATORS: Jundlina - Diary
      _([[Our escape from the cursed island was a close one: we managed to load three ships and make our escape, but the currents of the sinking island tested our new ships and their design. ]]) ..
      _([[And one failed the test: the last days of loading was done in chaos and without much planning and so one of our ships ended up carrying most of the heavy wares in our possession. ]]) ..
      _([[The heavy weight plus the strong current was too much for the ship and it was pulled below the sea level by Lutas forces and we lost most men and all wares it carried.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Jundlina - Diary
      _([[The fear was great in us all. We expected the other ships to be drowned as well, but they persisted. We escaped the island and began our smooth sailing to other coasts. ]]) ..
      _([[Many weeks have passed and we did only see small islands not suitable for habitation and without any sign of being blessed by Satul, the fire god. ]]) ..
      _([[Last night however, we made out a new island. We saw smoke hovering over it. The black comes from the fiery mountain in the north of the island, the grey however comes from settlers.]])),
      h=500,
   },
   {
      title = _"Planning for the future",
      body = jundlina(_"Jundlina continues",
      -- TRANSLATORS: Jundlina - Diary
      _([[At least the east side of the island is inhabited, so much is clear. We do not know if the people there are friends or foes. We do not even know what tribe they might be from. ]]) ..
      _([[The only thing that is sure that they can’t be Atlanteans for no ship of us has sailed as far as us in the last hundreds years. I can only hope they are friendly. ]]) ..
      _([[We have not much water left, our food is running low, we lost all our tools and most of our building materials with the third ship. We can’t afford sail any further an need to try to settle ourselves on this island.]]))
      .. new_objectives(obj_find_port),
      h=500,
   },
} -- end of initial messages.

initial_messages1= {
   {
      title = _"Jundlina gives orders",
      body = jundlina(_"Jundlina decides",
      -- TRANSLATORS: Jundlina - Diary
      _([[As we founded a port here, we can now plan our settlement. We still have plenty of iron ore, so a metal workshop and a smelter will provide us with good tools in no time. Also, we need to solve our building material problem. ]]) ..
      _([[The island is full of trees, we might not even need foresters. But we need stone, that is for sure!]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Jundlina - Diary
      _([[Let’s also explore this island. But we need to be careful. We should build at least one tower so that we can see potential enemies before they see us. ]]) ..
      _([[But we need to be careful with the stones and other precious material though. When we no longer need a building, we will dismantle it instead of burning it down. ]]) ..
      _([[This will take more time, but we can reuse some of its materials. Also, I want a scout out and exploring at all times.]]))
      .. new_objectives(obj_tools)
      .. new_objectives(obj_basic_infrastructure)
      .. new_objectives(obj_explore),
      h=500,
   },
} -- end of initial messages.

spidercloth_1 = {
   {
      title = _"Opol arrives at Jundlina",
      body = opol(
         -- TRANSLATORS: Opol
         _([[May Satul warm you, Jundlina. As highest weaver of the guild I have bad news for you. I just realized we are running out of spidercloth. This is very critical as you know that spidercloth is needed for many buildings.]])
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
         _([[Jundlina! It seems there is another problem even more urgent. I fear we haven’t build a farm to feed our spiders yet. As this is one of the buildings needing spidercloth for completion we are potentially running into a deadlock here.]]))
   },
   {
      title = _"Jundlina Acknowledges",
      body = jundlina(_"Jundlina",
         -- TRANSLATORS: Jundlina
         _([[I see the risk Opol. Thanks for reporting. We will build a farm first now.]]))
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
         _([[Jundlina, may Satul warm you! Another issue solved. We have found some ressources and build a mine of each type.]]))
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
         _([[Sidolus, may Satul warm you! Another issue solved. We have found some ressources and build a mine of each type.]]))
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
         _([[At most we are interested in any wares that can be used to forge weapons. I mean real weapons like our axes, and not the sort of toothpicks you use. So feel free to send us iron, coal, logs and gold.]])
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
         _([[First you need to upgrade our scout’s house to a trading post. Then you need to send the demanded tributes of 4 gold, 9 iron, 16 coal and 25 log.]]))
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
         _([[Well done, Loftomor, well done. May Satul warm our princess and our whole people. Now we need to begin sending goods to this barbarian. I think we should start with the wood deliveries as we could afford them the easiest. But we need to find more metals.]]))
   }
}
