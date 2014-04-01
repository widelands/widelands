include "scripting/formatting.lua"
include "scripting/format_scenario.lua"

function thron(title, text)
   return speech("map:chieftain.png", "2F9131", title, text)
end
function khantrukh(title, text)
   return speech("map:khantrukh.png", "8080FF", title, text)
end
function boldreth(title, text)
   return speech("map:boldreth.png", "FF0000", title, text)
end

-- =======================================================================
--                            Objective messages
-- =======================================================================


obj_build_small_food_economy = {
   name = "build_small_food_economy",
   title=_"Build up a small food economy",
   number = 5,
   body = objective_text(_"Build up a small food economy",
[[• ]] .. _([[Build up a basic food economy to provide your people with food]] ..
[[ – a fisher’s hut, a hunter’s hut, a well, a farm and a bakery.]])),
}

obj_build_cattlefarm = {
   name = "obj_build_cattlefarm",
   title = _ "Build a cattle farm",
   number = 1,
   body = objective_text(_"Remember to build a cattle farm",
_([[As your roads grow longer and your economy bigger, you should make good use of your oxen ]] ..
[[to help transport wares more quickly. Remember to build a cattle farm!]])),
}

obj_build_a_donjon = {
   name = "build_a_donjon",
   title=_"Build a donjon",
   number = 1,
   body = objective_text(_"Build a donjon",
[[• ]] .. _([[Build a donjon at the north-east of your territory to get greater visual range ]] ..
[[and to protect your people from sudden attacks of enemies.]])),
}

obj_explore_further = {
   name = "explore_further",
   title=_"Explore further",
   number = 1,
   body = objective_text(_"Explore further",
[[• ]] .. _([[Build more military buildings to explore the area around the headquarters ]] ..
[[and to ensure the safety of your people.]])),
}

obj_build_mining_economy = {
   name = "build_mining_economy",
   title=_"Build a mining infrastructure",
   number = 6,
   body = objective_text(_"Build a mining infrastructure",
[[• ]] .. _([[Expand your territory to the mountains, send geologists to search for ore and coal ]] ..
[[and build a mining economy with mines, taverns, smelting works and metalworks.]])),
}

obj_build_a_fortress = {
   name = "build_a_fortress",
   title=_"Build a fortress",
   number = 1,
   body = objective_text(_"Build a fortress",
[[• ]] .. _"Build a fortress to the east of the mountains."),
}

obj_build_training_infrastructure = {
   name = "build_training_infrastructure",
   title=_"Build a training infrastructure",
   number = 4,
   body = objective_text(_"Build a training infrastructure",
[[• ]] .. _"Build a battle arena and a training camp to the west of the mountains." ..
"<br>" ..
[[• ]] .. _([[Enhance your metal workshop to an axfactory in order to produce weapons, ]] ..
[[and build up a second metal workshop to ensure the production of tools. ]] ..
[[When the blacksmith reaches his next level (master blacksmith), ]] ..
[[you can even enhance the axfactory to a war mill, which will produce additional weapons.]]) ..
"<br>" ..
[[• ]] .. _"Build a helmsmithy to the west of the mountains to provide your soldiers with better armor."),
}


obj_build_a_warehouse = {
   name = "build_a_warehouse",
   title=_"Build a warehouse",
   number = 1,
   body = objective_text(_"Build a warehouse",
[[• ]] .. _"Build a warehouse to the west of the mountains." ..
"<br><br>" ..
[[→ ]] .. _([[Warehouses are similar to your headquarters, ]] ..
[[with the only difference that they aren’t defended by soldiers. ]] ..
[[It is often wise to build a warehouse when your territory is growing ]] ..
[[and the paths to your headquarters get longer and longer.]])),
}

obj_build_trainingssites = {
   name = "build_trainingssites",
   title=_"Build training sites",
   number = 2,
   body = objective_text(_"Build training sites",
[[• ]] .. _"Build a battle arena and a training camp to the west of the mountains." ..
"<br>" ..
[[→ ]] .. _([[Training sites like a training camp or a battle arena are used ]] ..
[[for training soldiers in their different attributes. ]] ..
[[These attributes are important in fights and the better a soldier is, the higher is the chance to win a fight.]]) .. "<br>" ..
_"The attributes are: health points, attack, defense and evade."),
}

obj_build_weapon_productions = {
   name = "build_weapon_productions",
   title=_"Build weapons production",
   number = 3,
   body = objective_text(_"Build weapons production",
[[• ]] .. _([[Enhance your metal workshop to an axfactory in order to produce weapons, ]] ..
[[and build up a second metal workshop to ensure the production of tools. ]] ..
[[When the blacksmith reaches his next level (master blacksmith), ]] ..
[[you can even enhance the axfactory to a war mill, which will produce additional weapons.]])),
}

