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
   return speech("map:Saledus.png", "8F9131", title, text)
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
obj_build_first_outpost = {
    name = "build_a_first_outpost",
    title=_"Build a first outpost",
    number = 1,
    body = objective_text(_"First Outpost",
       listitem_bullet(_[[Build a outpost at the yellow house symbol to the east of your provisional headquarters.]])
    ),
}

obj_build_quarry = {
   name = "build_quarry",
   title=_"Build a quarry",
   number = 1,
   body = objective_text(_"Quarry",
      listitem_bullet(_[[Build a quarry close to the lots of stone. So we can free some building lots and get some valued resources.]])
   ),
}


obj_build_sawmill_stonemason_and_lumberjacks = {
   name = "build_sawmill_stonemason_and_lumberjacks",
   title=_"Build at least a lumberjack’s house, a sawmill and a stonemason to complete the building material supply chain",
   number = 3,
   body = objective_text(_"Lumberjack’s House, Sawmill and Stonemason",
      listitem_bullet(_[[Build at least a lumberjack’s house, a sawmill and a stonemason as soon as there is enough space for them.]])
   ),
}

obj_lower_marble_column_demand = {
   name = "lower marble column demand",
   title=_"Lower the demand for marble columns in the economy settings",
   number = 1,
   body = objective_text(_"Lower demand for marble columns",
      listitem_bullet(_[[We should spare some marble for buildings. So please lower the demand for marble columns.]])
   ),
}

obj_find_port_space = {
   name = "find a portspace",
   title=_"Find a portspace on the small island",
   number = 1,
   body = objective_text(_"Port Space",
      listitem_bullet(_[[To get back to the sea we first must discover a portspace.]])
   ),
}

obj_build_port_and_shipyard = {
   name = "build a port and a shipyard",
   title=_"Build a Port and a shipyard on the island",
   number = 2,
   body = objective_text(_"Ship Industry",
      listitem_bullet(_[[Build a port to set sail from this island.]])..
	  listitem_bullet(_[[Build a shipyard to produce the ships to sail on.]])),
}

obj_produce_wheat = {
   name = "produce 50 wheat",
   title=_"Produce an amount of 50 wheat",
   number = 1,
   body = objective_text(_"Wheat production",
      listitem_bullet(_[[To start a wool production we need an amount of at least 50 wheat.]])
   ),
}

