-- =========================
-- Some formating functions
-- =========================
-- Rich Text
function rt(text_or_opts, text)
   k = "<rt>"
   if text then
      k = ("<rt %s>"):format(text_or_opts)
   else
      text = text_or_opts
   end

   return k .. text .. "</rt>"
end

-- Headings
function h1(s)
   return "<p font=FreeSerif font-size=18 font-weight=bold font-color=D1D1D1>"
      ..  s .. "<br></p><p font-size=8> <br></p>"
end
   
function direct_speech(heading, text)
   return "<p font-size=24 font-face=FreeSerif font-weight=bold " ..
      "font-color=2F9131>" .. heading .. "</p><p font-size=8> <br></p>" .. text
end

-- Simple flowing text. One Paragraph
function p(s)
   return "<p line-spacing=3 font-size=12>" .. s .. "<br></p>" ..
      "<p font-size=8> <br></p>"
end

-- No logic below

-- =======================================================================
--                            Objective messages
-- =======================================================================
obj_build_small_food_economy = {
   name = "build_small_food_economy",
   title=_"Build up a small food economy",
   body = "<rt><p line-spacing=3 font-size=12>" ..
_[[* Build up a basic food economy - a fisher's hut, a hunter's hut, a well, a farm and a bakery, to provide your people with food.]] ..
"</p></rt>",
}

obj_build_cattlefarm = {
   name = "obj_build_cattlefarm",
   title = _ "Remember to build a cattle farm",
   body = rt(p(_
[[As your roads grow longer and your economy bigger you should make good use
 of your oxen to help transport wares quicker. Remember to build a cattle
 farm!]]))
}

obj_build_a_donjon = {
   name = "build_a_donjon",
   title=_"Build a donjon",
   body= "<rt><p line-spacing=3 font-size=12>" ..
_"* Build a donjon at the northeast of your territory to get a farther visual range and to protect your people from sudden attacks of enemies." ..
"</p></rt>",
}

obj_explore_further = {
   name = "explore_further",
   title=_"Explore further",
   body= "<rt><p line-spacing=3 font-size=12>" ..
_"* Build more military buildings to explore the area around the headquarters and to ensure the safety of your people." ..
"</p></rt>",
}

obj_build_mining_economy = {
   name = "build_mining_economy",
   title=_"Build mines",
   body= "<rt><p line-spacing=3 font-size=12>" ..
_"* Expand your territory to the mountains, send geologists to search for ore and coal and build a mining economy with mines, taverns, smelting works and metalworks." ..
"</p></rt>",
}

obj_build_a_fortress = {
   name = "build_a_fortress",
   title=_"Build a fortress",
   body= "<rt><p line-spacing=3 font-size=12>" ..
_"* Build a fortress to the east of the mountains." ..
"</p></rt>",
}

obj_build_a_warehouse = {
   name = "build_a_warehouse",
   title=_"Build a warehouse",
   body= "<rt><p line-spacing=3 font-size=12>" ..
_"* Build a warehouse to the west of the mountains." ..
"<br>" ..
_"-> Warehouses are similiar to your headquarters, with the only difference that it is not defended by soldiers. It is often wise to build a warehouse when your territory is growing and the paths to your headquarters get longer and longer." ..
"</p></rt>",
}

obj_build_trainingssites = {
   name = "build_trainingssites",
   title=_"Build trainingssites",
   body= "<rt><p line-spacing=3 font-size=12>" ..
_"* Build a battle arena and a trainings camp to the west of the mountains." ..
"<br>" ..
_"-> Trainingssites like a training camp or a battle arena are used for training soldiers in their different attributes. Those attributes are important in fights and the better a soldier is, the higher is the chance to win a fight.<br>Attributes are: health points, attack, defense and evade." ..
"</p></rt>",
}

obj_build_weapon_productions = {
   name = "build_weapon_productions",
   title=_"Build weapon productions",
   body= "<rt><p line-spacing=3 font-size=12>" ..
_"* As soon as the blacksmith reaches his next level (master blacksmith) enhance your metalworks to an axefactory or even a war mill and build up another metalworks to ensure the production of tools." ..
"</p></rt>",
}

obj_build_a_helmsmithy = {
   name = "build_a_helmsmithy",
   title=_"Build a helmsmithy",
   body= "<rt><p line-spacing=3 font-size=12>" ..
_"* Build a helmsmithy to the west of the mountains to provide your soldiers with better armor." ..
"</p></rt>",
}

obj_destroy_kalitaths_army = {
   name = "destroy_kalitaths_army",
   title=_"Destroy Kalitaths army",
   body= "<rt><p line-spacing=3 font-size=12>" ..
_"* Destroy Kalitaths army and expand your territory to the east." ..
"<br>" ..
_"-> To attack an enemy you must click on the door of an advarse military building. A menu will pop up allowing you to select the number of soldiers that should attack. When you are ready with setting the number, click on the cross ('start attack')" ..
"</p></rt>",
}

