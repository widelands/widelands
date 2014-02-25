use("aux", "formatting")
use("aux", "format_scenario")

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
_[[* Build up a basic food economy – a fisher's hut, a hunter's hut, a well, a farm and a bakery, to provide your people with food.]]),
}

obj_build_cattlefarm = {
   name = "obj_build_cattlefarm",
   title = _ "Build a cattle farm",
   number = 1,
   body = objective_text(_"Remember to build a cattle farm", 
_[[As your roads grow longer and your economy bigger you should make good use
 of your oxen to help transport wares quicker. Remember to build a cattle
 farm!]]),
}

obj_build_a_donjon = {
   name = "build_a_donjon",
   title=_"Build a donjon",
   number = 1,
   body = objective_text(_"Build a donjon",
_"* Build a donjon at the northeast of your territory to get a farther visual range and to protect your people from sudden attacks of enemies."),
}

obj_explore_further = {
   name = "explore_further",
   title=_"Explore further",
   number = 1,
   body = objective_text(_"Explore further",
_"* Build more military buildings to explore the area around the headquarters and to ensure the safety of your people."),
}

obj_build_mining_economy = {
   name = "build_mining_economy",
   title=_"Build a mining infrastructure",
   number = 6,
   body = objective_text(_"Build a mining infrastructure",
_"* Expand your territory to the mountains, send geologists to search for ore and coal and build a mining economy with mines, taverns, smelting works and metalworks."),
}

obj_build_a_fortress = {
   name = "build_a_fortress",
   title=_"Build a fortress",
   number = 1,
   body = objective_text(_"Build a fortress",
_"* Build a fortress to the east of the mountains."),
}

obj_build_training_infrastructure = {
   name = "build_training_infrastructure",
   title=_"Build a training infrastructure",
   number = 4,
   body = objective_text(_"Build a training infrastructure",
_"* Build a battle arena and a trainings camp to the west of the mountains." ..
"<br>" ..
_"* Enhance your metalworks to an axefactory in order to produce weapons and build up another metalworks to ensure the production of tools. When the blacksmith reaches his next level (master blacksmith), you can even enhance it to a war mill which produces additional weapons." ..
"<br>" ..
_"* Build a helmsmithy to the west of the mountains to provide your soldiers with better armor."),
}


obj_build_a_warehouse = {
   name = "build_a_warehouse",
   title=_"Build a warehouse",
   number = 1,
   body = objective_text(_"Build a warehouse",
_"* Build a warehouse to the west of the mountains." ..
"<br>" ..
_"-> Warehouses are similar to your headquarters, with the only difference that it is not defended by soldiers. It is often wise to build a warehouse when your territory is growing and the paths to your headquarters get longer and longer."),
}

obj_build_trainingssites = {
   name = "build_trainingssites",
   title=_"Build training sites",
   number = 2,
   body = objective_text(_"Build training sites",
_"* Build a battle arena and a trainings camp to the west of the mountains." ..
"<br>" ..
_"-> Training sites like a training camp or a battle arena are used for training soldiers in their different attributes. Those attributes are important in fights and the better a soldier is, the higher is the chance to win a fight.<br>Attributes are: health points, attack, defense and evade."),
}

obj_build_weapon_productions = {
   name = "build_weapon_productions",
   title=_"Build weapons production",
   number = 3,
   body = objective_text(_"Build weapons production",
_"* Enhance your metalworks to an axefactory to start producing weapons, and build up another metalworks to ensure the production of tools. When the blacksmith reaches his next level (master blacksmith), you can even enhance your axefactory to a war mill."),
}

obj_build_a_helmsmithy = {
   name = "build_a_helmsmithy",
   title=_"Build a helmsmithy",
   number = 1,
   body = objective_text(_"Build a helmsmithy",
_"* Build a helmsmithy to the west of the mountains to provide your soldiers with better armor."),
}

obj_destroy_kalitaths_army = {
   name = "destroy_kalitaths_army",
   title=_"Destroy Kalitath's army",
   number = 2,
   body = objective_text(_"Destroy Kalitath's army",
_"* Destroy Kalitath's army and expand your territory to the east." ..
"<br>" ..
_"-> To attack an enemy you must click on the door of an adversary's military building. A menu will pop up allowing you to select the number of soldiers that should attack. When you are ready with setting the number, click on the cross ('start attack')"),
}

obj_military_assault_on_althunran = {
   name = "military_assault_on_althunran",
   title=_"Military assault on Al’thunran",
   number = 1,
   body = objective_text(_"Military assault on Al’thunran",
_"* Destroy all enforcements of Thron’s two brothers to liberate the throne-circle."),
}

-- =======================================================================
--                              Messages to the user
-- =======================================================================
story_msg_1 = {
   posy=1,
   title=_"Close to home",
   body =thron(_"Thron looks worried...",
_[[These last days we came closer to our capital. Many people have already joined us on our march and set their hopes on me. However, I fear that we are not strong enough to take up the battle with my brothers.<br>For now we rest at the borders of the old forest and prepare for the coming days.]]),
}

