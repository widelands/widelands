-- =======================================================================
--                     Texts for the Main Mission Thread                    
-- =======================================================================

briefing_msg_1 = {
   posy = 1,
   title =_ "The story continues",
   body =
"<rt><p font-size=24 font-weight=bold font-face=FreeSerif font-color=2F9131>" ..
_"Thron shakes his head..." ..
"</p></rt><rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
_[[ "The war goes on. More and more of our brothers and sisters flee the brutal war raging in the capital beneath the trees.<br> The stories they tell about the deeds of our kin are sad to hear. I've spent nights lying awake, restless, more tired I ever believed one could be. Yet whenever I close my eyes, I see the fortress my father built consumed by flames. The throne between the trees, the symbol of unity and peace among our kin became the wedge that drives us apart." ]] ..
"</p></rt>",
}

briefing_msg_2 = {
   posy = 1,
   title =_ "The story continues",
   body = 
"<rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
_ [[ "Today my hunters brought men, women and little children who had hidden out in the forests, trying to escape the war, hate and revenge that rage among the tribes fighting each other like in old times, when we were no more but the wild beasts driven and controlled by instincts. None of my brothers will ever gain and hold control over the wooden throne, none of the tribes will be strong enough to subdue the other. There will be no end to this slaughtering, unless... is this it? As father told me?<br> Rise against whoever threatens our very existence, may it even be one of your blood or mind?" ]] ..
"</p></rt>",
}


briefing_msg_3 = {
   posy = 1,
   title =_ "The story continues",
   body =
"<rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
_[[ "Boldreth seems more and more torn as the days go by. The spirits of my fellows sink as the cold season approaches, and we are still living in no more than huts and barracks. I never intended to stay out here in the wilderness for so long - but I never thought my brothers would engage in this senseless battle for so long either.<br> Perhaps it's time to make ourselves feel a little more at ease here. Perhaps it's time to give those who still live and think united a new home, replacing what is now lost to us? Until we can return to the place we once called our home..." ]] ..
"</p></rt>"
}


order_msg_1 = {
   title=_"Your loyal companion",
   body =
"<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=FF0000>" ..
_"Brave Boldreth steps aside you..." ..
"</p></rt><rt image=map:boldreth.png><p line-spacing=3 font-size=12>" ..
_[[ "So you want to prepare us for a longer stay in these forests, Thron? Well, I see the wisdom in doing so, yet my heart is yearning for a glorious return to Al'thunran. Too long have we waited while our brethren have fought on the battlefield, and our axes and swords have turned rusty over time." ]] ..
"</p></rt>"
}


order_msg_2 = {
   title =_ "The advisor",
   body =
"<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8080FF>" ..
_"Khantrukh joins the both of you..." ..
"</p></rt><rt image=map:khantrukh.png><p line-spacing=3 font-size=12>" ..
_[[ "I beg your forgiveness, chieftain, but I have a grave concern to discuss with you. The rocks close to our camp are shrinking fast under our workers' tools, and it is a matter of little time only before they are all gone." ]] ..
"</p></rt>"
}

order_msg_3 = {
   title=_"Your loyal companion", 
   posx = 0,
   body =
"<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=FF0000>" ..
_"Boldreth jumps up..." ..
"</p></rt><rt image=map:boldreth.png><p line-spacing=3 font-size=12>" ..
_[[ "While it is certainly no worthy task for a warrior, at least it is something to be done! Thron, what do you say about me taking a few of our most restless warriors and venturing somewhat closer to the great mountain of Kal'mavrath - maybe we will find some more big stones to please the old man?"<br><br>-- NEW OBJECTIVE --<br> ]] ..
_"* Build military buildings (like sentries and strongholds) to expand your territory.<br> Get to the rocks northeast from you and build a quarry there." .. 
"</p></rt>"
}

order_msg_4 = {
   title =_ "The advisor",
   posx = 0,
   body =
"<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8080FF>" ..
_"Khantrukh grumbles..." ..
"</p></rt><rt image=map:khantrukh.png><p line-spacing=3 font-size=12>" ..
_ [[ "Old man? Disrespectful youth!" ]] ..
"</p></rt>"
}

order_msg_5_quarry = {
   title=_"The advisor",
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8080FF>" ..
_"Khantrukh nods satisfied..." ..
"</p></rt><rt image=map:khantrukh.png><p line-spacing=3 font-size=12>" ..
_[[ "At last! These rocks should last for a while." ]] ..
"</p></rt>"
}


