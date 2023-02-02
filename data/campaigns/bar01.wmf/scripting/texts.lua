-- =========================
-- Some formating functions
-- =========================

function thron(title, text)
   return speech("map:chieftain.png", styles.color("campaign_bar_thron"), title, text)
end
function khantrukh(title, text)
   return speech("map:khantrukh.png", styles.color("campaign_bar_khantrukh"), title, text)
end
function boldreth(title, text)
   return speech("map:boldreth.png", styles.color("campaign_bar_boldreth"), title, text)
end

-- =======================================================================
--                                OBJECTIVES
-- =======================================================================
obj_build_rangers = {
   name = "mission rangers",
   title = _("Build two ranger’s huts"),
   number = 1,
   body = objective_text(_("Build two ranger’s huts"),
      li(_("Build a ranger’s hut next to each lumberjack’s hut.")) ..
      li_arrow(_("Naturally, trees only grow at a slow rate. To make sure you have enough logs, you have to build rangers."))
   )
}

obj_claim_northeastern_rocks = {
   name = "mission quarry",
   title = _("Expand north-east and build a quarry"),
   number = 1,
   body = objective_text(_("Expand north-east to the rocks"),
      li(_("Build military buildings (like sentries or barriers) to expand your territory.")) ..
      li(_("Get to the rocks north-east from you and build a quarry there.")))
}

obj_build_mines = {
   name = "mission mines",
   title = _("Start building mines on the mountain"),
   number = 2,
   body = objective_text(_("Build coal and iron mines"),
      li(_("Build a coal mine and an iron mine.")) ..
      li_arrow(_("To do so, place a flag up on the mountain’s flank to the east (on mountain terrain though, not mountain meadow). When you click on the new flag, you can send geologists there. Because the flag is on a mountain, the geologists will search for ores; otherwise, they would search for water. Then build a mine for both kinds of resources that they will find, choosing the appropriate mine to be built:")) ..
      li_object("barbarians_resi_coal_1", _("some coal")) ..
      li_object("barbarians_resi_coal_2", _("a lot of coal")) ..
      li_object("barbarians_resi_iron_1", _("some iron")) ..
      li_object("barbarians_resi_iron_2", _("a lot of iron")) ..
      li_object("barbarians_resi_gold_1", _("some gold")) ..
      li_object("barbarians_resi_gold_2", _("a lot of gold")) ..
      li_object("barbarians_resi_stones_1", _("some granite")) ..
      li_object("barbarians_resi_stones_2", _("a lot of granite")) ..
      li_object("barbarians_resi_water", _("water")) ..
      li_object("barbarians_resi_none", _("nothing was found here")) ..
      p(_([[Mines can only be built on mountain terrain. Suitable places for mines are displayed as orange mine symbols.]])))
}

obj_basic_food = {
   name = "basic food began",
   title = _("Provide your miners with food"),
   number = 3,
   body = objective_text(_("Build a hunter’s hut, a gamekeeper’s hut and a tavern"),
      li(_("In order to work, your miners need food.")) ..
      li_arrow(join_sentences(_([[A hunter can hunt down animals, while a gamekeeper prevents them from becoming extinct. The meat is then processed in a tavern into lunches for your miners.]]), _([[This is only the first example of a ware which has to be refined before being used in a secondary building – others will follow.]]))))
}

obj_begin_farming = {
   name = "farming began",
   title = _("Bake bread"),
   number = 3,
   body = objective_text(_("Build a well, a farm and a bakery"),
      _([[Other ways to produce food – different kinds of food – are fishers and farms. The wheat of the farms has to be processed with water in a bakery before it becomes edible. You can obtain water by building a well. For the best productivity, build it on a spot where your geologists have found a water source.]]))
}

obj_refine_ores = {
   name = "refining began",
   title = _("Refine your mined resources"),
   number = 1,
   body = objective_text(_("Build a smelting works"),
      _([[The iron ore your miners dig up is not usable yet – it has to be melted into iron first.]]))
}

