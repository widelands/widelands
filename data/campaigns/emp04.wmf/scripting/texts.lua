-- =========================
-- Some formating functions
-- =========================

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
function julia(title, text)
   return speech("map:Julia.png", "D0D0D0", title, text)
end
function marcus(title, text)
   return speech("map:Markus.png", "1175AF", title, text)
end


-- =======================================================================
--                           Texts - No logic here
-- =======================================================================

-- ===========
-- objectives
-- ===========
obj_dismantle_buildings = {
   name = "dismantle_unproductive_buildings",
   title=_"Dismantle the unproductive buildings",
   number = 1,
   body = objective_text(_"Dismantle Buildings",
      li(_[[Dismantle all unproductive small buildings to get some resources for new buildings.]]) ..
      li_arrow(_[[Remember to check the messages and the building status labels for unproductive buildings.]])
   ),
}

obj_clear_roads = {
   name = "clear_roads",
   title=_"Clear all unnecessary roads",
   number = 1,
   body = objective_text(_"Clear Road Network",
      li(_[[Resolve the chaotic road network by clearing all unnecessary roads.]])
   ),
}

obj_clear_roads_hint = rt(objective_text(_"Clear Road Network",
   li(_[[Resolve the chaotic road network by clearing all unnecessary roads.]]) ..
   li_arrow(_[[Ensure there are not more than 3 dead ends, nor more than one flag with more than 4 roads.]])
   ))

obj_click_farmbuilding = {
   name = "click_farm_building",
   title=_"Click on one of the farms",
   number = 1,
   body = objective_text(_"Open the Building Window of a Farm",
      li(_[[Click on a farm building to open its building window. This will let you analyze the building.]])
   ),
}

obj_find_farm_plans = {
   name = "find_farm_construction_plans",
   title=_"Find the construction plans for the farms",
   number = 1,
   body = objective_text(_"Find Farm Construction Plans",
      li(_[[Recover the construction plans for the farms. Search for them in the hills east of your border.]])
   ),
}

obj_build_quarries_and_lumberjacks = {
   name = "build_quarries_and_lumberjacks",
   title=_"Build at least 3 lumberjack’s houses and 2 quarries",
   number = 1,
   body = objective_text(_"Quarries and Lumberjacks",
      li(_[[Build at least three lumberjack’s houses and two quarries to renew your building material supply chain.]])
   ),
}

obj_produce_fish = {
   name = "produce_fish",
   title=_"Produce fish and rations",
   number = 1,
   body = objective_text(_"Food Production",
      li(_[[Find and catch some fish. Afterwards, produce rations for your miners.]])
   ),
}

obj_replace_foresters = {
   name = "replace_foresters",
   title=_"Replace the old and ineffective forester’s houses",
   number = 1,
   body = objective_text(_"Build Two New Forester’s Houses",
      li(_[[Replace the two forester’s houses by new ones to increase productivity.]])
   ),
}

obj_find_monastery = {
   name = "find_monastery",
   title=_"Find the monastery in the north",
   number = 1,
   body = objective_text(_"Find the Monastery",
      li(_[[Find the monastery in the north to obtain the improved technology for your wheat production chain.]])
   ),
}

obj_deal_with_julia = {
   name = "deal_with_julia",
   title=_"Deal with Julia to get the technology",
   number = 1,
   body = objective_text(_"Diplomacy: Trade or War",
      li(_[[Decide: Either collect 35 sheaves of wheat and 15 amphoras of wine for the goddess in your headquarters or conquer the monastery.]])
   ),
}

obj_heroes = {
   name = "train_heroes",
   title=_"Train at least 3 heroes",
   number = 1,
   body = objective_text(_"Hero Training",
      li(_[[Use your resources wisely to train at least three fully promoted heroes.]]) ..
      li_arrow(_[[To train a soldier to the highest level in one site, consider stopping the trainingsite until it is fully equipped with all food and weapons needed.]]) ..
      li_arrow(_[[To prevent waste of goods, reduce the soldiers to be trained to one per site.]]) ..
      li_arrow(_[[A site will always prefer the best suited soldier available for training. Make sure you don’t have your half trained soldiers garrisoned in a military building.]])
   ),
}

obj_produce_tools = {
   name = "produce_tools",
   title=_"Produce at least 10 tools",
   number = 1,
   body = objective_text(_"Tool Production",
      li(_[[Produce at least ten tools to improve your economy.]]) ..
      li_arrow(_[[Remember that you can control the production amount in the economy settings.]])
   ),
}