order_msg_6_geologist = {
   title=_"Your loyal companion",
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=FF0000>" ..
_"Boldreth exclaims..." ..
"</p></rt><rt image=map:boldreth.png><p line-spacing=3 font-size=12>" ..
_[[ "Just look at that! In the east is the great mountain of Kal'mavrath! I wonder what treasures nature might have hidden beneath its majestic flanks!<br> Let's expand to the east. There's a chance for the elderly to become useful once in a while! Let us send out some of those who understand the stone's tongue to unravel the mountain's secrets!"<br><br>-- NEW OBJECTIVE --<br> ]] ..
_"* Build a coal mine and an iron ore mine.<br> Place a flag up on the mountain's flank to the east (on mountain terrain though, not mountain meadow). When you click at implied flag you can send Geologists there. Because the flag is on mountains, the Geologists will search for ores; otherwise, they will search for water. Then build a mine for both kinds of resource he will find, choosing the mine to be built appropriately:<br> Black - coal<br> Red - iron ore<br> Yellow - gold<br> Grey - granite<br> Mines may only be built on mountain terrain. Suitable places for mines are displayed as hammer and pick." ..
"</p></rt>"
}

order_msg_7_mines_up = {
   title=_"Your loyal companion",
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=FF0000>" ..
_"Boldreth laughs..." ..
"</p></rt><rt image=map:boldreth.png><p line-spacing=3 font-size=12>" ..
_[[ "By Chat'Karuth's beard, this is awesome! Just imagine what we can use this coal and iron ore for!<br> It might even be enough to..." ]] ..
"</p></rt>"
}

order_msg_8_mines_up = {
   title=_"The advisor",
   body ="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8080FF>" ..
_"Khantrukh interrupts..." ..
"</p></rt><rt image=map:khantrukh.png><p line-spacing=3 font-size=12>" ..
_[[ "While this is true, we should consider it later, for now other obstacles are at hand. The miners will definately demand greater rations for working high up on the mountain in this bitter cold, and providing these supplies may not be easy." ]] ..
"</p></rt>"
}


order_msg_9_hunter = {
   title=_"Your loyal companion",
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=FF0000>" ..
_"Boldreth nods..." ..
"</p></rt><rt image=map:boldreth.png><p line-spacing=3 font-size=12>" ..
_[[ "I have to hand it to you: you are right here, old man.<br> There seem to be quite many animals in the forests here - we might just hunt down more of them. And, of course, the people would cheer to a new tavern - hey, we might call it 'Thron's Pride' or so if you want!"<br><br>-- NEW OBJECTIVE --<br> ]] ..
_"* Build a hunter, a gamekeeper and a tavern.<br> In order to work, your miners need food.<br> A hunter can hunt down animals, while a gamekeeper prevents them from becoming extinct. The meat is then processed in a tavern into lunches for your miners.<br> This is only the first example of a ware which has to be refined before being used in a secondary building - others will follow." ..
"</p></rt>"
}

order_msg_10_bread = {
   title=_"The advisor",
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8080FF>" ..
_"The elder jumps into the air..." ..
"</p></rt><rt image=map:khantrukh.png><p line-spacing=3 font-size=12>" ..
_[[ "Wisdom commands to seek variety.<br> North of the great mountain is a large plain - why don't we use the space Mother Nature gave us and build a farm? I sure would enjoy a freshly baken pitta bread for a change..."<br><br>-- NEW OBJECTIVE --<br> ]] ..
_"* Build a well, a farm and a bakery.<br> Other ways to produce food - different kinds of food - are fishers and hunters. The wheat of the farms has to be processed with water in a bakery before it becomes edible. You can obtain water by building a well at a spot where your Geologists find a water source." ..
"</p></rt>"
}

order_msg_11_basic_food_began = {
   title=_"Your loyal companion",
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=FF0000>" ..
_"Boldreth cheers up..." ..
"</p></rt><rt image=map:boldreth.png><p line-spacing=3 font-size=12>" ..
_[[ "Our hunters are out in the forests, Thron.<br> I promise you, before the sun sets today you will have a magnificient meal fit for the chieftain of all clans!" ]] ..
"</p></rt>"
}

order_msg_12_farming_began = {
   title=_"The advisor",
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8080FF>" ..
_"Khantrukh gazes over the plains..." ..
"</p></rt><rt image=map:khantrukh.png><p line-spacing=3 font-size=12>" ..
_[[ "Isn't it lovely, the view of golden fields, growing peacefully, dancing to the wind? And I believe the baker is just on his way here, with his most delicious bread reserved for our chieftain alone!" ]] ..
"</p></rt>"
}

