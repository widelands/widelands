-- =========================
-- Some formating functions
-- =========================

include "scripting/formatting.lua"
include "scripting/format_scenario.lua"

function lutius(title, text)
   return speech("map:Lutius.png", "2F9131", title, text)
end
function saledus2(title, text)
   return speech("map:Saledus2.png", "8F9131", title, text)
end
function saledus3(title, text)
   return speech("map:Saledus3.png", "8F9131", title, text)
end
function saledus4(title, text)
   return speech("map:Saledus2.png", "8F9131", title, text)
end
function amalea1(title, text)
   return speech("map:Amalea.png", "AF7511", title, text)
end
function amalea2(title, text)
   return speech("map:Amalea2.png", "AF7511", title, text)
end
function amalea3(title, text)
   return speech("map:Amalea3.png", "AF7511", title, text)
end
-- =======================================================================
--                           Texts - No logic here
-- =======================================================================


-- ===========
-- objectives
-- ===========
obj_dismantle_buildings = {
    name = "dismantle_unproductive_buildings",
    title=_"Dismantle the unproductive Buildings",
    number = 1,
    body = objective_text(_"Dismantle Buildings",
       listitem_bullet(_[[We should dismantle all the unproductive buildings to get some ressources for new buildings.]])
    ),
}

obj_clear_roads = {
   name = "clear_roads",
   title=_"Clear all unnecessary roads",
   number = 1,
   body = objective_text(_"Clear road network",
      listitem_bullet(_[[Resolve the chaotic road network by clearing all unnecessary roads.]])
   ),
}

obj_find_farm_plans = {
   name = "find_farm_construction_plans",
   title=_"Find the construction plans for our farm",
   number = 1,
   body = objective_text(_"Find our construction plans",
      listitem_bullet(_[[We need to recover our plans how to construct a farm.]])
   ),
}

obj_build_quarries_and_lumberjacks = {
   name = "build_quarries_and_lumberjacks",
   title=_"Build at least 3 lumberjack’s houses and 2 quarries to renew our building material supply chain",
   number = 3,
   body = objective_text(_"Quarries and Lumberjacks",
      listitem_bullet(_[[Build at least 3 lumberjack’s houses and 2 quarries to renew our building material supply chain]])
   ),
}

obj_produce_fish = {
   name = "produce_fish",
   title=_"Produce Fish and Rations",
   number = 1,
   body = objective_text(_"Produce food",
      listitem_bullet(_[[Find and catch some fish. Afterwards produce rations for our miners.]])
   ),
}