obj_military_assault_on_althunran = {
   name = "military_assault_on_althunran",
   title=_"Military assault on Al’thunran",
   body= "<rt><p line-spacing=3 font-size=12>" ..
_"* Destroy all enforcements of Thron’s two brothers to liberate the throne-circle." ..
"</p></rt>",
}

-- =======================================================================
--                              Messages to the user
-- =======================================================================
story_msg_1 = {
   posy=1,
   title=_"Close to home",
   body ="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=2F9131>" ..
_"Thron looks worried..." ..
"</p></rt><rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..

_[[ "The last days we came closer to our capital. Many people have already joined us on our march and set their hopes on me. However, I fear that we are not strong enough to take up the battle with my brothers.<br>For now we rest at the borders of the old forest and prepare for the coming days." ]] ..
"</p></rt>",
}

story_msg_2 = {
   posy=1,
   title=_"Another cold day",
   body ="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=2F9131>" ..
_"Thron looks worried." ..
"</p></rt><rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
_[[ "The news that a patrol brought is not good to hear. Our spies tell us, that the wartribes still rage in battle but some of the elder soldiers and tribe leaders seem to grow tired of this senseless war. Kalitath, son of Kun is the first warleader to leave Al’thunran, thus leaving his side weak and vulnerable. As my scouts report, he has set foot on the lower grounds around the capitol, securing the outer fortification rings." ]] ..
"</p></rt>",
}

story_msg_3 = {
   title=_"Doubts",
   posy=1,
   body ="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=2F9131>" ..
_"Boldreth shakes his head." ..
"</p></rt><rt image=map:boldreth.png><p line-spacing=3 font-size=12>" ..
_[[ "Thron seeks to believe that Kalitath is tired of fighting and will be willing to join us, once we march to re-establish law and order in Al’thunran. But I don’t think so. Kalitath is son to one of the mightiest generals of our forces and his family never quite liked the thought to bow before the banner of Chat’Karuth. Why should he march with his son to reclaim his heritage and the throne?" ]] ..
"</p></rt>",
}

story_msg_4 = {
   posy=1,
   title=_"The story continues",
   body= "<rt image=map:boldreth.png><p line-spacing=3 font-size=12>" ..
_[[ "However, Thron is not willing to listen to my doubts. He seeks to expand our fortifications to the borders of Kalitath’s territory and reunite with his forces. I have to keep an eye on everything that happens here." ]] ..
"</p></rt>",
}

story_msg_5 = {
   posy=1,
   title=_"Another cold day",
   body= "<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=2F9131>" ..
_"Thron speaks confidently." ..
"</p></rt><rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
_[[ "The frontier line of my forces has expanded to the edge of the forests. We have a long and cold winter and so many miles behind us. The fortification that we erected around Ondun will guarantee us fast supply of man and goods and once Al’thunran is in my hands, the two cities will be linked by a good and defendable infrastructure. Still I hope I will not walk alone on this mission, I hope Kailitat son of Kun will follow my call." ]] ..
"</p></rt>",
}

story_msg_6 = {
   posy=1,
   title=_"The story continues",
   body= "<rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
_[[ "I have sent men to meet Kalitath in this fortress. He has barricaded the former main entrance to the capital and is trying to set up a siege ring around Al’thunran. He will not let anyone enter or leave the city while the ring is intact. Until now, he has not answered any of my messages. But the time I wait won't be lost. I will further upgrade my soldiers and prepare for battle." ]] ..
"</p></rt>",
}

order_msg_7_destroy_kalitaths_army = {
   title=_"Your loyal companion",
   body= "<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=FF0000>" ..
_"Boldreth shouts out!" ..
"</p></rt><rt image=map:boldreth.png><p line-spacing=3 font-size=12>" ..
_[[ "This BASTARD! Only one man of our frontier patrol came back alive. He carried a message of Kalitath telling us that he will punish every move of our troops with their own blood! We cannot accept this behaviour. From now on there is war between Kalitath and us!" ]] ..
"<br><br>" ..
_"-- NEW OBJECTIVE --" ..
"<br>" ..
_"* Destroy Kalitaths army and expand your territory to the east." ..
"<br>" ..
_"-> To attack an enemy you must click on the door of an advarse military building. A menu will pop up allowing you to select the number of soldiers that should attack. When you are ready with setting the number, click on the cross ('start attack')" ..
"</p></rt>",
}