obj_enhance_buildings = {
   name = "enhance buildings",
   title = _("Enhance buildings and build a micro brewery"),
   number = 3,
   body = objective_text(_([[Enhance a mine and the tavern, and build a micro brewery.]]),
      li(_("Enhance the coal mine or the iron mine to a deep mine, and enhance the tavern to an inn.")) ..
      li(_("Also build a micro brewery.")) ..
      li_arrow(_("A normal mine can only dig up about one third of all the resources that lie beneath it; then it must be enhanced to a deep mine in order to keep it working properly. To enhance a building, choose it and then click the appropriate button in the appearing window."))  ..
      li_arrow(_("Workers gain experience by successful work. With enough experience, they become more advanced workers, who are necessary to operate the enhanced buildings. Do not enhance a building before you have enough advanced workers to operate the advanced building!")) ..
      li_arrow(_("Such buildings usually have greater demands than the basic kind of that building – for instance, deep mines need snacks instead of rations. You will have to enhance your tavern to an inn in order to produce snacks out of pitta bread AND a second kind of food (meat or fish) AND beer.")) ..
      _("You may of course enhance all mines to deep mines instantly given you have the workers – bigger mines work a bit faster, smaller mines need cheaper food. It’s up to you which strategy you prefer."))
}

obj_better_material_1 = {
   name = "build hardener",
   title = _("Build a wood hardener"),
   number = 1,
   body = objective_text(_("Build a wood hardener"),
      p(_([[Bigger and better buildings – including all military ones – require better building materials. They cannot be built out of simple logs – the logs have to be refined to blackwood by a wood hardener first. Always remember to build a wood hardener before you run out of blackwood, as without it you cannot expand.]])) ..
      li(_("Build a wood hardener")))
}

obj_better_material_2 = {
   name = "mission grout",
   title = _("Build a lime kiln and coal economy"),
   number = 3,
   body = objective_text(_("Build a lime kiln fed by a well, and by a charcoal kiln or by a coal mine"),
      p(_([[Better buildings may also require other improved materials besides blackwood. One of these is grout, which is produced out of granite, water and coal by a lime-burner.]])) ..
      p(_([[You can obtain water by building a well upon a water source, which your geologists can discover when you send them to any flag that is not on a mountain.]])) ..
      li_image("images/wui/fieldaction/menu_geologist.png", _("In order to call a geologist to search for water, click on a flag in the area that you want him to search and then on the button labeled ‘Send geologist to explore site’.")) ..
      p(_([[Coal can be obtained by building a charcoal kiln or a coal mine. Burning charcoal out of logs is slow. You should only build a charcoal kiln when no coal is available.]])) ..
      li(_("Build a lime kiln and a well. Additionally, build either a charcoal kiln or a coal mine for coal supply.")))
}

obj_better_material_3 = {
   name = "mission reed_yard",
   title = _("Build a reed yard"),
   number = 1,
   body = objective_text(_("Build a reed yard"),
      p(_([[The third material necessary for improved buildings is reed, used to cover roofs. Reed fields are planted by a gardener around his building, the reed yard.]])) ..
      li(_("Build a reed yard")))
}

obj_build_cattlefarm = {
   name = "mission cattlefarm",
   title = _("Build a cattle farm"),
   number = 1,
   body = objective_text(_("Build a cattle farm"),
      p(_([[When roads are under heavy load for a long time, one carrier is usually not enough to transport goods swiftly. Traffic jams are the consequence. Such roads therefore employ a second carrier: an ox that helps to carry the wares. This doubles the transport capacity.]])) ..
      li_arrow(_([[Oxen are bred in cattle farms out of wheat and water.]])) ..
      li(_("Build a cattle farm")))
}


-- =======================================================================
--                     Texts for the Main Mission Thread
-- =======================================================================