obj_recruit_soldiers = {
   name = "recruit_soldiers",
   title=_"Recruit new soldiers",
   number = 1,
   body = objective_text(_"New Soldier Recruiting",
      li(_[[Recruit at least ten new soldiers in your barracks.]])
   ),
}

obj_conquer_all = {
   name = "conquer_all",
   title=_"Defeat the Barbarians",
   number = 1,
   body = objective_text(_"Defeat the Enemy",
      li(_[[End the Barbarian intrusion into your very own part of the world.]])
   ),
}

obj_charcoal = {
   name = "charcoal",
   title=_"Build 2 charcoal kilns",
   number = 1,
   body = objective_text(_"Build Two Charcoal Kilns",
      li(_[[Build two charcoal kilns to support the iron industry.]]) ..
      li_arrow(_[[Remember to ensure a constant log supply for them.]])
   ),
}

obj_training = {
   name = "train_your_soldiers",
   title=_"Increase your military strength by training your soldiers",
   number = 1,
   body = objective_text(_"Training is Important",
      li(_[[Train your soldiers hard and train them fast.]]) ..
      li_arrow(_[[You need to increase your military strength.]])
   ),
}

obj_upgrade = {
   name = "upgrade",
   title=_"Build a training camp and enhance the arena",
   number = 1,
   body = objective_text(_"More Efficient Training Buildings",
      li(_[[Build a training camp and enhance your arena to a colosseum.]])
   ),
}

-- ==================
-- Texts to the user
-- ==================

-- Diary of Lutius