order_msg_7_renegade_fortification = {
   posy = 1,
   title=_"Renegade fortification",
   body= "<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=2F9131>" ..
_"Thron speaks:" ..
"</p></rt><rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
_[[ "The traitor has left his armies dying where they lay as he saw that he would not keep my forces back. Kalitath disappeared in the mess of the war, but I don’t care now. Shall he flee and be forgotten for all times as a tribeleader that would not bow before the wooden throne, that is mine to take now. Even more his flight brought us a big benefit: All year Kalitath was to busy fighting so that he did not even care to store enough food for all his men. Now that he fled the hunger brought some of his younger followers to desert. This group helds a strong fortification on the main ring. But there is more news: My scouts reported, that the two raging tribes of my brothers are in chaos and have barely noticed the new danger that is about to strike them. However they would not care if they noticed I guess, blinded by rage and hate, there is no sense left in them at all I believe." ]] ..
"</p></rt>",
}

briefing_msg_1 = {
   posy=1,
   title=_"Winter ahead",
   body= "<rt><p font-size=24 font-weight=bold font-face=FreeSerif font-color=2F9131>" ..
_"Boldreth speaks:" ..
"</p></rt><rt image=map:boldreth.png><p line-spacing=3 font-size=12>" ..
_[[ "This morning I found Thron trouble-minded. He returned from his patrol late last night and sat back against a tree watching over the encampment. The number of quarrels and disputes keeps growing and the winter has us firmly under control. We should put food in our storage to survive the long, cold winter!" ]] ..
"</p></rt>",
}

order_msg_1_small_food_economy = {
   posy=1,
   title=_"The advisor",
   body= "<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8080FF>" ..
_"Khantrukh nods" ..
"</p></rt><rt image=map:khantrukh.png><p line-spacing=3 font-size=12>" ..
_[[ "Boldreth is right! We don't have enough food to survive a long, cold winter. We should start to build a well, a fisher's and a hunter's hut. And if we find enough space, we should as well build up a farm and a bakery! One of our farmers seems to be very clever he already got some crops to grow in this cold soil." ]] ..
"<br><br>" ..
_"-- NEW OBJECTIVE --" ..
"<br>" ..
_"* Build up a basic food economy - a fisher's hut, a hunter's hut, a well, a farm and a bakery, to provide your people with food." ..
"</p></rt>",
}

