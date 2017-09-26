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
function vesta(title, text)
   return speech("map:Vesta.png", "757511", title, text)
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
       listitem_bullet(_[[We should dismantle all unproductive small buildings and the farms to get some ressources for new buildings. Remember to check the messages and the building stats which buildings are unproductive.]])
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
      listitem_bullet(_[[We need to recover our plans how to construct a farm. We should search for them in the hills east of our border.]])
   ),
}

obj_build_quarries_and_lumberjacks = {
   name = "build_quarries_and_lumberjacks",
   title=_"Build at least 3 lumberjack’s houses and 2 quarries to renew our building material supply chain",
   number = 1,
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
   name = "replace_foresters",
   title=_"Replace our old and uneffective foresters",
   number = 1,
   body = objective_text(_"Replace our foresters by new ones",
      listitem_bullet(_[[Our two forester's houses are old and weared out. We should replace them by new ones to increase productivity.]])
   ),
}

obj_find_monastry = {
   name = "find_monastry",
   title=_"Find the monastry in the north",
   number = 1,
   body = objective_text(_"Find the monastry",
      listitem_bullet(_[[Find the monastry in the north to obtain the improved technology for our wheat industry chain.]])),
}

obj_deal_with_vesta = {
   name = "deal_with_vesta",
   title=_"Deal with Vesta to get the technology",
   number = 1,
   body = objective_text(_"Diplomacy: Trade or War",
      listitem_bullet(_[[We need to get the improvements very soon. But we have to decide: either we collect 35 wheat and 15 wine for Vesta  in one of our warehouses or we just conquer the monastry.]])),
}

obj_heroes = {
   name = "train_heroes",
   title=_"Train at least 3 heroes",
   number = 1,
   body = objective_text(_"Training of heroes",
      listitem_bullet(_[[Use our ressources wisely to train at least 3 heroes.]])),
}

obj_produce_tools = {
   name = "produce_tools",
   title=_"Produce at least 10 tools",
   number = 1,
   body = objective_text(_"Produce 10  tools for the beginning",
      listitem_bullet(_[[We need to produce additional tools to improve our economy. For the beginning start with at least 10 of them. Remember the possibility to control the production in the economy settings.]])),
}

obj_recruit_soldiers = {
   name = "recruit_soldiers",
   title=_"Recruit new soldiers",
   number = 1,
   body = objective_text(_"Recruiting new soldiers",
      listitem_bullet(_[[Start to recruit new soldiers in our barracks. For the beginning recruit 10 of them at least. Don' forget you need to forge weapons and armour for them.]])
   ),
}

obj_conquer_all = {
   name = "conquer_all",
   title=_"Defeat the barbarians",
   number = 1,
   body = objective_text(_"Defeat the enemy",
      listitem_bullet(_[[Now it is time to end the barbarian aggression in our very own part of the world.]])
   ),
}

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

obj_training = {
   name = "train_your_soldiers",
   title=_"Increase your miltitary strength by training your soldiers",
   number = 1,
   body = objective_text(_"Training is important",
      listitem_bullet(_[[Train your soldiers hard and train them fast. We need to increase our military strength.]])
   ),
}


-- ==================
-- Texts to the user
-- ==================

-- Lutius diary

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

-- diary_page_4 = {
   -- title=_"Seafaring possible again",
   -- body= lutius(_"Diary of Lutius",
      -- -- TRANSLATORS: Lutius - Diary
      -- _([[Praise the gods. We finally have a ship industry.]])),
   -- posy=1,
-- }

diary_page_5 = {
   title=_"Battle is won",
   body= lutius(_"Diary of Lutius",
      -- TRANSLATORS: Lutius - Diary
      _([[Yes my fellow comrades, we have done it. We managed all the challenges that were imposed on us. And not to forget we have expelled the Babarians out of our country. But this was only a battle that was won, the war is even yet to begin. Let's see what the future will bring to us.]])
	  .. paragraphdivider() ..
      _([[You have completed this mission. You may continue playing if you wish, otherwise move on to the next mission.]])),
   posy=1,
}

-- Amalea

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

amalea_1 = {
   title=_"Amalea investigating",
   body= amalea2(_"Amalea is nodding thougtful…",
      -- TRANSLATORS: Amalea
      _([[Lutius in my opinion this will be a very difficult challenge again. But I'm afraid, we're doomed to manage this situation. To make things even worse I had a look into our warehouses. There is hardly some grain of dust left in there. No wares, no tools, no workers and no soldiers as well.]])
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
      _([[So we need to recover our construction plans for a farm. One older constructor told me that they might have been concealed in a cave in the hills east of our border.]]))
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
      .. new_objectives(obj_build_quarries_and_lumberjacks, obj_clear_roads),
	  --.. new_objectives(obj_clear_roads),
   posy=1,
   h=500,
}

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

amalea_9 = {
   title=_"Amalea shaking head",
   body= amalea3(_"Amalea is looking defeatist ...",
      -- TRANSLATORS: Amalea
      _([[Alright Lutius. Here is another problem. After the production of some beer and some flour I found out that the technology in the mill and the brewery is somewhat outdated. by this they consume far to much ressources.]])
      .. paragraphdivider() ..
	  -- TRANSLATORS: Amalea
      _([[And you wouldn't believe it nobody knows how to improve the technology nor how to build more efficient buildings. The only thing I found out is that there might be a monastry in the north were the female priests might know how to improve our technology.]])
      .. paragraphdivider() ..
	  -- TRANSLATORS: Amalea
      _([[But I'm not sure if they will give us their knowledge for free. Anyway we have no choice we need to find them to improve our economy.]]))
	  .. new_objectives(obj_find_monastry),
   posy=1,
   h=500,
}

amalea_10 = {
   title=_"Amalea looks happy",
   body= amalea2(_"Amalea comes in…",
      -- TRANSLATORS: Amalea
      _([[Lutius. I think this is rather a fair offer. And we could use some good karma from any of our gods as well.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[The only problem might be that it will take some time to collect all the wares. Especially we will need to shorten the supply to our mines drastically, which gives us a drawback in metal production.]])),
   posy=1,
}


amalea_11 = {
   title=_"Amalea sad",
   body= amalea1(_"Amalea is really sad ...",
      -- TRANSLATORS: Amalea
      _([[Lutius. Yes we have obtained the plans but we will never know if and how Vesta and her sisters could have helped us against the babarians.]])
      .. paragraphdivider() ..
	  -- TRANSLATORS: Amalea
      _([[Furthermore we will have to live with the guilt on our souls to have destroyed a temple of an empire god.]])),
   posy=1,
}

amalea_12 = {
   title=_"Amalea is very content",
   body= amalea2(_"Amalea is clapping her hands...",
      -- TRANSLATORS: Amalea
      _([[Lutius. Well done. Now we are able to build more effective buildings to refine our wheat. And best of all we have a new ally who just provided us with lots of water, flour and beer. Now I really think that nothing could stop us to get stronger and conquer back our homeland.]])),
   posy=1,
}

amalea_13 = {
   title=_"Amalea satisfied",
   body= amalea3(_"Amalea is satisfied with the progress ...",
      -- TRANSLATORS: Amalea
      _([[Alright Lutius another problem solved. Still more to come. As we are able to produce food now we should urgently start to mine some coal and iron ore. After all we need more tools to back up our economy and built more and different production sites.]])
      .. paragraphdivider() ..
	  -- TRANSLATORS: Amalea
      _([[But due to recent experience I assume that something could be wrong with our mines as well. Probably it would be a good idea to send a geologist to check whether there are enough ressources in the vicinity of our mines.]])
      .. paragraphdivider() ..
	  -- TRANSLATORS: Amalea
      _([[As soon as we have melted some iron we should start to produce tools. Perhaps we should control the production of them via the economy settings. In the meantime we could start to build the buildings we need most urgently. I think we miss a vineyard yet in our economy.]]))
	  .. new_objectives(obj_produce_tools),
   posy=1,
   h=500,
}
   
-- Saledus

saledus = {
    title=_"Military Strength",
    body= saledus2(_"Saledus looks very relaxed…",
       -- TRANSLATORS: Saledus
       _([[Sire, you are right it is a great pleasure to be at home again. And the best of all things: I can see we still have some military strength. I can count five towers and a fortress in our vicinity to guard us.]])),
    posy=1,
}

saledus_1 = {
   title=_"We could use the military instead",
   body= saledus2(_"Saledus raises his hand…",
      -- TRANSLATORS: Saledus
      _([[Sire, If we need to get the technology very quickly why not conquer it. We could defeat the little monastry in an instant.]])
	  .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[The only problem might be that probably the gods will not be that amused if we destroy a temple of them. And you never know what this could lead to.]]))
      .. new_objectives(obj_deal_with_vesta),  
   posy=1,
   h=500,
}

saledus_2 = {
   title=_"Easy victory",
   body= saledus3(_"Saledus cheering proudly…",
      -- TRANSLATORS: Saledus
      _([[Sire, our armies just swept over the priests and conquered the plans. That was rather an easy victory]])),
   posy=1,
}

saledus_3 = {
   title=_"Defiance",
   body= saledus2(_"Saledus is keeping his point…",
      -- TRANSLATORS: Saledus
      _([[Sire, I admit this has been proven to be a very good deal. But we could have had the plans much earlier. Anyhow we will not be able to make any deals with the babarians, so we better keep our soldiers in good mood and train them adequately.]])),
   posy=1,
}

saledus_4 = {
   title=_"Pride",
   body= saledus2(_"Saledus is keeping his point…",
      -- TRANSLATORS: Saledus
      _([[Sire, I admit that we have accumulated some bad karma, but in wartime sometimes the end justifies the means. And we won't be able to make any deals with the babarians either, so we better keep our soldiers in good mood and train them adequately.]])),
   posy=1,
}


saledus_5 = {
   title=_" Military Strength",
   body= saledus2(_"Saledus is asking for a stronger army …",
      -- TRANSLATORS: Saledus
      _([[Now that we have produced some tools, I think it is time to diverge some of our iron and coal to start some military production.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[I am really worried about the weakness of our army. We should start to inbcrease our military power. For the beginning we should start to recruit at least 10 new soldiers.]])
	  .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[Remember we need to produce wooden spears and helmets to recruit them in the barracks. Perhaps we could start to train them in the Arena as well. Probably you would even think of enhancing the Arena to a Colloseum first.]]))
	  .. new_objectives(obj_recruit_soldiers),
   posy=1,
   h=500,
}

saledus_6 = {
   title=_"Training is needed",
   body= saledus3(_"Saledus still has security concerns",
      -- TRANSLATORS: Saledus
      _([[Sire, although we have started to recruit new soldiers we still need to train them well. It is important to increase the strength of our soldiers as fast as we can.]])
      .. paragraphdivider() ..
      _([[Unfortunately we only have an arena and a very old and small training camp with very little storage capacity. And guess what none of our builders knows how to improve this. But we need to use what we have to get prepared for battle.]]))
	  .. new_objectives(obj_training),
   posy=1,
   h=500,
}

saledus_7 = {
   title=_"Praise the army of the empire",
   body= saledus2(_"Saledus is happy…",
      -- TRANSLATORS: Saledus
      _([[Sire, after training a bunch of recruits our trainers together with our builders developed better and more efficient training buildings.]])
      .. paragraphdivider() ..
      _([[Now we could really improve our army and build the military strength that we will need to defend our country.]])),
   posy=1,
}

saledus_8 = {
   title=_"We just need another hero",
   body= saledus4(_"Saledus is in a good mood…",
      -- TRANSLATORS: Saledus
      _([[Sire, I have good progress to report. After we now have more and better beer together with enhanced training facilities we should be able to train as much heroes as we can.]])
      .. paragraphdivider() ..
      _([[It will give us great advantage in battle if our soldiers are much more powerful than the babarians. So we need to spend our ressources wisely to get heroes as soon as we can.]])
      .. paragraphdivider() ..
      _([[I would say for the beginning 3 fully trained heroes would be sufficient. But don't forget to send them to the frontline, cause back at home they are not that worthy.]]))
	  .. new_objectives(obj_heroes),
   posy=1,
   h=500,
}

saledus_9 = {
   title=_"We should expell the babarians",
   body= saledus3(_"Saledus is excited…",
      -- TRANSLATORS: Saledus
      _([[Now that we have some fully trained soldiers it is time to expell the babarians of our homeland.]])
      .. paragraphdivider() ..
      _([[Let's finish them off and regain control over our lands. They should regret deeply they ever came.]]))
      .. new_objectives(obj_conquer_all),
   posy=1,
   h=500,
}   
   
saledus_10 = {
   title=_"Victory is our's",
   body= saledus2(_"Saledus is cheering out…",
      -- TRANSLATORS: Saledus
      _([[Sire, finally we have defeated the babarians. We have expelled even the last of them. May they never come back.]])
      .. paragraphdivider() ..
      _([[Now it is time to find out why this big mess around could have happened. But first of all we should have a beer or two to celebrate our victory and our newfound freedom.]])),
   posy=1,we
}

-- saledus_10 = {
   -- title=_"Train faster, train harder",
   -- body= saledus2(_"Saledus is demanding some discipline",
      -- -- TRANSLATORS: Saledus
      -- _([[Sire, we have all buildings available to train our soldiers but we should increase our efforts to train them to the highest level and to train more of them.]])
      -- .. paragraphdivider() ..
      -- _([[We need a lot of food, a lot of iron and a lot of gold to do so. So we have lots of challenges to cope with.]])),
   -- posy=1,
-- }



-- Vesta

vesta_0 = {
   title=_"Worship to be gifted",
   body= vesta(_"Vesta is demanding a sacrifice for the gods...",
      -- TRANSLATORS: Vesta
      _([[Ave Lutius. If you want us to help you, you first have to worship our gods. Therefore deliver 35 wheat and 15 wine to prepare a worthy sacrifice for our gods.]])),
   posy=1,
}

vesta_1 = {
   title=_"May the gods bless you",
   body= vesta(_"Vesta is blessing us...",
      -- TRANSLATORS: Vesta
      _([[Ave Lutius. You have served our gods well. You will not only be gifted with the improvements to your technology but with some goods that you could use so daringly as well. Furthermore we will pray for you and join your party to safeguard our land from the babarians.]])),
   posy=1,
}

vesta_2 = {
   title=_"Damn you...",
   body= vesta(_"Vesta is cursing us...",
      -- TRANSLATORS: Vesta
      _([[Damn you Lutius, for killing peacefull sisters of the goddess vesta. May your life and your land be cursed and may the wrath of the goods diminish your family from the earth.]])),
   posy=1,
}




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