obj_build_a_helmsmithy = {
   name = "build_a_helmsmithy",
   title=_"Build a helmsmithy",
   number = 1,
   body = objective_text(_"Build a helmsmithy",
[[• ]] .. _"Build a helmsmithy to the west of the mountains to provide your soldiers with better armor."),
}

obj_destroy_kalitaths_army = {
   name = "destroy_kalitaths_army",
   title=_"Destroy Kalitath’s army",
   number = 2,
   body = objective_text(_"Destroy Kalitath’s army",
[[• ]] .. _"Destroy Kalitath’s army and expand your territory to the east." ..
"<br><br>" ..
[[→ ]] .. _([[To attack an enemy, you must click on the door of an adversary’s military building. ]] ..
[[A menu will pop up allowing you to select the number of soldiers that should attack. ]] ..
[[When you are ready with setting the number, click on the cross (‘Start Attack’).]])),
}

obj_military_assault_on_althunran = {
   name = "military_assault_on_althunran",
   title=_"Military assault on Al’thunran",
   number = 1,
   body = objective_text(_"Military assault on Al’thunran",
[[• ]] .. _"Destroy all forces of Thron’s two brothers to liberate the throne-circle."),
}

-- =======================================================================
--                              Messages to the user
-- =======================================================================
story_msg_1 = {
   posy=1,
   title=_"Close to Home",
   body =thron(_"Thron looks worried...",
_([[These last days, we came closer to our capital. ]] ..
[[Many people have already joined us on our march and set their hopes on me. ]] ..
[[However, I fear that we are not strong enough to take up the battle against my brothers.]]) ..
"<br><br>" ..
_([[For now, we are resting at the borders of the old forest and preparing for the coming days.]])),
}

story_msg_2 = {
   posy=1,
   title=_"Another Cold Day",
   body =thron(_"Thron looks worried...",
_([[The news that a patrol brought is not good to hear. ]] ..
[[Our spies tell us that the war tribes still rage in battle, ]] ..
[[but some of the elder soldiers and tribe leaders seem to grow tired of this senseless war. ]] ..
[[Kalitath, son of Kun is the first war leader to leave Al’thunran, thus leaving his side weak and vulnerable. ]] ..
[[As my scouts report, he has set foot on the lower grounds around the capital, ]] ..
[[securing the outer fortification rings.]])),
}

story_msg_3 = {
   title=_"Doubts",
   posy=1,
   body = boldreth(_"Boldreth is shaking his head...",
_([[Thron seeks to believe that Kalitath is tired of fighting and would be willing to join us ]] ..
[[once we march to re-establish law and order in Al’thunran. But I don’t think so. ]] ..
[[Kalitath is son to one of the mightiest generals of our forces ]] ..
[[and his family never quite liked the thought of bowing before the banner of Chat’Karuth. ]] ..
[[Why should he march with Chat’Karuth’s son to reclaim his heritage and the throne?]])),
}

story_msg_4 = {
   posy=1,
   title=_"The Story Continues",
   body= boldreth(
_([[However, Thron is not willing to listen to my doubts. ]] ..
[[He seeks to expand our fortifications to the borders of Kalitath’s territory and to reunite with his forces. ]] ..
[[I have to keep an eye on everything that happens here.]])),
}

story_msg_5 = {
   posy=1,
   title=_"Another Cold Day",
   body= thron(_"Thron is speaking confidently.",
_([[The frontier line of my forces has expanded to the edge of the forests. ]] ..
[[We have a long and cold winter and so many miles behind us. ]] ..
[[The fortification that we erected around Ondun will guarantee us a fast supply of men and goods, ]] ..
[[and once Al’thunran is in my hands, the two cities will be linked by a good and defensible infrastructure. ]] ..
[[Still, I hope I will not walk alone on this mission, I hope Kalitath son of Kun will follow my call.]])),
}

story_msg_6 = {
   posy=1,
   title=_"The Story Continues",
   body= thron(
_([[I have sent men to meet with Kalitath in his fortress. ]] ..
[[He has barricaded the former main entrance to the capital and is trying to set up a siege ring around Al’thunran. ]] ..
[[He will not let anyone enter or leave the city while the ring is intact. ]] ..
[[Until now, he has not answered any of my messages. But the time I spend waiting won’t be lost. ]] ..
[[I will further upgrade my soldiers and prepare for battle.]])),
}