briefing_msg_01 = {
   title = _("The Story Begins"),
   body = thron(_("Thron sighs…"),
      -- TRANSLATORS: Thron
      _([[It’s been months, and we are still hiding where the forests are old and dark.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Thron
      _([[My warriors hunt at day and lie awake at night – listening to the sounds of the cruel slaughter echoing from afar amongst the ancient trees.]])
   )
}


briefing_msg_02 = {
   title = _("The Story Begins"),
   body = thron("",
      -- TRANSLATORS: Thron
      _([[We can see the raging flames that swallow Al’thunran from here, miles away.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Thron
      _([[The red lights flash in the darkness and dance to the rhythm of the war drums that haunt me even in my nightmares.]])),
   position = "topleft",
}


briefing_msg_03 = {
   title = _("The Story Begins"),
   body = thron("",
      -- TRANSLATORS: Thron
      _([[My father’s bones rest peacefully in the ground on which he once ended the senseless spilling of blood that had arisen amongst us. It pains me that his peace only endured for one generation.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Thron
      _([[Boldreth, my loyal companion and friend is a source of peace and comfort to me in these dark times. He keeps my spirits high and those of my warriors awake, preventing greed or despair from destroying the bonds between us as well.]])),
   position = "topleft",
}


briefing_msg_04 = {
   title = _("The Story Begins"),
   body = thron("",
      -- TRANSLATORS: Thron
      _([[As father told me, there are times to fight and times to lie and wait, trying not to fall asleep or die before the right time comes. And so I do wait.]])),
   position = "topleft",
}


briefing_msg_05 = {
   title = _("Thron is Contemplating the War"),
   body = thron(_("Thron is shaking his head…"),
      -- TRANSLATORS: Thron
      _([[Yet the war goes on. More and more of our brothers and sisters flee the brutal war raging in the capital beneath the trees.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Thron
      _([[The stories they tell about the deeds of our kin are sad to hear. I’ve spent nights lying awake, restless, more tired than I ever believed one could be. Yet whenever I close my eyes, I see the fortress my father built consumed by flames. The Throne Among the Trees, the symbol of unity and peace among our kin, became the wedge that drives us apart.]])),
   field = al_thunran,
   position = "topleft",
}


briefing_msg_06 = {
   title = _("Thron is Contemplating the War"),
   body = thron("",
      -- TRANSLATORS: Thron
      _([[Today my hunters brought men, women and little children before me who had hidden out in the forests, trying to escape the war, hate and revenge that rage among the tribes fighting each other like in olden times, when we were no more but wild beasts driven and controlled by instincts. None of my brothers will ever gain and hold control over the wooden throne, none of the tribes will be strong enough to subdue the other. There will be no end to this slaughter, unless… is this it? As father told me?]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Thron
      _([[To rise against whoever threatens our very existence, even though it may be one of your own blood or mind?]])),
   field = sf,
}


briefing_msg_07 = {
   title = _("Thron is Considering the Future"),
   body = thron("",
      -- TRANSLATORS: Thron
      _([[Boldreth seems more and more torn as the days go by. The spirits of my fellows sink as the cold season approaches, and we are still living in no more than huts and barracks. I never intended to stay out here in the wilderness for so long – but I never thought my brothers would engage in this senseless battle for so long either.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Thron
      _([[Perhaps it’s time to make ourselves feel a little more at ease here. Perhaps it’s time to give those who still live and think united a new home, replacing what is now lost to us? Until we can return to the place we once called our home…]]))
}


briefing_msg_08 = {
   title = _("Somebody Comes up to You"),
   body = khantrukh(_("An old man says…"),
      -- TRANSLATORS: Khantrukh
      _([[Hail, chieftain. I am Khantrukh and have seen many winters pass. Please allow me to aid you with my counsel through these darkened days.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Khantrukh
      _([[Only the gods know for how long we have to remain hidden here. The warriors hope we may march back gloriously any day now, but I strongly doubt that will happen soon. And the days are short and cold…]])),
   field = sf -- scroll back when showing this and the next few message boxes
}


order_msg_ranger = {
   title = _("The Advisor"),
   body = khantrukh(_("Khantrukh notes…"),
      -- TRANSLATORS: Khantrukh
      _([[I see you have already built a quarry and two lumberjack’s huts. That is a good beginning if we want to stay here longer.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Khantrukh
      _([[But never forget – these forests are our heritage, entrusted upon us by our ancestors. We must always respect and care for them. What we take, we must give back again.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Khantrukh
      _([[So, in order to replace the trees we chop down, we should build some ranger’s huts, preferably close to the lumberjack’s huts.]]))
      .. new_objectives(obj_build_rangers)
}


story_msg1 = {
   title = _("In the Night"),
   body = thron(_("Thron says…"),
      -- TRANSLATORS: Thron
      _([[During another sleepless night, I went up to the hill and gazed towards the north. The fires are still burning, satisfying their hunger upon my father’s legacy.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Thron
      _([[They are a constant reminder of why we have to hide here… and why we must return in the end!]])),
   field = al_thunran,
   position = "topleft",
   scroll_back = true
}


story_msg2= {
   title = _("At the Tomb"),
   body = thron(_("Thron says…"),
      -- TRANSLATORS: Thron
      _([[Once again, I went up to my father’s tomb, in a sacred grove at the foot of the great spire of Kal’mavrath. I just stood there and felt neither the hours pass nor the cold rain pouring down from the darkened sky…]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Thron
      _([[Somehow, it felt like a farewell. For the first time, I wondered what the future might hold for me…]])),
   field = grave,
   position = "topleft",
   scroll_back = true
}


story_msg3 = {
  title = _("The Other Day"),
  body = thron(_("Thron says thoughtfully…"),
      -- TRANSLATORS: Thron
      _([[Some time ago, Boldreth came to me. His advice was to move to a place closer to home – to strike at the first sign of my brothers’ forces wavering.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Thron
      _([[But when I look over the forests I can still see black smoke rising to the sky. I know – it is too early yet, and what he hopes for will not happen any time soon. He might still be right, but I fear the bloodshed that returning too fast would cause on both sides…]]))
}


story_msg4 = {
   title = _("The Oath"),
   body = thron(_("Thron looks furious…"),
      -- TRANSLATORS: Thron
      _([[Today, my warriors picked up an old man, wandering sick and wounded through the dark forest. We listened in horror as he told us of the atrocities taking place in Al’thunran.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Thron
      _([[I hereby renew my oath – I will stop this madness at any cost!]]))
}


order_msg_1 = {
   title = _("Your Loyal Companion"),
   body = boldreth(_("Brave Boldreth steps to your side…"),
      -- TRANSLATORS: Boldreth
      _([[So you want to prepare us for a longer stay in these forests, Thron? Well, I see the wisdom in doing so, yet my heart is yearning for a glorious return to Al’thunran. Too long have we waited while our brethren have fought on the battlefield, and our axes and swords have turned rusty over time.]]))
}


order_msg_2 = {
   title = _("The Advisor"),
   body = khantrukh(_("Khantrukh joins in…"),
      -- TRANSLATORS: Khantrukh
      _([[I beg your forgiveness, chieftain, but I have a grave concern to discuss with you. The rocks close to our camp are shrinking fast under our workers’ tools, and it is a matter of little time only before they are all gone.]]))
}

order_msg_3 = {
   title = _("Your Loyal Companion"),
   position = "left",
   body = boldreth(_("Boldreth jumps up…"),
      -- TRANSLATORS: Boldreth
      _([[While it is certainly no worthy task for a warrior, at least it is something to do! Thron, what would you say about my taking a few of our most restless warriors and venturing somewhat closer to the great mountain of Kal’mavrath – maybe we will find some more big rocks to please the old man?]]))
      .. new_objectives(obj_claim_northeastern_rocks)
}

order_msg_4 = {
   title = _("The Advisor"),
   position = "left",
   body = khantrukh(_("Khantrukh grumbles…"),
      -- TRANSLATORS: Khantrukh
      _([[Old man? Disrespectful youth!]]))
}

order_msg_5_quarry = {
   title = _("The Advisor"),
      -- TRANSLATORS: Khantrukh
   body = khantrukh(_("Khantrukh nods satisfied…"),
      _([[At last! These rocks should last for a while.]]))
}

order_msg_6_geologist = {
   title = _("Your Loyal Companion"),
   body = boldreth(_("Boldreth exclaims…"),
      -- TRANSLATORS: Boldreth
      _([[Just look at that! In the east is the great mountain of Kal’mavrath! I wonder what treasures nature might have hidden beneath its majestic flanks!]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Boldreth
      _([[Let’s expand to the east. There’s a chance for the elderly to become useful once in a while! Let us send out some of those who understand the stone’s tongue to unravel the mountain’s secrets!]]))
      .. new_objectives(obj_build_mines)
}

order_msg_7_mines_up = {
   title = _("Your Loyal Companion"),
   body = boldreth(_("Boldreth laughs…"),
      -- TRANSLATORS: Boldreth
      _([[By Chat’Karuth’s beard, this is amazing! Just imagine what we can use this coal and iron ore for!]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Boldreth
      _([[It might even be enough to…]]))
}

order_msg_8_mines_up = {
   title = _("The Advisor"),
   body = khantrukh(_("Khantrukh interrupts…"),
      -- TRANSLATORS: Khantrukh
      _([[While this is true, we should consider it later, for now other obstacles are at hand. The miners will definitely demand greater rations for working high up on the mountain in this bitter cold, and providing these supplies may not be easy.]]))
}


order_msg_9_hunter = {
   title = _("Your Loyal Companion"),
   body = boldreth(_("Boldreth nods…"),
      -- TRANSLATORS: Boldreth
      _([[I have to hand it to you: you are right here, old man.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Boldreth
      _([[There seem to be quite a lot of animals in the forests here – we might just hunt down more of them. And, of course, the people would cheer a new tavern – hey, we might call it ‘Thron’s Pride’ or so if you want!]]))
      .. new_objectives(obj_basic_food)
}

order_msg_10_bread = {
   title = _("The Advisor"),
   body = khantrukh(_("The elder jumps into the air…"),
      -- TRANSLATORS: Khantrukh
      _([[Wisdom commands to seek variety.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Khantrukh
      _([[North of the great mountain is a large plain – why don’t we use the space Mother Nature gave us and build a farm? I sure would enjoy a freshly baked pitta bread for a change…]]))
      .. new_objectives(obj_begin_farming)
}

order_msg_11_basic_food_began = {
   title = _("Your Loyal Companion"),
   body = boldreth(_("Boldreth cheers up…"),
      -- TRANSLATORS: Boldreth
      _([[Our hunters are out in the forests, Thron.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Boldreth
      _([[I promise you, before the sun sets today you will have a magnificent meal fit for the chieftain of all clans!]]))
}

order_msg_12_farming_began = {
   title = _("The Advisor"),
   body = khantrukh(_("Khantrukh gazes over the plains…"),
      -- TRANSLATORS: Khantrukh
      _([[Isn’t it lovely, the view of golden fields, growing peacefully, dancing to the wind? And I believe the baker is just on his way here, with his most delicious bread reserved for our chieftain alone!]]))
}

order_msg_13_tavern = {
   title = _("A Beautiful Morning"),
   body = thron(_("Thron recognizes…"),
      -- TRANSLATORS: Thron
      _([[A tavern opened for our people yesterday. While I am hardly in the mood for celebration, I noticed how much this tiny bit of home means to my people. Their songs filled the air until deep in the night, and they were in higher spirits still the day after.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Thron
      _([[Maybe we actually are slowly creating a place here which we can… call home.]]))
}

order_msg_14_refine_ore = {
   title = _("Your Loyal Companion"),
   body = boldreth(_("Boldreth remarks…"),
      -- TRANSLATORS: Boldreth
      _([[So now that we have both the mines and the food to sustain them, we should bring what our brave miners gain for us into a more useful state. The iron ore has to be melted into iron so that it is ready to be forged into whatever we want. And we need the coal to keep the smelter’s ovens burning.]]))
      .. new_objectives(obj_refine_ores)
}

order_msg_15_mines_exhausted = {
   title = _("The Advisor"),
   body = khantrukh(_("Khantrukh speaks…"),
      -- TRANSLATORS: Khantrukh
      _([[Our miners are digging up less and less by the day! We have to go deeper, closer to the mountain’s core, if we want more of its treasures!]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Khantrukh
      _([[Of course, it is dark and cold in such depths and only a few venture voluntarily into these places. We should reward this bravery with greater rations for them. And a pint of beer or two will keep their spirits high.]]))
      .. new_objectives(obj_enhance_buildings)
}

order_msg_16_blackwood = {
   title = _("Your Loyal Companion"),
   body = boldreth(_("Boldreth seems concerned…"),
      -- TRANSLATORS: Boldreth
      _([[As I just discovered, we are running short on blackwood! We cannot put our warriors into some crumbling huts, and even less so in times as dangerous as ours!]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Boldreth
      _([[We need a wood hardener, and we need one now!]]))
      .. new_objectives(obj_better_material_1)
}

order_msg_17_grout = {
   title = _("Your Loyal Companion"),
   body = boldreth(_("Boldreth smiles…"),
      -- TRANSLATORS: Boldreth
      _([[Well, old friend, this should ensure that our fortifications do not break down with our foes’ first battle cry! Now we can expand safely!]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Boldreth
      _([[Still, it would not hurt to accumulate some grout for our further campaign; thus we could, in times of need, build a fortress such as the world has never seen before!]]))
      .. new_objectives(obj_better_material_2)
}

order_msg_18_reed = {
   title = _("The Advisor"),
   body = khantrukh(_("Khantrukh steps in…"),
      -- TRANSLATORS: Khantrukh
      _([[Chieftain, this is a disgrace! It is well that we can produce grout for mighty fortifications and great buildings now – only this does not prevent our roofs from becoming leaky! Maybe the young ones like spending their nights in the rain, but I just can’t find any sleep with these raindrops dripping on my face!]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Khantrukh
      _([[Now this is a problem we should do something about!]]))
      .. new_objectives(obj_better_material_3)
}

order_msg_19_all_material = {
   title = _("The Advisor"),
   body = khantrukh(_("Khantrukh seems relieved…"),
      -- TRANSLATORS: Khantrukh
      _([[Wonderful! Our roofs are not dripping anymore, and we have all the material we need for, well, whichever monumental building we see fit!]]))
}

msg_cattlefarm_00 = {
   title = _("We Need Oxen!"),
   body = khantrukh(_("Khantrukh speaks…"),
      -- TRANSLATORS: Khantrukh
      _([[As our realm is getting bigger and bigger, the traffic on the roads is overwhelming. Our poor carriers are no longer able to transport the goods as fast as we need them to. I suggest we give them some support by breeding oxen.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Khantrukh
      _([[They are amazing animals: The ox is as swift as a human being while being much stronger and very frugal: all we need is wheat and water to breed them and they will do their work on the roads loyally and reliably.]]))
      .. new_objectives(obj_build_cattlefarm)
}

msg_mission_complete = {
   allow_next_scenario = true,
   title = _("Mission Complete"),
   body = thron(_("Thron speaks…"),
      -- TRANSLATORS: Thron
      _([[The other day Boldreth asked me to accompany him to the new inn. It would cheer me up, he said.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Thron
      _([[When I looked around, I saw faithful faces, trusting that I could guide them through these dark days. Yet before I could speak any words of gratitude or encouragement, one of my warriors ran into the inn. He had been far out in the forest for the past days and I could see how weary he was.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Thron
      _([[The news he brought changed everything…]]))
      .. objective_text(_("Victory"),
         _([[You have completed this mission. You may continue playing if you wish, otherwise move on to the next mission.]]))
}

msg_story_1 = {
   title = _("One Full Moon Night"),
   body = thron(_("Thron speaks…"),
      -- TRANSLATORS: Thron
      _([[One night, when the moon shone brightly, I climbed to the peak of Kal’mavrath.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Thron
      _([[When I gazed at the horizon, I still saw crimson lights flicker in the distance. It is incredible with how much passion my brethren fight this war. I fear the moment I will see Al’thunran again – will there be anything but ashes and wasted ruins left of our once beautiful capital when we get there?]]))
}

msg_story_2 = {
    title = _("Another Cold Day"),
    body = thron(_("Thron looks worried…"),
      -- TRANSLATORS: Thron
      _([[The winter is upon us. Many of us are suffering from the cold, yet we must endure. The day we may return cannot be far anymore – it must not be far anymore.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Thron
      _([[I prayed that it might get warmer again, as I prayed that the war would finally come to an end.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Thron
      _([[So far, it seems that neither prayer was fulfilled.]]))
}

msg_village = {
   title = _("A friendly village has joined us!"),
   body = thron(_("Thron speaks…"),
      -- TRANSLATORS: Thron
      _([[We have found a village with friendly and productive people, impressed by our wealth, technology and strength.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Thron
      _([[They have lived simply, yet blithely, from hunting and farming. They have not been involved in any conflict so far, and are not ready for fighting.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Thron
      _([[But they fear that the war around Al’thunran will set an end to this life. Therefore, they have decided to join us, hoping that we can help each other.]])
   )
}