diary_page_1 = {
   title =_"Home, Sweet Home",
   body=lutius(_"Diary of Lutius",
      -- TRANSLATORS: Lutius - Diary
      _([[Finally, we managed to reach home. I am so glad to see our beautiful country again. I am really looking forward to a walk on our lovely coast and to hunting in our deep forests.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[I expect that Saledus and Amalea are deeply delighted as well.]])),
   posy=1,
}

diary_page_2= {
   title=_"Tools, Tools, Tools",
   body= lutius(_"Diary of Lutius",
      -- TRANSLATORS: Lutius - Diary
      _([[Praise the gods! We just ensured a constant tool supply. Now we should be able to expand our economy.]])),
   posy=1,
}

diary_page_3= {
   title=_"Military Strength",
   body= lutius(_"Diary of Lutius",
      -- TRANSLATORS: Lutius - Diary
      _([[Thank the gods! Now we have everything prepared to fully train our soldiers. We can increase our military strength at last.]])),
   posy=1,
}

diary_page_4 = {
   title=_"The Battle is Won",
   body= lutius(_"Diary of Lutius",
      -- TRANSLATORS: Lutius - Diary
      _([[Praise the gods, we have succeeded. We managed all the challenges that were imposed on us. We even expelled the Barbarians out of our country. However, our victory is only temporary, as this is still just the beginning of the war. Let’s see what the future still holds for us.]]))
      .. objective_text(_"Victory",
      _[[You have completed this mission. You may continue playing if you wish, otherwise move on to the next mission.]]),
   posy=1,
   allow_next_scenario = true,
}


-- Lutius

lutius_1 = {
   title=_"Chaos",
   body= lutius(_"Lutius is disappointed",
      -- TRANSLATORS: Lutius
      _([[Oh no! Amalea is right. In fact, I can’t see any productivity at all. And the road network seems to be completely in shambles as well. Who might be responsible for this chaos?]])),
   posy=1,
}

lutius_2 = {
   title=_"Explanation Needed",
   body= lutius(_"Lutius greets the official",
      -- TRANSLATORS: Lutius
      _([[Ave! Who are you and what happened to our beautiful land?]])),
   posy=1,
}

lutius_3 = {
   title=_"Difficult Times",
   body= lutius(_"Lutius accepts the challenge",
      -- TRANSLATORS: Lutius
      _([[Mayor, we have to thank you for your efforts to safeguard our city. And we will do our very best to recover from the chaos. But unfortunately, this seems to be very difficult.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius
      _([[I am really afraid, because we are completely unprepared should the Barbarians decide to attack us.]])),
   posy=1,
}


-- Marcus (mayor of Fremil)

marcus_1 = {
   title=_"Welcome Back",
   body= marcus(_"A high Fremil official is welcoming you…",
      -- TRANSLATORS: Marcus - Mayor of Fremil welcoming Lutius and explaining the chaos
      _([[Ave, Sire! The people and me are so glad to see you returning back home. We really need some good leadership around here.]])
      .. paragraphdivider() ..
      _([[As you have already noticed, things have gone terribly wrong around here since you left.]])),
   posy=1,
}

marcus_2 = {
   title=_"A Long Story",
   body= marcus(_"The official sighs deeply…",
      -- TRANSLATORS: Marcus - Mayor of Fremil welcoming Lutius and explaining the chaos
      _([[Sire, let me start from the beginning. The king left Fremil a long time ago to fight the Barbarians. As this duty was demanding his full commitment, he delegated the authority of running the city to his former secretary and instated him as his surrogate around here.]])
      .. paragraphdivider() ..
      _([[But this was one of his worst decisions ever. The secretary got blinded by his new power. His selfish instincts were as awful as his complete stupidity and inability to govern the city.]])
      .. paragraphdivider() ..
      _([[After a while, the people discovered the truth and expelled the utter fool. Afterwards, they elected me as mayor of this city. However, recovering from the chaos seems to be a task too big for my abilities.]])),
   posy=1,
}


-- Amalea

amalea = {
   title=_"Amalea Looks Puzzled",
   body= amalea1(_"Amalea is doubtful…",
      -- TRANSLATORS: Amalea
      _([[Ave, Saledus! On the one hand you are right, it really is a delight to see our homeland again. But on the other hand, I have the impression that something went deeply wrong here.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[Have a look at the economy. There is hardly any productivity at all. Whoever managed our country while we were absent created utter chaos. I’m not quite sure how we can fix this, if at all possible.]])),
   posy=1,
}

amalea_1 = {
   title=_"Amalea Investigates",
   body= amalea2(_"Amalea is nodding thoughtfully…",
      -- TRANSLATORS: Amalea
      _([[Lutius, in my opinion this will again be a very difficult challenge. But I’m afraid that we’re doomed to manage this situation. To make things even worse, I was met with a nasty surprise at our warehouses: they’re all empty. You can hardly find a grain of dust left in there: no wares, no tools, no workers and no soldiers either.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[So, first of all we need some building materials to start correcting the mistakes made by the infamous secretary. I think we should try dismantling unproductive small buildings to recover some building materials from them and collect them in our headquarters. As far as I can see now, the fishermen’s houses and the quarries don’t have any resources near them. All of the lumberjacks’ houses and the wells seem also to be inefficient or worn out.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[For the other buildings we can’t tell yet, if they are working properly. We need to closely monitor them when we are able to supply them. Until then we shouldn’t do anything with them.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[Except, maybe we should restrict the input of all buildings which consume any of our building materials to zero. Or maybe we could even pause the production in all bigger buildings and get the workers some rest until we have produced some of their input wares.]]))
      .. new_objectives(obj_dismantle_buildings),
   posy=1,
   h=500,
}

amalea_2 = {
   title=_"Amalea has Bad News",
   body= amalea3(_"Amalea recommends…",
      -- TRANSLATORS: Amalea
      _([[Brother, I’m really worried that I have to deliver bad news again. As you can see, our farms aren’t producing anything and we can’t dismantle them.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[This situation was caused by the sad fact that our people lost the instructions on how to construct and operate farms. Therefore they have zero productivity and the constructors don’t know how to dismantle them either.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[So, we urgently need to recover the plans regarding the construction and operation of farms. One older constructor told me that they might have been concealed in a cave in the hills east of our border.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[Until we have found the plans, the only option for our farms is to destroy them.]]))
      .. new_objectives(obj_find_farm_plans),
   posy=1,
   h=500,
}

amalea_3 = {
   title=_"Amalea Looks Confident",
   body= amalea2(_"Amalea is more confident",
      -- TRANSLATORS: Amalea
      _([[Lutius, now we are getting somewhere. As we have gained some construction materials, we can start to rebuild our economy.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[First of all, we need more construction materials. So, we should build at least three lumberjacks’ houses and two quarries. Be sure to have dismantled all of the unproductive buildings though.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[Oh, before I forget, there is another task to accomplish. While our builders are doing their job, somebody should clear up our road network. They are leaving us no space to place the buildings that we need.]]))
      .. new_objectives(obj_build_quarries_and_lumberjacks, obj_clear_roads),
   posy=1,
   h=500,
}

amalea_4 = {
   title=_"Amalea is Somewhat Relieved",
   body= amalea2(_"Amalea is giving a deep sigh…",
      -- TRANSLATORS: Amalea
      _([[Praise the gods, Lutius! We have found the plans on how to build and operate farms.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[Now we can start building farms to produce the beer which our miners need so desperately. Furthermore, the plans have enabled us to upgrade our old farms to get to work again.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[But I’m afraid that this problem hasn’t been the last in our economy.]])),
   posy=1,
   h=500,
}

amalea_5 = {
   title=_"Amalea has Good News",
   body= amalea2(_"Amalea smiles for the first time in weeks…",
      -- TRANSLATORS: Amalea
      _([[Lutius, well done so far. I just got the news that we have finished the basic buildings to obtain some construction materials.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[This is offering us more options to get the shambles in our economy solved.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[I really think we can start to be hopeful about our future now. I pray that we can make ourselves comfortable in our homeland again.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[But this will be some hard work still. For now, we should start by building some houses for our fishermen. As all the fish has been caught at our coast, we should try our luck in the eastern part of our territory.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[After we have managed this, we can start making rations in our tavern.]]))
      .. new_objectives(obj_produce_fish),
   posy=1,
   h=500,
}

amalea_6 = {
   title=_"Amalea is Pleased",
   body= amalea3(_"Amalea is nodding her head…",
      -- TRANSLATORS: Amalea
      _([[Well done, well done. Our road network looks a lot more structured than before.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[Now we can focus on rebuilding our economy.]])),
   posy=1,
   h=400,
}

amalea_7 = {
   title=_"Amalea Shakes Her Head",
   body= amalea2(_"Amalea is getting fed up with all the problems in this economy…",
      -- TRANSLATORS: Amalea
      _([[For the sake of Neptune, I just discovered another problem! It seems that really very few things are working as expected in this economy.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[One of our lumberjacks told me that the reproduction of our forests is far behind his experience and expectations. So, I took a close look at our foresters’ performance.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[And guess what? They are very old. Their houses and tools are worn and their seed is degenerated. For this reason, they need much more time than usual for planting trees.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[The only solution is to build two new forester’s houses near our lumberjacks. Be sure to first build a new forester’s house and then eventually destroy the old one or at least expel the forester to change houses.]]))
      .. new_objectives(obj_replace_foresters),
   posy=1,
   h=500,
}

amalea_8 = {
   title=_"Amalea Laughs Sarcastically",
   body= amalea1(_"Amalea is laughing sarcastically…",
      -- TRANSLATORS: Amalea
      _([[Well, Lutius, we have just solved another weird behavior in our economy. Now our lumberjacks should be supplied with enough trees to enhance our economy.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[I am really very curious about what will go wrong next!]])),
   posy=1,
}

amalea_9 = {
   title=_"Amalea Shrugs",
   body= amalea3(_"Amalea is getting used to bad news…",
      -- TRANSLATORS: Amalea
      _([[Alright Lutius, here is another problem: after the production of some beer and some flour, I realized that the technology that we are using in our mills and breweries is somewhat outdated. This way, they are consuming far too many resources.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[And you wouldn’t believe it, but nobody knows how to improve the technology or how to build more efficient buildings. I only heard some rumors about a monastery in the north where the priestesses are supposed to possess some knowledge about improved technologies.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[But I’m not sure if they will share their knowledge with us for free. Anyway, we have no choice. We need to find them to improve our economy.]]))
      .. new_objectives(obj_find_monastery),
   posy=1,
   h=500,
}

amalea_10 = {
   title=_"Amalea Looks Happy",
   body= amalea2(_"Amalea comes in…",
      -- TRANSLATORS: Amalea
      _([[Brother, I think this is a rather fair offer. And some good will from any of our gods could be very helpful as well.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[The only problem might be that it will take significant time to collect all the wares. In particular, we will need to shorten the supply to our mines drastically. This will give us a real drawback in metal production.]])),
   posy=1,
}


amalea_11 = {
   title=_"Amalea Looks Sad",
   body= amalea1(_"Amalea is really sad…",
      -- TRANSLATORS: Amalea
      _([[I’m not sure if that was the right thing to do. Yes, we have obtained the plans. But although we are now able to build and upgrade our mills and breweries with the improved technology, we will never know if and how Vesta and her priestesses could have helped us against the Barbarians.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[Furthermore, we will have to live with the guilt of destroying a temple of an Empire goddess on our souls.]])),
   posy=1,
}

amalea_12 = {
   title=_"Amalea is Very Content",
   body= amalea2(_"Amalea claps her hands…",
      -- TRANSLATORS: Amalea
      _([[Well done. Now we are able to build more efficient buildings to refine our wheat. Furthermore, the plans enable us to upgrade our current mill and brewery with the improved technology.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[And, best of all, we have a new ally who just provided us with lots of water, flour and beer. Now I really think that nothing can prevent us from getting stronger and taking back our homeland.]])),
   posy=1,
}

amalea_13 = {
   title=_"Amalea is Satisfied",
   body= amalea3(_"Amalea is satisfied with the progress…",
      -- TRANSLATORS: Amalea
      _([[Alright Lutius, another problem solved. Still more to come. As we are able to produce food now, we should start mining some coal and iron ore immediately. After all, we need more tools to get our economy back up and build more and different production sites.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[But according to our recent experiences, I assume that something will be wrong with our mines as well. It’s probably a good idea to send a geologist to check whether there are enough resources in the vicinity of our mines.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[As soon as we have melted some iron, we need to start producing tools. Perhaps we should control the production of them via the economy settings. In the meantime, we can start raising the buildings that we need most urgently. I think we are still lacking a vineyard in our economy.]]))
      .. new_objectives(obj_produce_tools),
   posy=1,
   h=500,
}

amalea_14 = {
   title=_"Amalea has Some Advice",
   body= amalea3(_"Amalea is providing economic advice…",
      -- TRANSLATORS: Amalea
      _([[Lutius, it seems that our coal supply is a little weak. So, we need to expand and explore all mineable areas for more coal.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[In the meantime, it might help to build a charcoal kiln for buffering any mining shortfalls with charcoal. But be careful to ensure a continuous log supply afterwards.]]))
      .. new_objectives(obj_charcoal),
   posy=1,
   h=500,
}

amalea_15 = {
   title=_"Amalea is in a Good Mood",
   body= amalea2(_"Amalea is celebrating success…",
      -- TRANSLATORS: Amalea
      _([[Great, another issue solved! Now the charcoal will support our metal production and we can concentrate more on exploration and military strength.]])),
   posy=1,
}

amalea_16 = {
   title=_"Amalea Looks Thoughtful",
   body= amalea2(_"Amalea is sorrowful…",
      -- TRANSLATORS: Amalea
      _([[Dear brother, I have bad news to report. One of our buildings has just been destroyed by an uncontrolled kitchen fire. I fear this might be a sign of the goddess Vesta still being in a bad mood.]])),
   posy=1,
}

amalea_17 = {
   title=_"Amalea is Positively Surprised",
   body= amalea2(_"Amalea is celebrating a happy event…",
      -- TRANSLATORS: Amalea
      _([[Cheers Lutius, I don’t know how, but we have been gifted with some beer and wine. We found the additional wares while we were taking stock recently. Maybe the goddess Vesta is still supporting us.]])),
   posy=1,
}

amalea_18 = {
   title=_"Amalea Analyzes the Economy",
   body= amalea1(_"Amalea reminds Lutius of the farms…",
      -- TRANSLATORS: Amalea
      _([[Look Lutius, I have just analyzed our economy somewhat further. I think there might be a problem with our farms. They don’t show any productivity although there is enough space to plant wheat.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[I think we should have a deeper look into the issue and visit one of them.]]))
      .. new_objectives(obj_click_farmbuilding),
   posy=1,
   h=500,
}

amalea_19 = {
   title=_"Amalea’s Restrictions",
   body= amalea1(_"Amalea restricts the building possibilities…",
      -- TRANSLATORS: Amalea
      _([[Excuse me Lutius, but I think the most critical resources in the current state of our economy are logs.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[Therefore I took the freedom to restrict our current building options to the lumberjack’s house until we have gained enough building materials by dismantling the ineffective buildings.]])),
   posy=1,
}

amalea_20 = {
   title=_"Amalea Reminds to Clear Road Network",
   body= amalea3(_"Amalea reminds Lutius of the scrambled road network…",
      -- TRANSLATORS: Amalea
      _([[Look Brother, I have just realized we haven’t cleared the road network yet. I think we should do so very quickly to free enough space for new buildings.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[To achieve this, we should make sure that there aren’t more than three dead ends (flags with only one road) and not more than one flag with more than four roads attached.]])),
   posy=1,
}

amalea_21 = {
   title=_"Defeated!",
   body= amalea2(_"Amalea reports our defeat…",
      -- TRANSLATORS: Amalea
      _([[Oh no Lutius, I don’t know how this could have happened, but the Barbarians have sacked our headquarters. So, we have lost this battle and our empire!]])),
   posy=1,
}

amalea_22 = {
   title=_"Amalea has Bad News",
   body= amalea2(_"Amalea reports our headquarters lost…",
      -- TRANSLATORS: Amalea
      _([[Oh no Lutius, I don’t know how this could have happened, but the Barbarians have destroyed our headquarters. So, we can’t deliver the wares to Julia anymore.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[Now we need to try to fulfil our duties without their technology. But this will not be an easy task at all.]])),
   posy=1,
}

amalea_23 = {
   title=_"Defeated!",
   body= amalea2(_"Amalea reports our defeat…",
      -- TRANSLATORS: Amalea
      _([[Oh no Lutius, I don’t know how this could have happened, but the Barbarians have destroyed our last warehouse. So, we have lost this battle and our empire!]])),
   posy=1,
}

amalea_24 = {
   title=_"Amalea in a Hurry",
   body= amalea1(_"Amalea is very busy…",
      -- TRANSLATORS: Amalea
      _([[Brother, as you can see, a lot of things seem to be wrong in this economy. I’ll be very busy to examine everything, so I won’t be here to provide as detailed advice as I used to do.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[But I am very confident that you will handle everything properly all by yourself. Of course I will report and advise immediately whenever I discover new problems.]])),
   posy=1,
}

-- Saledus

saledus = {
   title=_"Military Strength",
   body= saledus2(_"Saledus looks very relaxed…",
      -- TRANSLATORS: Saledus
      _([[Sire, it is really a great pleasure to be at home again. And best of all: I can see we still have some military strength. I can count five towers and a fortress in our vicinity to guard us.]])),
   posy=1,
}

saledus_1 = {
   title=_"We Could Use The Military Instead",
   body= saledus2(_"Saledus raises his voice…",
      -- TRANSLATORS: Saledus
      _([[Sire, if we need the technology that badly, why don’t we conquer it? We could defeat the little monastery in an instant.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[The problem is however that the gods won’t be amused if we destroy one of their temples. And you never know what this could lead to.]]))
      .. new_objectives(obj_deal_with_julia),
   posy=1,
   h=500,
}

saledus_2 = {
   title=_"Easy Victory",
   body= saledus3(_"Saledus is cheering proudly…",
      -- TRANSLATORS: Saledus
      _([[General, our armies just swept over the priestesses and conquered the plans. That was a rather easy victory.]])),
   posy=1,
}

saledus_3 = {
   title=_"Defiance",
   body= saledus2(_"Saledus asserts his point…",
      -- TRANSLATORS: Saledus
      _([[Sire, I admit this has been proven to be a very good deal, although we could have had the plans much earlier. Anyhow, we will not be able to make any deals with the Barbarians, so, we better keep our soldiers in a good mood and train them adequately.]])),
   posy=1,
}

saledus_4 = {
   title=_"Pride",
   body= saledus2(_"Saledus asserts his point…",
      -- TRANSLATORS: Saledus
      _([[Sire, I admit that we most probably courted the gods’ resentment, but in wartime, the end will sometimes justify the means. And we won’t be able to make any deals with the Barbarians either, so, we better keep our soldiers in good mood and train them adequately.]])),
   posy=1,
}


saledus_5 = {
   title=_"Military Strength",
   body= saledus2(_"Saledus asks for a stronger army…",
      -- TRANSLATORS: Saledus
      _([[Now that we have produced some tools, I think it is time to divert some of our iron and coal towards military production.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[I am really worried about the weakness of our army. We should start to increase our military power. As a starting point, we should recruit at least ten new soldiers.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[Perhaps we should train them further in the arena as well.]]))
      .. new_objectives(obj_recruit_soldiers),
   posy=1,
   h=500,
}

saledus_6 = {
   title=_"Training is Needed",
   body= saledus3(_"Saledus still has security concerns…",
      -- TRANSLATORS: Saledus
      _([[General, although we have started recruiting new soldiers, we still need to train them well. It is important to increase the strength of our soldiers as fast as we can.]])
      .. paragraphdivider() ..
      _([[Unfortunately, we only have an arena and a very old and small training camp with very little storage capacity. And none of our builders knows how to improve this. Alas! But we need to use what we have to get prepared for battle.]]))
      .. new_objectives(obj_training),
   posy=1,
   h=500,
}

saledus_7 = {
   title=_"Praise The Army of The Empire",
   body= saledus2(_"Saledus is happy…",
      -- TRANSLATORS: Saledus
      _([[Sire, after training a bunch of recruits, our trainers conferred with our builders and developed better and more efficient training buildings together.]])
      .. paragraphdivider() ..
      _([[Now we can really improve our army and build the military strength that we will need to defend our country.]]))
      .. new_objectives(obj_upgrade),
   posy=1,
   h=500,
}

saledus_8 = {
   title=_"We Just Need Another Hero",
   body= saledus4(_"Saledus is in a good mood…",
      -- TRANSLATORS: Saledus
      _([[General, now that we have more and better beer in addition to enhanced training facilities, we should train as many fully promoted soldiers as we can. I really would consider them heroes after that.]])
      .. paragraphdivider() ..
      _([[It will give us great advantage in battle if our soldiers are much more powerful than the Barbarians’. So, we need to spend our resources wisely to get some heroes as soon as we can.]])
      .. paragraphdivider() ..
      _([[I would say that three fully trained heroes should be sufficient to begin with. But don’t forget to send them to the front line, because they are not that useful back home.]]))
      .. new_objectives(obj_heroes),
   posy=1,
   h=500,
}

saledus_9 = {
   title=_"We Should Expel The Barbarians",
   body= saledus3(_"Saledus is excited…",
      -- TRANSLATORS: Saledus
      _([[Now that we have some fully trained soldiers, it is time to expel the Barbarians from our homeland.]])
      .. paragraphdivider() ..
      _([[Let’s finish them off and regain control over our lands. They shall regret deeply that they ever came!]]))
      .. new_objectives(obj_conquer_all),
   posy=1,
   h=500,
}

saledus_10 = {
   title=_"Victory is Ours",
   body= saledus2(_"Saledus is cheering loudly…",
      -- TRANSLATORS: Saledus
      _([[Sire, finally we have defeated the Barbarians. We have expelled even the last of them. May they never come back!]])
      .. paragraphdivider() ..
      _([[Now it is time to find out why this big mess around us could have happened. But first of all we should have some wine to celebrate our victory and our reclaimed freedom.]])),
   posy=1,
}

saledus_11 = {
   title=_"The Enemy is Near",
   body= saledus3(_"Saledus is alerted…",
      -- TRANSLATORS: Saledus
      _([[Sire, although we don’t have enough fully trained soldiers yet, we just have made contact with the enemy.]])
      .. paragraphdivider() ..
      _([[We have to finish them off and regain control over our lands. They shall regret deeply that they ever came!]]))
      .. new_objectives(obj_conquer_all),
   posy=1,
   h=500,
}



-- Julia - priestess of the goddess Vesta

julia_0 = {
   title=_"Worship to be Gifted",
   body= julia(_"Julia is demanding a sacrifice for Vesta…",
      -- TRANSLATORS: Julia - priestess of the goddess Vesta
      _([[Ave, Lutius! If you want us to help you, you first have to worship our goddess Vesta. Therefore deliver 35 sheaves of wheat and 15 amphoras of wine to prepare a worthy sacrifice for her.]])),
   posy=1,
}

julia_1 = {
   title=_"May The Gods Bless You",
   body= julia(_"Vesta is blessing us…",
      -- TRANSLATORS: Julia - priestess of the goddess Vesta
      _([[Thank you Lutius, you have served our goddess well. You will not only be gifted with the improvements to your technology but with some of the goods you need so desperately as well. Furthermore, we will pray for you and join your party to safeguard our land from the Barbarians.]])),
   posy=1,
}

julia_2 = {
   title=_"Damned",
   body= julia(_"Vesta is cursing us…",
      -- TRANSLATORS: Julia - priestess of the goddess Vesta
      _([[Damn you Lutius for killing peaceful servants of the goddess Vesta! May your life and your land be cursed and may the wrath of the goddess scourge your family from the face of the earth!]])),
   posy=1,
}