story_msg_2 = {
   posy=1,
   title=_"Another cold day",
   body =thron(_"Thron looks worried.",
_[[The news that a patrol brought is not good to hear. Our spies tell us, that the wartribes still rage in battle but some of the elder soldiers and tribe leaders seem to grow tired of this senseless war. Kalitath, son of Kun is the first warleader to leave Al’thunran, thus leaving his side weak and vulnerable. As my scouts report, he has set foot on the lower grounds around the capitol, securing the outer fortification rings.]]),
}

story_msg_3 = {
   title=_"Doubts",
   posy=1,
   body = boldreth(_"Boldreth shakes his head.",
_[[Thron seeks to believe that Kalitath is tired of fighting and will be willing to join us, once we march to re-establish law and order in Al’thunran. But I don’t think so. Kalitath is son to one of the mightiest generals of our forces and his family never quite liked the thought to bow before the banner of Chat’Karuth. Why should he march with his son to reclaim his heritage and the throne?]]),
}

story_msg_4 = {
   posy=1,
   title=_"The story continues",
   body= boldreth(
_[[However, Thron is not willing to listen to my doubts. He seeks to expand our fortifications to the borders of Kalitath’s territory and reunite with his forces. I have to keep an eye on everything that happens here.]]),
}

story_msg_5 = {
   posy=1,
   title=_"Another cold day",
   body= thron(_"Thron speaks confidently.",
_[[The frontier line of my forces has expanded to the edge of the forests. We have a long and cold winter and so many miles behind us. The fortification that we erected around Ondun will guarantee us a fast supply of men and goods and once Al’thunran is in my hands, the two cities will be linked by a good and defendable infrastructure. Still I hope I will not walk alone on this mission, I hope Kalitath son of Kun will follow my call.]]),
}