order_msg_13_tavern = {
   title=_"A beautiful morning",
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=2F9131>" ..
_"Thron recognizes..." ..
"</p></rt><rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
_[[ "A tavern opened for our people yesterday. While I am hardly in the mood for celebration, I noticed how much this tiny bit of home meant to my people. Their songs filled the air until the deep night, and they were in higher spirit still the day after.<br> Maybe we actually are slowly creating a place here which we can... call home." ]] ..
"</p></rt>"
}

order_msg_14_refine_ore = {
   title=_ "Your loyal companion",
   body= "<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=FF0000>" ..
_"Boldreth remarks..." ..
"</p></rt><rt image=map:boldreth.png><p line-spacing=3 font-size=12>" ..
_[[ "So now that we have both the mines and the food to sustain them, we should bring what our brave miners gain for us into a more useful state. The iron ore has to be melted into iron so that it is ready to be forged into whatever we want. And we need the coal to keep the smelter's ovens burning."<br><br>-- NEW OBJECTIVE --<br> ]] ..
_"* Build a smelting works. The iron ore your miners bring up is not usable yet - it has to be melted into iron first." ..
"</p></rt>",
}

order_msg_15_mines_exhausted = {
   title=_ "The advisor", 
   body = "<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8080FF>" ..
_"Khantrukh speaks..." ..
"</p></rt><rt image=map:khantrukh.png><p line-spacing=3 font-size=12>" ..
_[[ "Our miners are digging up less and less by day! We have to go deeper, closer to the mountain's core, if we want more of its treasures!<br> Of course, it is dark and cold in such depths and only few venture voluntarily into these places. We should reward this bravery with greater rations for them."<br><br>-- NEW OBJECTIVE --<br> ]] ..
_"* Enhance the coal mine and the tavern to the better versions of their buildings respectively.<br> A normal mine can only dig up about one third of all resources lying beneath it; then it must be enhanced to a deep mine in order to keep it working properly. To enhance a building, choose it and then click the appropriate button in the appearing menu.<br> Workers gain experience by successful work. With enough experience they become more advanced workers, who are necessary to operate the enhanced buildings. Do not enhance a building before you have enough advanced workers to operate the advanced building! Such buildings usually have greater demands than the basic kind of that building - for instance, deep mines need snacks instead of rations. You will have to enhance your tavern to an inn in order to produce snacks out of pitta bread AND a second kind of food (meat or fish) AND beer. Beer is produced in a micro brewery out of water and wheat.<br> Consider that an inn produces both rations and snacks. So the production of rations for not yet enhanced mines will be lower. Therefore, you have to decide how many taverns to enhance. The same with the mines: You may of course enhance all mines to deep mines instantly - bigger mines work a bit faster, smaller mines need cheaper food. It's up to you which strategy you prefer." ..
"</p></rt>"
}

order_msg_16_blackwood = {
   title=_"Your loyal companion",
   body= "<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=FF0000>" ..
_"Boldreth seems concerned..." ..
"</p></rt><rt image=map:boldreth.png><p line-spacing=3 font-size=12>" ..
_[[ "As I just discovered, we are running short on blackwood! We cannot put our warriors into some crumbling huts, and even less so in times as dangerous as ours!<br> We need a wood hardener, and we need one now!"<br><br>-- NEW OBJECTIVE --<br> ]] ..
_"* Build a wood hardener.<br> Bigger and better buildings - including all military ones - require better building materials. They cannot be built of simple trunks - the wood has to be refined to blackwood by a wood hardener first. Always remember to build a wood hardener before you run out of blackwood, as without it you cannot expand." ..
"</p></rt>"
}

order_msg_17_grindstone = {
   title=_"Your loyal companion",
   body= "<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=FF0000>" ..
_"Boldreth smiles..." ..
"</p></rt><rt image=map:boldreth.png><p line-spacing=3 font-size=12>" ..
_[[ "Well, old friend, this should ensure that our fortifications do not break down with our foes' first battle cry! Now we can expand safely!<br> Still, it would not hurt to assemble some grout for our further campaign; thus we could, in times of need, build a fortress as the world has not seen before!"<br><br>-- NEW OBJECTIVE --<br> ]] ..
_"* Build a lime kiln, a well and a charcoal burner or coal mine.<br> Better buildings may require also other improved materials besides blackwood. One of those is grout, which is produced out of raw stone, water and coal by a lime-burner. You can obtain water by building a well upon a water source, which your Geologists can discover when you send them to any flag that is not on a mountain, by clicking on the implied flag and the Geologist-symbol in the consequently appearing menu. Coal can be obtained by building a charcoal burner or a coal mine." ..
"</p></rt>"
}