order_msg_7_destroy_kalitaths_army = {
   title=_"Your Loyal Companion",
   body= boldreth(_"Boldreth shouts out!",
_([[The BASTARD! Only one man of our frontier patrol came back alive. ]] ..
[[He carried a message from Kalitath, telling us that he will punish every move of our troops with their own blood! ]] ..
[[We cannot accept this behavior. From now on, there is war between Kalitath and us!]]))
.. new_objectives(obj_destroy_kalitaths_army),
}



order_msg_7_renegade_fortification = {
   posy = 1,
   title=_"Renegade Fortification",
   body= thron(_"Thron says:",
_([[The traitor left his armies dying where they lay when he saw that he would not keep my forces back. ]] ..
[[Kalitath disappeared in the confusion of the war, but I don’t care now. ]] ..
[[Shall he flee and be forgotten for all times as a tribal leader that would not bow before the wooden throne, ]] ..
[[that is mine to take now.]]) .. "<br><br>" ..
_([[Furthermore, his flight brought us a great benefit: ]] ..
[[all year, Kalitath was so busy fighting that he did not even care to store enough food for all his men. ]] ..
[[Now that he has fled, their hunger has caused some of his younger followers to desert. ]] ..
[[This group holds a strong fortification on the main ring.]]) ..
_([[But there is more news: ]] ..
[[My scouts have reported that the two raging tribes of my brothers are in chaos ]] ..
[[and have barely noticed the new danger that is about to strike them. ]] ..
[[However, they would not care if they noticed I guess, blinded by rage and hate, ]] ..
[[there is no sense left in them at all I believe.]])),
}

briefing_msg_1 = {
   posy=1,
   title=_"Winter Ahead",
   body= boldreth(_"Boldreth says:",
_([[This morning, I found Thron’s mind troubled. He returned from his patrol late last night ]] ..
[[and sat back against a tree, watching over the encampment. ]] ..
[[The number of quarrels and disputes keeps growing, and the winter has us firmly in its grip. ]] ..
[[We should put food in our storage to survive the long, cold winter!]])),
}

order_msg_1_small_food_economy = {
   posy=1,
   title=_"The Advisor",
   body= khantrukh(_"Khantrukh nods.",
_([[Boldreth is right! We don’t have enough food to survive a long, cold winter. ]] ..
[[We should start to build a well, a fisher’s and a hunter’s hut. ]] ..
[[And if we find enough space, we should as well build up a farm and a bakery! ]] ..
[[One of our farmers seems to be very clever – he already got some crops to grow in this cold soil.]]))
.. new_objectives(obj_build_small_food_economy),
}

story_note_1 = {
   posy=1,
   title=_"Food Economy Completed",
   body= thron(
_([[Khantrukh has just informed me that our basic food economy was built up successfully. ]] ..
[[Finally, none of us has to fear starvation anymore.]])),
}

order_msg_2_build_a_donjon = {
   posy=1,
   title=_"Tracks",
   body= boldreth(
   _([[Thron told me that he discovered tracks in the snow to the northeast of our settlement. ]] ..
[[He has instructed us to build a Donjon, to have a greater visual range over the area around our hall ]] ..
[[to protect our people from sudden attacks.]]))
.. new_objectives(obj_build_a_donjon),
}

order_msg_3_explore_further = {
   posy=1,
   title=_"Further Exploration",
   body= boldreth(
_([[The first fortification Thron ordered has just been completed. ]] ..
[[At the moment, he seeks to bring trust and belief to those who live inside the walls of our new habitat, ]] ..
[[that we named ‘Ondun’, which means ‘those who wait’ in the old tongue. ]] ..
[[But Thron is not yet satisfied – and I fully understand his fears: ]] ..
[[the forests are deep, and the frontier to Al’thunran is near. ]] ..
[[I am sure that there are a thousand greater dangers out there than the unknown tracks.]]) ..
"<br><br>" ..
_([[To overlook more of the area around our hall, we should explore further ]] ..
[[and set up more guards and scouts to observe the frontier ]] ..
[[and keep an eye on the raging wars on our doorstep. ]] ..
[[THEN we will be prepared once trouble seeks to capture our woods.]]))
.. new_objectives(obj_explore_further),
}

order_msg_4_build_mining_economy = {
   posy=1,
   title=_"Mountains Found",
   body= khantrukh(
_([[I have just spoken to the leader of our patrol. ]] ..
[[He told me that they found some mountains to the north-east and east of our territory. ]] ..
[[With a bit of luck this will open up a new chance for rearming our men, ]] ..
[[because as far as I remember, most of the mountains west to Al’thunran hold coal and some metal ores.]]))
.. new_objectives(obj_build_mining_economy),
}