story_msg_6 = {
   posy=1,
   title=_"The story continues",
   body= thron(
_[[I have sent men to meet with Kalitath in his fortress. He has barricaded the former main entrance to the capital and is trying to set up a siege ring around Al’thunran. He will not let anyone enter or leave the city while the ring is intact. Until now, he has not answered any of my messages. But the time I wait won't be lost. I will further upgrade my soldiers and prepare for battle.]]),
}

order_msg_7_destroy_kalitaths_army = {
   title=_"Your loyal companion",
   body= boldreth(_"Boldreth shouts out!",
_[[This BASTARD! Only one man of our frontier patrol came back alive. He carried a message from Kalitath telling us that he will punish every move of our troops with their own blood! We cannot accept this behaviour. From now on there is war between Kalitath and us!]])
.. new_objectives(obj_destroy_kalitaths_army),
}



order_msg_7_renegade_fortification = {
   posy = 1,
   title=_"Renegade fortification",
   body= thron(_"Thron speaks:",
_[[The traitor has left his armies dying where they lay as he saw that he would not keep my forces back. Kalitath disappeared in the mess of the war, but I don’t care now. Shall he flee and be forgotten for all times as a tribeleader that would not bow before the wooden throne, that is mine to take now. Even more his flight brought us a big benefit: All year Kalitath was so busy fighting that he did not even care to store enough food for all his men. Now that he has fled, their hunger has brought some of his younger followers to desert. This group holds a strong fortification on the main ring. But there is more news: My scouts reported, that the two raging tribes of my brothers are in chaos and have barely noticed the new danger that is about to strike them. However they would not care if they noticed I guess, blinded by rage and hate, there is no sense left in them at all I believe.]]),
}

briefing_msg_1 = {
   posy=1,
   title=_"Winter ahead",
   body= boldreth(_"Boldreth speaks:",
_[[This morning I found Thron trouble-minded. He returned from his patrol late last night and sat back against a tree watching over the encampment. The number of quarrels and disputes keeps growing and the winter has us firmly in its grip. We should put food in our storage to survive the long, cold winter!]]),
}

order_msg_1_small_food_economy = {
   posy=1,
   title=_"The advisor",
   body= khantrukh(_"Khantrukh nods",
_[[Boldreth is right! We don't have enough food to survive a long, cold winter. We should start to build a well, a fisher's and a hunter's hut. And if we find enough space, we should as well build up a farm and a bakery! One of our farmers seems to be very clever he already got some crops to grow in this cold soil.]])
.. new_objectives(obj_build_small_food_economy),
}

story_note_1 = {
   posy=1,
   title=_"Food economy completed",
   body= thron(
_[[Khantrukh just informed me, that our basic food economy was built up successfully. Finally none of us has to fear starvation anymore.]]),
}

order_msg_2_build_a_donjon = {
   posy=1,
   title=_"Tracks",
   body= boldreth(
   _[[Thron told me that he discovered tracks in the snow to the northeast of our settlement. He instructed us to build a Donjon, to have a greater visual range on the area around our hall to protect our people from sudden attacks.]])
.. new_objectives(obj_build_a_donjon),
}

order_msg_3_explore_further = {
   posy=1,
   title=_"Further explorations",
   body= boldreth(
_[[The first fortification Thron ordered has just been completed. At the moment he seeks to bring trust and belief to those who live inside the walls of our new habitat, that we named 'Ondun', which means 'those who wait' in the old tongue. But Thron is not yet satisfied – and I fully understand his fears: The forests are deep and the frontier to Al’thunran is near. I am sure that there are a thousand greater dangers out there than the unknown tracks.<br>To overlook more of the area around our hall we should explore further and set up more guards and scouts to observe the frontier and keep an eye on the raging wars on our doorsteps. THEN we will be prepared once trouble seeks to capture our woods.]])
.. new_objectives(obj_explore_further),
}

order_msg_4_build_mining_economy = {
   posy=1,
   title=_"Mountains found",
   body= khantrukh(
_[[I just talked to the leader of our patrol. He told me, that they found some mountains to the north east and the east of our territory. With a bit of luck this opens us a new chance for rearming our men, because as far as I remember, most of the mountains west to Al'thunran hold coal and some metal ores.]])
.. new_objectives(obj_build_mining_economy),
}

story_note_2 = {
   posy=1,
   title=_"Mining economy completed",
   body= thron(
_[[I just got informed, that our mining economy started its work. This news brought a smile to my face – even if the hardest part is still waiting to be done, now our smiths will give their best to further ease our lives.]]),
}

order_msg_5_build_a_fortress = {
   posy=1,
   title=_"Fortress needed",
   body= thron(
_[[Boldreth and I just explored the area around the three mountains. One major strategic problem of that area seems to be, that there are two bigger passages – one north and one south to the middle mountain. Those passages are quite hard to control and so it seems to me, like we have only one chance to secure the mountains from enemy hands: We must build a fortress to the east of the mountains!]])
.. new_objectives(obj_build_a_fortress),
}

story_note_3 = {
   posy=1,
   title=_"Fortress completed",
   body=thron(
_[[Finally!<br>The fortress was just completed, so the entrance to our settlement should be safe for now. For the future we might think on building some more fortifications like barriers and donjons at the passages, but for now we can take a deep breath.]]),
}

order_msg_6_build_enhanced_economy_and_training = {
   posy=1,
   title=_"Enhanced economy and training",
   body= boldreth(
_[[Our mining economy seems to work fine, that is already good news! Anyway there are still a lot of men and women waiting in our hall to get a task, so they can help our soldiers to prepare for the future fights.]] ..
"<br>" ..
_[[Our geologists found a lot more deposits of iron ore, coal and even gold. We should enhance our current mines as soon as our miners are more experienced. We should build up some more mines, a bigger food infrastructure to always supply our men with sufficient food and a further processing economy. But that's still not everything – most of our soldiers are quite young and neither have a good weapon nor are they well trained. We should build up trainingssites to prepare them for their future tasks.]])
.. new_objectives(obj_build_training_infrastructure),
}

story_msg_7 = {
   posy=1,
   title=_"Another cold day",
   body= thron(_"Thron looks worried.",
_[[My brothers and their soldiers are dead and left fire and destruction behind. In none of all the ruins could I find any man or woman of the normal folk and so I just may hope that they fled from this cruel battleground and started a better life somewhere else. Nothing is left from that beauty I remember, only ruins remain from the old capital. I am sorrowful but also relieved. I never hoped to ever see Al'thunran's old beauty again – it was only a few months ago that I wondered whether I would ever be able to set my feet in Al'thunran again. Now the old town is back in my hands and I will not hesitate to rebuild it with all the strength my people have. I can't wait to see it again in it's old beauty.]])
  .. rt("<p font-size=10> <br></p>" ..
      h1(_"Victory") .. p(_
[[You have completed the last mission of the barbarian tutorial campaign. You may continue playing if you wish, otherwise move on to the next campaign.]]
      )
   )
}

order_msg_7_free_althunran = {
   posy = 1,
   title=_"Military assault",
   body= thron(
_[[So be it, the generals are out to set up my troops and Boldreth himself will lead the first strike. I will ride by his side, to free Al’thunran of this war and bring peace back to our capital. When the young sun rises above the trees tomorrow morning, I will order the assault. By dusk, I will celebrate my victory in the wooden halls of the warlord and sacrifice a newborn lamb in the honour of my father whose eyes are set upon me today.<br>So be it!]])
.. new_objectives(obj_military_assault_on_althunran),
}

cattlefarm_01 = {
   title = _ "Khantrukh speaks",
   body= khantrukh(
_[[Chieftain, remember the poor carriers: Our roads get longer and longer and
 the burden on their shoulders never gets lighter. We should breed more oxen to
 make their work lighter and our transportation more efficient.]])
.. new_objectives(obj_build_cattlefarm),
}