obj_replace_foresters = {
   name = "Replace_foresters",
   title=_"Replace our old and uneffective foresters",
   number = 1,
   body = objective_text(_"Replace our foresters by new ones",
      listitem_bullet(_[[Our two forester's houses are old and weared out. We should replace them by new ones to increase productivity.]])
   ),
}

-- obj_build_port_and_shipyard = {
   -- name = "build a port and a shipyard",
   -- title=_"Build a port and a shipyard on the island",
   -- number = 2,
   -- body = objective_text(_"Ship Industry",
      -- listitem_bullet(_[[Build a port to set sail again.]])..
	  -- listitem_bullet(_[[Build a shipyard to produce the ships to sail on.]])),
-- }

-- obj_produce_wheat = {
   -- name = "produce 50 wheat",
   -- title=_"Produce at least 50 wheat",
   -- number = 1,
   -- body = objective_text(_"Wheat production",
      -- listitem_bullet(_[[To start a wool production we need at least 50 wheat.]])
   -- ),
-- }

-- obj_produce_cloth = {
   -- name = "produce cloth",
   -- title=_"Produce cloth for building our first ship",
   -- number = 1,
   -- body = objective_text(_"Cloth production",
      -- listitem_bullet(_[[Now let's use the wheat to produce wool (at least 50) and then use the wool to weave cloth to build a ship.]])
   -- ),
-- }

-- obj_build_expedition_ports = {
   -- name = "build two expedition ports",
   -- title=_"Build 2 ports on distant islands",
   -- number = 2,
   -- body = objective_text(_"Build 2 distant ports by sending expeditions",
      -- listitem_bullet(_[[We should explore the island in the south, use an expedition to build a port there. Afterwards let's try to find some mining resources.]])..
	  -- listitem_bullet(_[[We should send a ship northwards to check if the barbarians are there. Try to build a port to found an outpost.]])
   -- ),
-- }

-- obj_produce_gold = {
   -- name = "produce gold",
   -- title=_"Produce gold to send out expeditions",
   -- number = 1,
   -- body = objective_text(_"Gold production",
      -- listitem_bullet(_[[We need to produce at least 6 gold to be able to colonise our vicinity.]])
   -- ),
-- }

-- obj_conquer_all = {
   -- name = "Conquer all",
   -- title=_"Defeat the barbarians",
   -- number = 1,
   -- body = objective_text(_"Defeat the enemy",
      -- listitem_bullet(_[[We should end the barbarians existence in this part of the world.]])
   -- ),
-- }

-- obj_find_all_ports = {
   -- name = "Explore additional islands and search for port spaces",
   -- title=_"Explore further and build ports",
   -- number = 1,
   -- body = objective_text(_"Explore additional islands and search for port spaces",
      -- listitem_bullet(_[[We should discover more land and build as many ports as we can.]])
   -- ),
-- }

-- obj_find_artifacts = {
   -- name = "Find the pieces of Neptune's shrine",
   -- title=_"Recover the shrine",
   -- number = 1,
   -- body = objective_text(_"Find all 6 pieces of the holy shrine",
      -- listitem_bullet(_[[We need to find and secure all pieces of the holy shrine of Neptune.]])
   -- ),
-- }

-- obj_training = {
   -- name = "Build training infrastructure",
   -- title=_"Build at least a trainingcamp, some barracks and an arena or colosseum",
   -- number = 1,
   -- body = objective_text(_"Training Infrastructure",
      -- listitem_bullet(_[[Build a trainingcamp, some barracks and an arena or colosseum to train our soldiers.]])
   -- ),
-- }


-- ==================
-- Texts to the user
-- ==================
diary_page_1 = {
   title =_"Home, sweet home",
   body=lutius(_"Diary of Lutius",
      -- TRANSLATORS: Lutius - Diary
      _([[Finally we managed to get home. I am so glad to see the towers of our city again. I really look forward for a walk on our lovely coast and a hunt in our deep forrests.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[Saledus, Amalea, aren't you delighted as well?]])),
	
   posy=1,
}

diary_page_2 = {
   title=_"Chaos",
   body= lutius(_"Diary of Lutius",
      -- TRANSLATORS: Lutius - Diary
      _([[Oh no. Amalea is right. In fact I can't see any productivity overall. And our road network seems to be a complete mess as well. Who might be responsible for this chaos?]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[I am really afraid we could be totally unprepared if the babarians will attack us.]])),

   posy=1,
}

-- diary_page_3 = {
   -- title=_"Shipwrecked again",
   -- body= lutius(_"Diary of Lutius",
      -- -- TRANSLATORS: Lutius - Diary
      -- _([[Finally, the wrath of the Gods and the ocean seems to be over.]])
      -- .. paragraphdivider() ..
      -- -- TRANSLATORS: Lutius - Diary
      -- _([[We landed on an unknown coast and found some rest under the palm trees growing at the shore. But that is about as good as it gets. The truth is: when I woke up this morning, I saw nothing but sand and stone around us.]])
      -- .. paragraphdivider() ..
      -- -- TRANSLATORS: Lutius - Diary
      -- _([[It really seems as if we have landed on a very small island with little space.]])
	  -- .. paragraphdivider() ..
      -- -- TRANSLATORS: Lutius - Diary
      -- _([[To make it even worse all the land we can see is blocked by stone and trees. I fear we won’t find the resources we need to build a new ship.]])
	  -- .. paragraphdivider() ..
      -- -- TRANSLATORS: Lutius - Diary
      -- _([[I really don't know if and how we will get back home. And the war is going on without us. I hope my comrades defend our empire well.]])),

   -- posy=1,
-- }

saledus = {
    title=_"Military Strength",
    body= saledus2(_"Saledus looks very relaxed…",
       -- TRANSLATORS: Saledus
       _([[Sire, you are right it is a great pleasure to be at home again. And the best of all things: I can see we still have some military strength. I can count five towers and a fortress in our vicinity to guard us.]])),
 
    posy=1,
}

amalea = {
    title=_"Amalea looking puzzled",
    body= amalea1(_"Amalea is doubtfull…",
    -- TRANSLATORS: Amalea
    _([[Salve Lutius. Yes, on the one hand I'm glad to see our homeland again. But on the other hand I have the impression that something went deeply wrong here.]])
    .. paragraphdivider() ..
    -- TRANSLATORS: Lutius - Diary
    _([[Have a look on the economy. There is nearly no productivity at all. Who ever managed our country while we were absent, created a huge mess. I'm not quite sure how we can fix this, if at all.]])),

    posy=1,
}

-- saledus_1 = {
   -- title=_"Spare resources",
   -- body= saledus2(_"Saledus is nodding in agreement…",
      -- -- TRANSLATORS: Saledus
      -- _([[Sire, Amalea is right about the resources. Therefore I correct myself. We should only build an outpost as this is the most effective military building.]]))
      -- .. new_objectives(obj_build_first_outpost),
	  
   -- posy=1,
   -- h=500,
-- }

-- saledus_2 = {
   -- title=_"Stone, nothing but stone",
   -- body= saledus3(_"Saledus speaks with a sigh of relief…",
      -- -- TRANSLATORS: Saledus
      -- _([[Sire, I saw that the construction of the outpost was completed, so I have assigned some of my best soldiers to it to keep the watch.]])
      -- .. paragraphdivider() ..
      -- -- TRANSLATORS: Saledus
      -- _([[But although this is a good step forward I can't see much space to build on. There is only stone all over the place. How can we ever build a new ship or even have some basic infrastructure to make our lives more comfortable?]])),
   -- posy=1,
-- }

amalea_1 = {
   title=_"Amalea investigating",
   body= amalea2(_"Amalea is nodding thougtful…",
      -- TRANSLATORS: Amalea
      _([[Lutius in my opinion this will be a very difficult challenge again. But I'm afraid, we're doomed to manage this situation. To make things even worse I had a look into our warehouses. There is hardly some grain of dust left in there. No Wares, no tools, no workers and no soldiers as well.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[So, first of all we need some building material to start correcting the mistakes made.I think we should try to dismantle the unproductive small buildings and the farms to collect the ressources and regain them in our headquarters.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[Additionally we should restrict the input to all builidngs which consume any of our building material to zero. Or maybe you could pause the Production in all bigger Buildings and get the workers some rest until we will have produced some of their input wares.]]))
      .. new_objectives(obj_dismantle_buildings),
   posy=1,
   h=500,
}

amalea_2 = {
   title=_"Amalea's bad news",
   body= amalea3(_"Amalea recommends…",
      -- TRANSLATORS: Amalea
      _([[Lutius, I'm really worried to deliver bad news again. As you might have noticed we couldn't dismantle our farm, but just destroy it.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[This is caused by the sad fact that our constructors have lost the plans how to construct a farm. for this reason they don't know how to dismantle it either.]])
	  .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[So we need to recover our construction plans for a farm. One older constructor told me that they might have been concealed in a cave in the northern mountains.]]))
	  .. new_objectives(obj_find_farm_plans ),
   posy=1,
   h=500,
}

amalea_3 = {
   title=_"Amalea looking confident",
   body= amalea2(_"Amalea is more confident",
      -- TRANSLATORS: Amalea
      _([[Lutiius now we have some options. As we have gained some construction material we can start to rebuild our economy.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[first of all we need more construction material. So we should build at least 3 lumberjack_houses and 2 quarries.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[Oh before I forget this in the meantime while our builders are doing their job somebody should clear the mess in our road network. This is such a chaos wasting a lot of building lots.]]))
      .. new_objectives(obj_build_quarries_and_lumberjacks)
	  .. new_objectives(obj_clear_roads),
   posy=1,
   h=500,
}

-- amalea_10 = {
   -- title=_"Amalea looks happy",
   -- body= amalea2(_"Amalea comes in…",
      -- -- TRANSLATORS: Amalea
      -- _([[Ave Lutius. I have very good progress to report now.]])
      -- .. paragraphdivider() ..
      -- -- TRANSLATORS: Amalea
      -- _([[I found out that the construction of the lumberjack’s house and the sawmill is complete, so we can begin to harvest the logs and make planks out of them.]])
      -- .. paragraphdivider() ..
      -- -- TRANSLATORS: Amalea
      -- _([[This is a great step forward in constructing advanced buildings, and eventually we can use some planks to build new ships.]])),
   -- posy=1,
-- }

-- saledus_3 = {
   -- title=_"We need a port",
   -- body= saledus2(_"Saledus thoughtfully…",
      -- -- TRANSLATORS: Saledus
      -- _([[Sire, I just thought about our way home. It is utmost important to build a port for loading our ships and start an expedition.]])
      -- .. paragraphdivider() ..
      -- -- TRANSLATORS: Saledus
      -- _([[The problem is that a port is a building which needs a very special place to be build. So while buidling our economy we should nevertheless expand further to discover the whole island. Let's try to find such a valuable construction site.]]))
      -- .. new_objectives(obj_find_port_space),
   -- posy=1,
   -- h=500,
-- }

-- saledus_4 = {
   -- title=_"A Step closer to get home",
   -- body= saledus3(_"Saledus looks excited…",
      -- -- TRANSLATORS: Saledus
      -- _([[Sire, today we got a lot closer to finally get home again. Our military has spotted a space to build a port on. We should start to build a port quickly.]])
      -- .. paragraphdivider() ..
      -- -- TRANSLATORS: Saledus
      -- _([[Now we truly can look forward to build some ships. So we should build a shipyard as well.]]))
	  -- .. new_objectives(obj_build_port_and_shipyard),
   -- posy=1,
   -- h=500,
-- }

amalea_4 = {
   title=_"Amalea is somewhat relieved",
   body= amalea2(_"Amalea is giving a deep sigh ...",
      -- TRANSLATORS: Amalea
      _([[Praise the gods Lutius. We have refound the plans how to build and operate a farm.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[Now we can start to build farms to produce some beer which our miners need so daringly.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[But I'm afraid that this problem hasn't been the last in our economy.]])),
   posy=1,
   h=500,
}


-- diary_page_4 = {
   -- title=_"Seafaring possible again",
   -- body= lutius(_"Diary of Lutius",
      -- -- TRANSLATORS: Lutius - Diary
      -- _([[Praise the gods. We finally have a ship industry.]])),
   -- posy=1,
-- }


amalea_5 = {
   title=_"Amalea has good news",
   body= amalea2(_"Amalea smiles for the first time since long…",
      -- TRANSLATORS: Amalea
      _([[Lutius, well done so far. I just got the news that we have finished the basic buildings to get some construction material.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[This is offering us more options to get the mess in our economy solved.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[I really think we can start to look forward into our future now. I pray that we can make ourselves comfortable in our homeland again.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[But this will be still hard work. For now we should start to find some fish and build some huts for our fishermen. As all the fish has been caught at our coast we should try our luck in the eastern part of our territory.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[After we have managed this we could start making rations in our tavern.]]))
      .. new_objectives(obj_produce_fish),
   posy=1,
   h=500,
}

-- saledus_5 = {
   -- title=_"We should explore the islands",
   -- body= saledus2(_"Saledus is excited…",
      -- -- TRANSLATORS: Saledus
      -- _([[Now that we have all resources ready to built a ship, we should explore the surrounding islands.]])
      -- .. paragraphdivider() ..
      -- -- TRANSLATORS: Saledus
      -- _([[First we need to find some more resources to back up our growing economy. I have spotted an island with some mountains in the south. Perhaps we can find some valuable resources like iron there.]])
	  -- .. paragraphdivider() ..
      -- -- TRANSLATORS: Saledus
      -- _([[But even more important is the following objective.]])
      -- .. paragraphdivider() ..
      -- -- TRANSLATORS: Saledus
      -- _([[I think I have heard the wind carrying some weird but well known sounds from the north. If I'm not totally wrong I heard some barbarian war drums, so we should better check what is going on up there.]]))
	  -- .. new_objectives(obj_build_expedition_ports),
   -- posy=1,
   -- h=500,
-- }

amalea_6 = {
   title=_"Amalea is pleased",
   body= amalea3(_"Amalea is nodding her head…",
      -- TRANSLATORS: Amalea
      _([[Well done. Well done. Now our road network looks a lot more structured than before.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[Now we can focus on rebuilding our economy.]])),
   posy=1,
   h=600,
}

-- saledus_6 = {
   -- title=_"We should conquer the whole islands",
   -- body= saledus3(_"Saledus is excited…",
      -- -- TRANSLATORS: Saledus
      -- _([[Now it is time to conquer the islands.]])
      -- .. paragraphdivider() ..
      -- _([[Let's finish the barbarians off and provide a nice colony for the caesar of our empire.]]))
      -- .. new_objectives(obj_conquer_all),
   -- posy=1,
   -- h=500,
-- }

amalea_7 = {
   title=_"Amalea shaking her head",
   body= amalea2(_"Amalea is getting fed up with all the problems in this economy",
      -- TRANSLATORS: Amalea
      _([[Lutius for the sake of Neptune, I just discovered another problem. It seems that really very few things are working as expected in this economy.]])
      .. paragraphdivider() ..
	  -- TRANSLATORS: Amalea
      _([[One of our lumberjacks told me that the reproduction of our forests is far behind his experience and expectations. So I had a deep look on our foresters effectiveness.]])
      .. paragraphdivider() ..
	  -- TRANSLATORS: Amalea
      _([[And guess what, they are both old. Their houses and tools are weared and their seed is degenerated. For this reason they need much more time to plant a tree then usual.]])
      .. paragraphdivider() ..
	  -- TRANSLATORS: Amalea
      _([[The only solution is we need to replace them both with new foresters houses. Be sure to first build a new forester and then destroy the old one. Dismantling is not an option because they are so weared.]]))
	  .. new_objectives(obj_replace_foresters),
   posy=1,
   h=500,
}

amalea_8 = {
   title=_"Amalea sarcastic",
   body= amalea1(_"Amalea is laughing sarcastically",
      -- TRANSLATORS: Amalea
      _([[Ok Lutius. We have solved just another weird behaviour in our economy. Now our lumberjacks would be supplied with enough wood to enhance our economy.]])
      .. paragraphdivider() ..
	  -- TRANSLATORS: Amalea
      _([[I am deeply curious what will go wrong next.]])),
   posy=1,
}

-- amalea_9 = {
   -- title=_"Amalea shaking head",
   -- body= amalea3(_"Amalea is reminding ...",
      -- -- TRANSLATORS: Amalea
      -- _([[Lutius. I really appreciate that you and Saledus are so concerned about our security. But I think we could use the little space on this island better than building a lot of military buildings.]])
      -- .. paragraphdivider() ..
	  -- -- TRANSLATORS: Amalea
      -- _([[I strongly recommend to dismantle most of them to be able to build industrial buildings. I don't think there is much danger on this island, but if you want to be cautious you can keep some sentries or blockhouses near the coastline.]])
      -- .. paragraphdivider() ..
	  -- -- TRANSLATORS: Amalea
      -- _([[But be careful not to waste any bigger building lots.]])),
   -- posy=1,
-- }

-- amalea_11 = {
   -- title=_"Amalea excited",
   -- body= amalea1(_"Amalea is looking forward for exploration ...",
      -- -- TRANSLATORS: Amalea
      -- _([[Lutius. We have made a lot of progress so far. I just got information that we have completed our third ship. ]])
      -- .. paragraphdivider() ..
	  -- -- TRANSLATORS: Amalea
      -- _([[Now that our fleet is growing and growing we should explore the complete region. Let's try to find more land and eventually get more resources.]])
      -- .. paragraphdivider() ..
	  -- -- TRANSLATORS: Amalea
      -- _([[In particular it would be very helpful to find more marble either within stone fields or in the mountains to mine it.]]))
	  -- .. new_objectives(obj_find_all_ports),
   -- posy=1,
   -- h=500,
-- }

-- amalea_12 = {
   -- title=_"Amalea is very content",
   -- body= amalea2(_"Amalea is clapping her hands...",
      -- -- TRANSLATORS: Amalea
      -- _([[Lutius. Well done. Well done. We have discovered all possible port locations in the complete region. Now we can strengthen our economy and we will manage to sail home soon.]])),
   -- posy=1,
-- }
   
-- saledus_7 = {
   -- title=_"Victory is our's",
   -- body= saledus3(_"Saledus is cheering out…",
      -- -- TRANSLATORS: Saledus
      -- _([[Sire, finally we have defeated the babarians in this region. And furthermore we have built a nice colony for our empire.]])
      -- .. paragraphdivider() ..
      -- _([[Caesar will be very pleased about our victory.]])),
   -- posy=1,
-- }

-- saledus_8 = {
   -- title=_"Pleasing Neptune",
   -- body= saledus4(_"Saledus is very sad and anxious…",
      -- -- TRANSLATORS: Saledus
      -- _([[Sire, we are making good progress to get home eventually. But there is one thing I need to talk about:]])
      -- .. paragraphdivider() ..
      -- _([[As we have seen on our last journey it is of great importance that the Gods, especially Neptune, are in a good mood to guard our journey.]])
      -- .. paragraphdivider() ..
      -- _([[Unfortunately we lost our holy shrine of Neptune in the great storm, so we can't worship him well. I think we should search the whole region to see if we can recover the shrine with all its 6 pieces.]])
      -- .. paragraphdivider() ..
      -- _([[To ease the search I made some models of the pieces as you can see. You can give them to our soldiers and sailors to look for them.]]))
	  -- .. new_objectives(obj_find_artifacts),
   -- posy=1,
   -- h=500,
-- }

-- saledus_9 = {
   -- title=_"Praise, praise, praise",
   -- body= saledus2(_"Saledus is happily worshipping…",
      -- -- TRANSLATORS: Saledus
      -- _([[Sire, we recovered the holy shrine of Neptune. Now we can seek his protection for our journey.]])
      -- .. paragraphdivider() ..
      -- _([[Oh great Neptune safeguard our journey. Oh great Neptune let the winds be friendly for our course. Oh great Neptune protect us from the creatures of the sea. Oh great Neptune......]])),
   -- posy=1,
-- }

-- saledus_12 = {
   -- title=_"Unpleasant discoveries",
   -- body= saledus3(_"Saledus looks very worried…",
      -- -- TRANSLATORS: Saledus
      -- _([[Sire, we found the ruins of a former babarian fortress and village. I'm somewhat worried we will have to fight against them sooner than we wanted.]])),
   -- posy=1,
-- }

-- saledus_10 = {
   -- title=_"Train faster, train harder",
   -- body= saledus2(_"Saledus is demanding some discipline",
      -- -- TRANSLATORS: Saledus
      -- _([[Sire, we have all buildings available to train our soldiers but we should increase our efforts to train them to the highest level and to train more of them.]])
      -- .. paragraphdivider() ..
      -- _([[We need a lot of food, a lot of iron and a lot of gold to do so. So we have lots of challenges to cope with.]])),
   -- posy=1,
-- }

-- saledus_11 = {
   -- title=_"Training infrastructure is needed",
   -- body= saledus3(_"Saledus has security concerns",
      -- -- TRANSLATORS: Saledus
      -- _([[Sire, we need to train some of our people to serve in your great army. It is important to increase the number of our soldiers as fast as we can. And we should train them well to improve their fighting abilities.]])
      -- .. paragraphdivider() ..
      -- _([[Therefore we need to build a full training infrastructure despite the limited space available. We need some barracks, a trainingcamp and an arena or a colosseum.]]))
	  -- .. new_objectives(obj_training),
   -- posy=1,
   -- h=500,
-- }



-- diary_page_5 = {
   -- title=_"Now it's time to get home again",
   -- body= lutius(_"Diary of Lutius",
      -- -- TRANSLATORS: Lutius - Diary
      -- _([[Yes my fellow comrades, we have done it. We managed all the challenges that were imposed on us. But my heart is still with our home. We shall set sail as soon as possible to help defend the empire from the babarian tribe.]])
	  -- .. paragraphdivider() ..
      -- _([[You have completed this mission. You may continue playing if you wish, otherwise move on to the next mission.]])),
   -- posy=1,
-- }


-- function diary_page_6(count)
   -- local text = ""

      -- text = text .. paragraphdivider() ..
      -- -- TRANSLATORS: Lutius - Diary
      -- ngettext([[chaos noch da.]], [[Noch %d Flaggen.]], count):bformat(count)

   -- return {
      -- title=_"flaggen im Chaos",
      -- body= lutius(_"Diary of Lutius", text),
      -- posy=1,
      -- w=500,
   -- }
-- end