story_note_2 = {
   posy=1,
   title=_"Mining Economy Completed",
   body= thron(
_([[I just got informed that our mining economy has started its work. ]] ..
[[This news brought a smile to my face – even if the hardest part is still waiting to be done, ]] ..
[[now that our smiths will give their best to further ease our lives.]])),
}

order_msg_5_build_a_fortress = {
   posy=1,
   title=_"Fortress Needed",
   body= thron(
_([[Boldreth and I just explored the area around the three mountains. ]] ..
[[One major strategic problem of that area seems to be that there are two bigger passages]] ..
[[ – one north and one south of the central mountain. Those passages are quite hard to control, ]] ..
[[and so it seems to me like we have only one chance to secure the mountains from enemy hands: ]] ..
[[We must build a fortress to the east of the mountains!]]))
.. new_objectives(obj_build_a_fortress),
}

story_note_3 = {
   posy=1,
   title=_"Fortress Completed",
   body=thron(
_([[Finally! The fortress has just been completed, so the entrance to our settlement should be safe for now. ]] ..
[[For the future, we might think on building some more fortifications like barriers and donjons at the passages, ]] ..
[[but for now we can take a deep breath.]])),
}

order_msg_6_build_enhanced_economy_and_training = {
   posy=1,
   title=_"Enhanced Economy and Training",
   body= boldreth(
_([[Our mining economy seems to work fine, that is already good news! ]] ..
[[Anyway, there are still a lot of men and women waiting in our hall to get a task, ]] ..
[[so they can help our soldiers to prepare for the future fights.]]) ..
"<br><br>" ..
_([[Our geologists found a lot more deposits of iron ore, coal and even gold. ]] ..
[[We should enhance our current mines as soon as our miners are more experienced. ]] ..
[[We should build up some more mines, a bigger food infrastructure ]] ..
[[to always supply our men with sufficient food and a further processing economy. ]] ..
[[But that’s still not everything – most of our soldiers are quite young ]] ..
[[and neither have a good weapon nor are they well trained. ]] ..
[[We should build up training sites to prepare them for their future tasks.]]))
.. new_objectives(obj_build_training_infrastructure),
}

story_msg_7 = {
   posy=1,
   title=_"Another Cold Day",
   body= thron(_"Thron looks worried...",
_([[My brothers and their soldiers are dead and left fire and destruction behind. ]] ..
[[In none of all the ruins could I find any man or woman of the normal folk, ]] ..
[[and so I just may hope that they fled from this cruel battleground and started a better life somewhere else.]]) .. "<br><br>" ..
_([[Nothing is left of that beauty I remember, only ruins remain of the old capital. ]] ..
[[I am sorrowful but also relieved. I never hoped to ever see Al’thunran’s old beauty again]] ..
[[ – it was only a few months ago that I wondered whether I would ever be able to set my feet in Al’thunran again. ]] ..
[[Now the old town is back in my hands, and I will not hesitate to rebuild it with all the strength my people have. ]] ..
[[I can’t wait to see it again in its old beauty.]]))
..  objective_text(_"Victory",
   _([[You have completed the last mission of the barbarian tutorial campaign. ]] ..
[[You may continue playing if you wish, otherwise move on to the next campaign.]]))
}

order_msg_7_free_althunran = {
   posy = 1,
   title=_"Military Assault",
   body= thron(
_([[So be it, the generals are out and about arranging my troops, and Boldreth himself will lead the first strike. ]] ..
[[I will ride by his side, to free Al’thunran of this war and return peace to our capital.]]) .. "<br><br>" ..
_([[When the young sun rises above the trees tomorrow morning, I will order the assault. ]] ..
[[By dusk, I will celebrate my victory in the wooden halls of the warlord ]] ..
[[and sacrifice a newborn lamb in the honor of my father, whose eyes are set upon me today.]]) ..
"<br><br>" ..
_([[So be it!]]))
.. new_objectives(obj_military_assault_on_althunran),
}

cattlefarm_01 = {
   title = _"Khantrukh says:",
   body= khantrukh(
_([[Chieftain, remember the poor carriers: Our roads get longer and longer ]] ..
[[and the burden on their shoulders never gets lighter. ]] ..
[[We should breed more oxen to make their work lighter and our transportation more efficient.]]))
.. new_objectives(obj_build_cattlefarm),
}