order_msg_18_fernery = {
   title=_"The advisor", 
   body= "<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8080FF>" ..
_"Khantrukh steps in..." ..
"</p></rt><rt image=map:khantrukh.png><p line-spacing=3 font-size=12>" ..
_[[ "Chieftain, this is a disgrace! It is well that we can produce grout for mighty fortifications and great buildings now - only this does not prevent our roofs from becoming leaky! Maybe the young ones like spending their nights in the rain, but I just can't find any sleep with these raindrops dripping on my face!<br> Now this is a problem we should do something about!"<br><br>-- NEW OBJECTIVE --<br> ]] ..
_"* Build a fernery.<br> The third material necessary for improved buildings is thatch reed, used to cover roofs. Thatch reed is planted by a fernery, around the building." ..
"</p></rt>"
}

order_msg_19_all_material = {
   title=_"The advisor",
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8080FF>" ..
_"Khantrukh seems relieved..." ..
"</p></rt><rt image=map:khantrukh.png><p line-spacing=3 font-size=12>" ..
_[[ "Wonderful! Our roofs are not dripping anymore, and we have all the material we need for, well, whichever monumental building we see fit!" ]] ..
"</p></rt>"
}

msg_mission_complete = {
   title=_"Mission Complete",
   body="<rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
_[[ "The other day Boldreth asked me to accompany him to the new inn. It would cheer me up, he said.<br> When I looked around, I saw faithful faces, trusting that I could guide them through these dark days. Yet before I could speak any words of gratitude or encouragement, one of my warriors ran into the inn. He had been far out in the forest for the past days and I could see how weary he was.<br> The news he brought changed everything..."<br><br>--------------------- VICTORY! ----------------------<br><br> You have completed this mission. You may continue playing if you wish, otherwise move on to the next mission.]] .. 
"</p></rt>"
}

msg_story_1 = {
   title=_"One fullmoon night",
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=2F9131>" ..
_"Thron speaks..." ..
"</p></rt><rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
_[[ "One night, when the moon shone brightly, I climbed to the peak of Kal'mavrath.<br> When I gazed at the horizon, I still saw crimson lights flicker in the distance. It is incredible how much passion my brethren fight this war with. I fear the moment I see Al'thunran again - will there be anything but ashes and wasted ruins left of our once beautiful capital when we get there?" ]] ..
"</p></rt>"
}

msg_story_2 = {
    title= _"Another cold day",
    body= "<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=2F9131>" ..
 _"Thron looks worried..." ..
 "</p></rt><rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
 _[[ "The winter is upon us. Many of us are suffering from the cold, yet we must endure. The day we may return cannot be far anymore - it must not be far anymore.<br> I prayed it might get warmer again, as I prayed the war would finally come to an end.<br> So far, it seems that neither prayer was fulfilled." ]] ..
 "</p></rt>"
}

msg_village = {
   title=_"A friendly village joined us!",
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=2F9131>" ..
_"Thron speaks..." ..
"</p></rt><rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
_[[ "We have found a village with friendly and productive people, impressed by our wealth, technology and strength.<br> They have decided to join us!" ]] ..
"</p></rt>"
}

-- =======================================================================
--                                OBJECTIVES                                
-- =======================================================================
obj_claim_northeastern_rocks = {
   name = "mission quarry",
   title =_"Expand north-east and build a quarry",
   check = function(obj)
      local f = wl.map.Field(27,48):region(6)
      while not check_for_buildings(wl.game.Player(1), { quarry = 1 }, f) do 
         sleep(5000)
      end

      obj.done = true
      send_msg(order_msg_5_quarry)

      quarry_done = true
   end, 
   body = "<rt><p>" ..
_"* Build military buildings (like sentries and strongholds) to expand your territory.<br> Get to the rocks northeast from you and build a quarry there." ..
"</p></rt>",
}

obj_build_mines = {
name="mission mines",
title=_"Start building mines on the mountain",
body="<rt><p>" .. 
_"* Build a coal mine and an iron ore mine.<br> Place a flag up on the mountain's flank to the east (on mountain terrain though, not mountain meadow). When you click at implied flag you can send Geologists there. Because the flag is on mountains, the Geologists will search for ores; otherwise, they will search for water. Then build a mine for both kinds of resource he will find, choosing the mine to be built appropriately:<br> Black - coal<br> Red - iron ore<br> Yellow - gold<br> Grey - granite<br> Mines may only be built on mountain terrain. Suitable places for mines are displayed as hammer and pick." .. 
"</p></rt>"
}