obj_produce_cloth = {
   name = "produce cloth",
   title=_"Produce cloth for building our first ship",
   number = 1,
   body = objective_text(_"Cloth production",
      listitem_bullet(_[[Now let's turn all the wheat into wool (at least 50) and all the wool into cloth to build a ship.]])
   ),
}

obj_build_expedition_ports = {
   name = "build two expedition ports",
   title=_"Build 2 expedition ports",
   number = 2,
   body = objective_text(_"Build 2 expedition ports",
      listitem_bullet(_[[We should explore the Island in the south and build an expedition port. Let's try to find some mining ressources there.]])..
	  listitem_bullet(_[[We should send a ship to the north to get certainty about the barbarians. Try to build a port on the island to found an outpost.]])
   ),
}

obj_produce_gold = {
   name = "produce gold",
   title=_"Produce gold to send out expeditions",
   number = 1,
   body = objective_text(_"Gold production",
      listitem_bullet(_[[We need to produce at least 6 gold to be able to colonize our vicinity.]])
   ),
}

obj_conquer_all = {
   name = "Conquer all",
   title=_"Defeat the barbarians",
   number = 1,
   body = objective_text(_"Defeat the enemy",
      listitem_bullet(_[[We should end the barbarians existence in this piece of the world.]])
   ),
}

obj_find_all_ports = {
   name = "Explore additional islands and search for port spaces",
   title=_"Explore further and build ports",
   number = 1,
   body = objective_text(_"Explore additional islands and search for port spaces",
      listitem_bullet(_[[We should discover more land and build as much ports as we can.]])
   ),
}

obj_find_artifacts = {
   name = "Find the pieces of Neptune's shrine",
   title=_"Recover the shrine",
   number = 1,
   body = objective_text(_"Find all pieces of the holy shrine",
      listitem_bullet(_[[We need to find and secure all pieces of the holy shrine of Neptune.]])
   ),
}

obj_training = {
   name = "Build training infrastructure",
   title=_"Build at least a trainingcamp, some barracks and an arena or colosseum",
   number = 1,
   body = objective_text(_"Training Infrastructure",
      listitem_bullet(_[[Build a trainingcamp, some barracks and an arena or colosseum to train our soldiers.]])
   ),
}


-- ==================
-- Texts to the user
-- ==================
diary_page_1 = {
   title =_"Journey home",
   body=lutius(_"Diary of Lutius",
      -- TRANSLATORS: Lutius - Diary
      _([[Finally we were set to Fremil again. I couldn't await the chance to clear my records of this big defeat by the babarians.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[But the gods themselves didn't want us to get home so quickly!]])),
	
   posy=1,
}

diary_page_2 = {
   title=_"The wrath of Neptune",
   body= lutius(_"Diary of Lutius",
      -- TRANSLATORS: Lutius - Diary
      _([[Neptune himself was against us and drove us into a dark, wild storm again. But this time it felt even worse than last time. The waves were double the height of our brave ship and the winds drove us into unsafe waters.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[Our ship is heavily damaged. The mast has gone overboard and the hull is taking in more and more water. We would be glad to survive this black night with barely more than our lives.]])),

   posy=1,
}

diary_page_3 = {
   title=_"Shipwrecked again",
   body= lutius(_"Diary of Lutius",
      -- TRANSLATORS: Lutius - Diary
      _([[Finally, the wrath of the gods and the ocean seems to be over.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[We landed on an unknown coast and found some rest under the palm trees growing at the shore. But that is about as good as it gets. The truth is: when I woke up this morning, I saw nothing but sand and stone around us.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[It really seems as if we have landed on a very small island with little space.]])
	  .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[To make it even worse all the land we can see is blocked by stone and trees. I fear we won’t find any help we need to build a new ship.]])
	  .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[I really don't know if and how we will get back home. And the war is going on without us. I hope my comrades defend our empire well.]])),

   posy=1,
}

saledus = {
    title=_"No sight",
    body= saledus2(_"Saledus looks around nervously…",
       -- TRANSLATORS: Saledus
       _([[Sire, we do not know anything about the island. And I believe we are not safe as well. Probably it would be reasonable to build a tower to explore the island and add some military strength.]])),
 
    posy=1,
}

amalea = {
    title=_"Amalea interrupting",
    body= amalea1(_"Amalea raises her hand…",
    -- TRANSLATORS: Amalea
    _([[Salve Lutius. As we don't know how much resources we can get we should be very restrictive in spending them. Even for military buildings we should be very careful.]])),

    posy=1,
}

saledus_1 = {
   title=_"Spare ressources",
   body= saledus2(_"Saledus is nodding in agreement…",
      -- TRANSLATORS: Saledus
      _([[Sire, Amalea is right about the resources. Therefore I correct myself. We should only build an outpost as this is the most effective military building.]]))
      .. new_objectives(obj_build_first_outpost),
	  
   posy=1,
}

saledus_2 = {
   title=_"Stone, nothing but stone",
   body= saledus3(_"Saledus speaks with a sigh of relief…",
      -- TRANSLATORS: Saledus
      _([[Sire, I saw that the construction of the outpost was completed, so I have assigned some of my best soldiers to it to keep the watch.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[But although this is a good step forward I can't see much space for buildings. There is only stone all over the place. How can we ever build a new ship or even have some basic infrastructure to make our lives more comfortable.]])),
   posy=1,
}

amalea_1 = {
   title=_"Amalea's plan",
   body= amalea2(_"Amalea smiles…",
      -- TRANSLATORS: Amalea
      _([[Lutius this can be very tricky but it is reachable. First of all we need patience as well as speed and a plan to succeed in this part of the world.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[I think we should try to reveal some building space by cutting all the stone in a quarry.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[Additionally this would gain us some stone and some marble to build up our new world. We can use them as soon as there is enough space for buildings finally.]]))
      .. new_objectives(obj_build_quarry),
   posy=1,
}

amalea_2 = {
   title=_"Amalea's new instructions",
   body= amalea3(_"Amalea recommends…",
      -- TRANSLATORS: Amalea
      _([[Good news from the people, Lutius. I got the message that our first quarry has started it's work today.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[Perhaps it would be a good idea to wait until it has cleared some space for building up a basic construction material industry.]]))
	  .. new_objectives(obj_build_sawmill_stonemason_and_lumberjacks),
   posy=1,
}

amalea_3 = {
   title=_"Amalea's recommendations",
   body= amalea2(_"Amalea comes in…",
      -- TRANSLATORS: Amalea
      _([[I’ve got important things to talk about… First the good news:]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[I noticed that our stonemason has started it’s work finally. He will cut the number of columns defined in our basic economy settings.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[But here is the problem, if we use all of our marble for columns we might ran out of it. So we should lower the settings for marble columns in the economy settings to a smaller amount. I think a reserve of 4 columns might be sufficient for the moment being.]]))
      .. new_objectives(obj_lower_marble_column_demand),
   posy=1,
}

amalea_10 = {
   title=_"Amalea looks happy",
   body= amalea2(_"Amalea comes in…",
      -- TRANSLATORS: Amalea
      _([[Ave Lutius. I have very good progress to report now.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[I found out that the construction of the lumberjack and the sawmill is complete, so we can begin to harvest the logs and make planks out of it.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[This is a great step forward in constructing advanced buildings. And we can think of using some planks to build new ships.]])),
   posy=1,
}

saledus_3 = {
   title=_"We need a port",
   body= saledus2(_"Saledus thoughtfully…",
      -- TRANSLATORS: Saledus
      _([[Sire, I just thought about our way home. It is utmost important to build a port for loading our ships and start an expedition.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[The problem is that a port is a building which needs a very special place to be build. So while buidling our economy we should nevertheless expand further to discover the whole island. Let's try to find such a valuable place.]]))
      .. new_objectives(obj_find_port_space),
   posy=1,
}

saledus_4 = {
   title=_"A Step closer to get home",
   body= saledus3(_"Saledus looks excited…",
      -- TRANSLATORS: Saledus
      _([[Sire, today we got a lot closer to finally get home again. Our military has spotted a space to build a port on. We should start to build a port quickly.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[Now we truly can look forward to build some ships. So we should build a shipyard as well.]]))
	  .. new_objectives(obj_build_port_and_shipyard),
   posy=1,
}

amalea_4 = {
   title=_"Amalea's wisdom",
   body= amalea2(_"Amalea recommends…",
      -- TRANSLATORS: Amalea
      _([[I understand the euphoria of saledus but we have some serious problems to solve first.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[To send out an expeditionary ship we first need a ship. Unfortunately we have only 5 cloth left which we will need to build the port and the shipyard.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[So we need more cloth to finally be able to build ships. But there is no space on the island to build a whole economy to produce cloth. Therefore we need to build the necessary wares in a more serial way.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[So let's think this over. To produce cloth we need some wool. And to produce wool we need wheat so let'S find a space for a big building and build a wheat farm first.]])
	  .. paragraphdivider() ..
	  -- TRANSLATORS: Amalea
      _([[After the production of enough wheat we can dismantle the farm and build a sheepfarm to produce wool. Later on we could build a weaving mill in the same spot to use our wool for weaving cloth. But for the moment being let's start with producing a reasonable amount of wheat.]])
	  .. paragraphdivider() ..
	  -- TRANSLATORS: Amalea
      _([[I think for the beginning a stock of 50 wheat would be enough. Perhaps we have to adjust the target quantity for wheat to achieve this.]]))
      .. new_objectives(obj_produce_wheat),
   posy=1,
}


diary_page_4 = {
   title=_"Seafaring possible again",
   body= lutius(_"Diary of Lutius",
      -- TRANSLATORS: Lutius - Diary
      _([[Praise the gods. We finally have a ship industry.]])),
   posy=1,
}


amalea_5 = {
   title=_"Amalea's pride",
   body= amalea2(_"Amalea cheers…",
      -- TRANSLATORS: Amalea
      _([[Well done so far. We have produced enough wheat. Now dismantle the farm and build the next building in the production chain.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[Don't forget to check the target quantity of wool and cloth in the economy settings. We need to turn all the wheat into wool and then the all the wool into cloth.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[Perhaps it would be a good idea to stop the shipyard's production until we have some cloth available to not waste any logs or planks.]]))
      .. new_objectives(obj_produce_cloth),
   posy=1,
}

saledus_5 = {
   title=_"We should explore the islands",
   body= saledus2(_"Saledus is excited…",
      -- TRANSLATORS: Saledus
      _([[Now that we have all ressources ready to built a ship, we should explore the surrounding islands.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[First we need to find some more ressources to back up our growing economy. I have spotted an island with some mountains in the south. Perhaps we can find some valuable ressources like iron there.]])
	  .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[But even more important is the following objective.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[I think I have heard the wind carrying some weird but well known sounds from the north. If I'm not totally wrong I heard some barbarian war drums, so we should better going to look what is going on up there.]]))
	  .. new_objectives(obj_build_expedition_ports),
   posy=1,
}

amalea_6 = {
   title=_"Amalea has some advice",
   body= amalea3(_"Amalea looks thoughtfully…",
      -- TRANSLATORS: Amalea
      _([[Salve Lutius. While talking to all our brave builders, they told me that they had to use all our remaining gold for building the port. Now we don't have any gold left.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[But the problem is that for sending out an expedition we need some gold.]])
      .. paragraphdivider() ..
	  -- TRANSLATORS: Amalea
      _([[Fortunately I recognized there is a mountain which looks very promising to have some mining ressources. Perhaps we should send some geologists to explore the hidden ressources.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[I strongly recommend to build a production chain for mining and melting some gold. I think we need a vineyard, a winery, a tavern, a fisher's house or a hunter's house and a smelting works. And the mines themselves of course.]])
	  .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[If there are difficulties to build all the buildings needed, we should think about all we have learned so far. We can cut all the stone. We should perhaps optimize our roads to free up some building site, or even dismantle some military buildings to get more space for our economy.]]))
      .. new_objectives(obj_produce_gold),
   posy=1,
}

saledus_6 = {
   title=_"We should conquer the whole islands",
   body= saledus3(_"Saledus is excited…",
      -- TRANSLATORS: Saledus
      _([[Now it is time to conquer the islands.]])
      .. paragraphdivider() ..
      _([[Let's finish the barbarians off and provide a nice colony to the caesar of our empire.]]))
      .. new_objectives(obj_conquer_all),
   posy=1,
}

amalea_7 = {
   title=_"Amalea's luck",
   body= amalea2(_"Amalea is really proud of the achievements…",
      -- TRANSLATORS: Amalea
      _([[Lutius we have just solved another problem. We managed to melt enough gold to send out at least 3 expeditions. Perhaps we might even keep some gold to make some jewelry.]])
      .. paragraphdivider() ..
	  -- TRANSLATORS: Amalea
      _([[I'm just kidding. Of course we should spare as much gold as we can for building an Arena and other buildings to train our soldiers.]])),
   posy=1,
}

amalea_8 = {
   title=_"Amalea is pleased",
   body= amalea1(_"Amalea is content that we will manage all our challenges",
      -- TRANSLATORS: Amalea
      _([[Well done so far. I really think we will need this feature quite often to adjust our economy. Remember our industry is just working until the given amount in the economy settings is reached.]])
      .. paragraphdivider() ..
	  -- TRANSLATORS: Amalea
      _([[For example it might be a good idea to produce a reasonable amount of a ware and afterwards dismantle the building to gain space for another building.]])),
   posy=1,
}

amalea_9 = {
   title=_"Amalea shaking head",
   body= amalea3(_"Amalea is reminding ...",
      -- TRANSLATORS: Amalea
      _([[Lutius. I really appreciate that you and Saledus are so concerned about our security. But I think we could use the little space on this island better than building a lot of military buildings.]])
      .. paragraphdivider() ..
	  -- TRANSLATORS: Amalea
      _([[I strongly recommend to dismantle the most of them to be able to build industry buildings. I don't think there is much danger on this island, but if you want to be cautious you can keep some sentries or blockhouses at the waterline.]])
      .. paragraphdivider() ..
	  -- TRANSLATORS: Amalea
      _([[But be careful not to waste any bigger building lots.]])),
   posy=1,
}

amalea_11 = {
   title=_"Amalea excited",
   body= amalea1(_"Amalea is looking forward for exploration ...",
      -- TRANSLATORS: Amalea
      _([[Lutius. We have made so much progress so far. I just got the information that we have completed our third ship. ]])
      .. paragraphdivider() ..
	  -- TRANSLATORS: Amalea
      _([[Now that our fleet is growing and growing we should discover the complete region. Let's try to find more land and eventually get more ressources.]])
      .. paragraphdivider() ..
	  -- TRANSLATORS: Amalea
      _([[Especially it would be very helpful to find more marble either within stone fields or in the mountains to mine it.]]))
	  .. new_objectives(obj_find_all_ports),
   posy=1,
}

amalea_12 = {
   title=_"Amalea is very content",
   body= amalea2(_"Amalea is clapping her hands...",
      -- TRANSLATORS: Amalea
      _([[Lutius. Well done. Well done. We have discovered all possible port locations in the complete region. Now we can backup our economy and we will manage to sail home soon.]])),
   posy=1,
}
   
saledus_7 = {
   title=_"Victory is our's",
   body= saledus3(_"Saledus is cheering out…",
      -- TRANSLATORS: Saledus
      _([[Sire, finally we have defeated the babarians in this region. And furthermore we have build a nice colony for our empire.]])
      .. paragraphdivider() ..
      _([[Caesar will be very pleased about our victory.]])),
   posy=1,
}

saledus_8 = {
   title=_"Pleasing Neptune",
   body= saledus4(_"Saledus is very sad and anxious…",
      -- TRANSLATORS: Saledus
      _([[Sire, we are making good progress to get home eventually. But there is one thing I need to talk about:]])
      .. paragraphdivider() ..
      _([[As we have seen the last time it is of great importance that the gods and especially Neptune are in a good mood to guard our journey.]])
      .. paragraphdivider() ..
      _([[Unfortunately we lost our holy shrine of Neptune in the great storm, so we can't worship him well. I think we should search the whole region to see if we can recover the shrine with all its 5 pieces.]])
      .. paragraphdivider() ..
      _([[To ease up the search I made some models of the pieces as you can see. You can give them to our soldiers and sailors to look for them.]]))
	  .. new_objectives(obj_find_artifacts),
   posy=1,
}

saledus_9 = {
   title=_"Praise, praise, praise",
   body= saledus2(_"Saledus is happily worshipping…",
      -- TRANSLATORS: Saledus
      _([[Sire, we recovered the holy shrine of Neptune. Now we can seek his protection for our journey.]])
      .. paragraphdivider() ..
      _([[Oh great Neptune safeguard our journey. Oh great Neptune let the winds be friendly for our course. Oh great Neptune beware us from the creatures of the sea. Oh great Neptune......]])),
   posy=1,
}

saledus_12 = {
   title=_"Unpleasant discoveries",
   body= saledus3(_"Saledus looks very worried…",
      -- TRANSLATORS: Saledus
      _([[Sire, we found the ruins of a former babarian fortress and village. I'm somewhat worried we will have to fight against them sooner than we wanted.]])),
   posy=1,
}

saledus_10 = {
   title=_"Train faster, train harder",
   body= saledus2(_"Saledus is demanding some discipline",
      -- TRANSLATORS: Saledus
      _([[Sire, we have all buildings available to train our soldiers but we should increase our efforts to train them to the highest level and to train more of them.]])
      .. paragraphdivider() ..
      _([[We need a lot of food, a lot of iron and a lot of gold to do so. So we have lots of challenges to cope with.]])),
   posy=1,
}

saledus_11 = {
   title=_"Training infrastructure is needed",
   body= saledus3(_"Saledus has security concerns",
      -- TRANSLATORS: Saledus
      _([[Sire, we need to train some of our people to serve in your great army. It is important to increase the number of our soldiers as fast as we can. And we should train them well to improve their fighting abilities.]])
      .. paragraphdivider() ..
      _([[Therefore we need to build a full training infrastructure despite the limited space available. We need some barracks, a trainingcamp and an arena or a colosseum.]]))
	  .. new_objectives(obj_training),
   posy=1,
}



diary_page_5 = {
   title=_"Now it's time to get home again",
   body= lutius(_"Diary of Lutius",
      -- TRANSLATORS: Lutius - Diary
      _([[Yes my fellow comrades, we have done it. We managed all the challenges that were put on us. But my heart is still with our home. We shall set sail as soon as possible to help defend the empire from the babarian tribe.]])
	  .. paragraphdivider() ..
      _([[You have completed this mission. You may continue playing if you wish, otherwise move on to the next mission.]])),
   posy=1,
}


diary_page_6 = {
   title=_"Artifact found",
   body= lutius(_"Diary of Lutius",
      -- TRANSLATORS: Lutius - Diary
      _([[Finally we found a piece of our shrine. Praise Neptune.]])),
   posy=1,
}