story_note_1 = {
   posy=1,
   title=_"Food economy completed",
   body= "<rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
_[[ "Khantrukh just informed me, that our basic food economy was built up successfully. Finally none of us has to fear starvation anymore.]] ..
"</p></rt>",
}

order_msg_2_build_a_donjon = {
   posy=1,
   title=_"Tracks",
   body= "<rt image=map:boldreth.png><p line-spacing=3 font-size=12>" ..
   _[[ "Thron told me that he discovered tracks in the snow to the northeast of our settlement. He instructed us to build a Donjon, to have a further visual range on the area around our hall to protect our people from sudden attacks." ]] ..
   "<br><br>" ..
_"-- NEW OBJECTIVE --" ..
"<br>" ..
_"* Build a donjon at the northeast of your territory." ..
"</p></rt>",
}

order_msg_3_explore_further = {
   posy=1,
   title=_"Further explorations",
   body= "<rt image=map:boldreth.png><p line-spacing=3 font-size=12>" ..
_[[ "The first fortification Thron ordered has just been completed. At the moment he seeks to bring trust and belief to those who live inside the walls of our new habitat, that we named 'Ondun', which means 'those who wait' in the old tongue. But Thron is not yet satisfied - and I fully understand his fears: The forests are deep and the frontier to Al’thunran is near. I am sure that there are thousand bigger dangers out there than the unknown tracks.<br>To overlook more of the area around our hall we should explore further and set up more guards and scouts to observe the frontier and keep an eye on the raging wars on our doorsteps. THEN we will be prepared once trouble seeks to capture our woods." ]] ..
"<br><br>" ..
_"-- NEW OBJECTIVE --" ..
"<br>" ..
_"* Build more military buildings to explore the area around the headquarters and to ensure the safety of your people." ..
"</p></rt>",
}

order_msg_4_build_mining_economy = {
   posy=1,
   title=_"Mountains found",
   body= "<rt image=map:khantrukh.png><p line-spacing=3 font-size=12>" ..
_[[ "I just talked to the leader of our patrol. He told me, that they found some mountains to the north east and the east of our territory. With a bit of luck this opens us a new chance for rearming our men, because as far as I remember, most of the mountains west to Al'thunran hold coal and some metal ores." ]] ..
"<br><br>" ..
_"-- NEW OBJECTIVE --" ..
"<br>" ..
_"* Expand your territory to the mountains, send geologists to search for ore and coal and build a mining economy with mines, taverns, smelting works and metalworks." ..
"</p></rt>",
}

story_note_2 = {
   posy=1,
   title=_"Mining economy completed",
   body= "<rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
_[[ "I just got informed, that our mining economy started its work. These news brought a smile on my face - even if the hardest part is still waiting to be done, now our smiths will give their best to further ease our lives." ]] ..
"</p></rt>",
}

order_msg_5_build_a_fortress = {
   posy=1,
   title=_"Fortress needed",
   body= "<rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
_[[ "Boldreth and I just explored the area around the three mountains. One major strategic problem of that area seems to be, that there are two bigger passages - one north and one south to the middle mountain. Those passages are quite hard to control and so it seems to me, like we have only one chance to secure the mountains from enemy hands: We must build a fortress to the east of the mountains!" ]] ..
"<br><br>" ..
_"-- NEW OBJECTIVE --" ..
"<br>" ..
_"* Build a fortress to the east of the mountains." ..
"</p></rt>",
}

story_note_3 = {
   posy=1,
   title=_"Fortress completed",
   body= "<rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
_[[ "Finally!<br>The fortress was just completed, so the entrance to our settlement should be safe for now. For the future we might think on building some more fortifications like barriers and donjons at the passages, but for now we can take a deep breath." ]] ..
"</p></rt>",
}

order_msg_6_build_enhanced_economy_and_training = {
   posy=1,
   title=_"Enhanced economy and training",
   body= "<rt image=map:boldreth.png><p line-spacing=3 font-size=12>" ..
_[[ "Our mining economy seems to work fine, that is already good news! Anyway there are still a lot of men and women waiting in our hall to get a task, so they can help our soldiers to prepare for the future fights.]] ..
"<br>" ..
_[[Our geologists found a lot more deposits of iron ore, coal and even gold. We should enhance our current mines as soon as our miners are more experienced. We should build up some more mines, a bigger food infrastructure to always supply our men with sufficient food and a further processing economy. But that's still not everything - most of our soldiers are quite young and neither have a good weapon nor are they well trained. We should build up trainingssites to prepare them for their future tasks." ]] ..
"<br><br>" ..
_"-- NEW OBJECTIVES --" ..
"<br>" ..
_"* Build a warehouse to the west of the mountains." ..
"<br>" ..
_"* Build a battle arena and a trainings camp to the west of the mountains." ..
"<br>" ..
_"* As soon as the blacksmith reaches his next level (master blacksmith) enhance your metalworks to an axefactory or even a war mill and build up another metalworks to ensure the production of tools." ..
"<br>" ..
_"* Build a helmsmithy to the west of the mountains to provide your soldiers with better armor." ..
"</p></rt>",
}

story_msg_7 = {
   posy=1,
   title=_"Another cold day",
   body= "<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=2F9131>" ..
_"Thron looks worried." ..
"</p></rt><rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
_[[ "My brothers and their soldiers are dead and left fire and destruction behind. In none of all the ruins I found any man or woman of the normal folk and so I just may hope that they fled from this cruel battleground and started a better life somewhere else. Nothing is left from that beauty I remember, only ruins remain from the old capital. I am sorrowful but also relieved. I never hoped to ever see Al'thunrans old beauty again - few months ago I even wondered whether I will ever be able to set my feet near to Al'thunran again. Now the old town is back in my hands and I will not hesitate to rebuild it with all forces my people have. I can't wait to see it again in it's old beauty." ]] ..
"<br><br>" ..
_"--------------------- VICTORY! ----------------------" ..
"<br><br>" ..
_"You have completed this mission. You may continue playing if you wish, otherwise move on to the next mission." ..
"</p></rt>",
}

order_msg_7_free_althunran = {
   posy = 1,
   title=_"Military assault",
   body= "<rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
_[[ "So be it, the generals are out to set up my troops and Boldreth himself will lead the first strike. I will ride by his side, to free Al’thunran of this war and bring peace back in our capital. When the young sun rises above the trees tomorrow morning, I will order the assault. By dusk, I will celebrate my victory in the wooden halls of the warlord and sacrifice a newborn lamb in the honour of my father whose eyes lie upon me today.<br>So be it!" ]] ..
"<br><br>" ..
_"-- NEW OBJECTIVE --" ..
"<br>" ..
_"* Destroy all enforcements of Thron’s two brothers to liberate the throne-circle." ..
"</p></rt>",
}

cattlefarm_01 = {
   title = _ "Khantrukh speaks",
   body = rt("image=map:khantrukh.png", direct_speech(
_"Khantrukh approaches", p(_
[["Chieftain, remember the poor carriers: Our roads get longer and longer and
 the burden on their shoulders never gets lighter. We should breed more oxen to
 make their work lighter and our transportation more efficient."]]

       ))) .. rt(p(_"-- NEW OBJECTIVE --")) ..  obj_build_cattlefarm.body 
}
       