obj_basic_food = {
   name="basic food began",
   title=_"Provide your miners with food",
   check = function(obj) 
      local p = wl.game.Player(1)
      local tavern_msg_done = nil
      local hunter_msg_done = nil
      while true do
         local rv = p:get_buildings{"hunters_hut", "gamekeepers_hut", "tavern"}
         if #rv.hunters_hut >= 1 and not hunter_msg_done then
            send_msg(order_msg_11_basic_food_began)
            hunter_msg_done = true
         end
         if #rv.tavern >= 1 and not tavern_msg_done then
            send_msg(order_msg_13_tavern)
            tavern_msg_done = true
         end
         if #rv.hunters_hut >= 1 and #rv.gamekeepers_hut >= 1
                  and #rv.tavern >= 1 then break end
         sleep(5331)
      end
      obj.done = true
   end,
   body="<rt><p>" .. 
_"* Build a hunter, a gamekeeper and a tavern.<br> In order to work, your miners need food.<br> A hunter can hunt down animals, while a gamekeeper prevents them from becoming extinct. The meat is then processed in a tavern into lunches for your miners.<br> This is only the first example of a ware which has to be refined before being used in a secondary building - others will follow." ..
"</p></rt>"
}

obj_begin_farming = {
name="farming began",
title=_"Bake bread",
check = function(obj) 
   while 1 do
      local rv = p:get_buildings{"well", "bakery", "farm"}
      if #rv.well >= 1 and #rv.bakery >= 1 and #rv.farm >= 1 then
         break
      end
      sleep(4234)
   end
   send_msg(order_msg_12_farming_began)
   obj.done = true
end,
body="<rt><p>" ..
_"* Build a well, a farm and a bakery.<br> Other ways to produce food - different kinds of food - are fishers and hunters. The wheat of the farms has to be processed with water in a bakery before it becomes edible. You can obtain water by building a well at a spot where your Geologists find a water source." ..
"</p></rt>"
}

obj_refine_ores = {
name="refining began",
title=_"Refine your mined resources",
body="<rt><p>" ..
_"* Build a smelting works. The iron ore your miners bring up is not usable yet - it has to be melted into iron first." ..
"</p></rt>"
}

obj_enhance_buildings = {
name="enhance buildings",
title=_"Enhance Buildings",
check = function (obj)
   local p = wl.game.Player(1)
   while true do
      rv = p:get_buildings{"inn", "deep_coalmine"}
      if #rv.inn >= 1 and #rv.deep_coalmine >= 1 then
         break
      end
      sleep(5742)
   end
   obj.done = true
   enhance_buildings_done = true
end,
body="<rt><p>" ..
_"* Enhance the coal mine and the tavern to the better versions of their buildings respectively.<br> A normal mine can only dig up about one third of all resources lying beneath it; then it must be enhanced to a deep mine in order to keep it working properly. To enhance a building, choose it and then click the appropriate button in the appearing menu.<br> Workers gain experience by successful work. With enough experience they become more advanced workers, who are necessary to operate the enhanced buildings. Do not enhance a building before you have enough advanced workers to operate the advanced building! Such buildings usually have greater demands than the basic kind of that building - for instance, deep mines need snacks instead of rations. You will have to enhance your tavern to an inn in order to produce snacks out of pitta bread AND a second kind of food (meat or fish). You may of course enhance all mines to deep mines instantly - bigger mines work a bit faster, smaller mines need cheaper food. It's up to you which strategy you prefer." ..
"</p></rt>"
}

obj_better_material_1 = {
name="built hardener",
title=_"Build a wood hardener",
body="<rt><p>" ..
_"* Build a wood hardener.<br> Bigger and better buildings - including all military ones - require better building materials. They cannot be built of simple trunks - the wood has to be refined to blackwood by a wood hardener first. Always remember to build a wood hardener before you run out of blackwood, as without it you cannot expand." ..
"</p></rt>"
}

obj_better_material_2 = {
name="mission grindstone",
title=_"Build a lime kiln and coal economy",
body="<rt><p>" ..
_"* Build a lime kiln, a well and a charcoal burner or coal mine.<br> Better buildings may require also other improved materials besides blackwood. One of those is grout, which is produced out of raw stone, water and coal by a lime-burner. You can obtain water by building a well upon a water source, which your Geologists can discover when you send them to any flag that is not on a mountain, by clicking on the implied flag and the Geologist-symbol in the consequently appearing menu. Coal can be obtained by building a charcoal burner or a coal mine." ..
"</p></rt>"
}

obj_better_material_3 = {
name="mission fernery",
title=_"Build a fernery",
body="<rt><p>"..
_"* Build a fernery.<br> The third material necessary for improved buildings is thatch reed, used to cover roofs. Thatch reed is planted by a fernery, around the building." ..
"</p></rt>"
}


