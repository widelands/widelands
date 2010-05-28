-- =======================================================================
--                           Texts - No logic here
-- =======================================================================


-- ==================
-- Texts to the user
-- ==================
diary_page_1 = {
   title =_ "A dark night",
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=2F9131>" ..
   _"Diary of Lutius" ..
   "</p></rt><rt image=map:Lutius.png><p line-spacing=3 font-size=12><br>" ..
   _[[ "What has become of our Empire? I really ask this question - why did my king forbid me from fighting against that monsterous barbarian tribe, who first acted as a peaceful friend and then attacked my army in the darkest night?<br> With an army of 150 men, I was assigned to patrol on our northern frontier, which lies near the Galdin Mountains in a great, beautiful and ancient forest. Soon, we met a barbarian tribe, which at first was friendly. A few of my men even traded with them and their kids came to us, to admire our clean and tidy uniforms and weapons.<br> But in our fifth night in that region, they attacked us with no reason. I lost dozens of good men, and found myself imprisoned and brought to their chieftain. He spat at me, mocked me and told me to leave this land forever. He did not want new land and did not want our land, but he told me that THIS forest was their land - and would stay theirs until the ghosts take the last man of his folk!<br> Three weeks later, I came back to Fremil to speak with our king. He was furious at the Barbarians, yet disallowed an attack of these tribes. He believed that our army was too weak to survive in a war against the barbarians.<br> Still, I saw no way that I could erase my shame - without fighting against the Barbarians. So I left Fremil by boat to find myself a new world, and a new life, somewhere in the south. Now I am sailing on the perl-gulf with my family, some of my friends and a few of my best warriors." ]] ..
   "</p></rt>",
   width=500,
   height=450,
   posy=1,
}

diary_page_2 = {
   title=_"The rough sea",
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=2F9131>" ..
   _"Diary of Lutius" ..
   "</p></rt><rt image=map:Lutius.png><p line-spacing=3 font-size=12><br>" ..
   _[[ "It seems as if sailing on the perl-gulf was one of our biggest mistakes. Nature was against us and drove us into a dark, wild storm. I really don't know how many hours have passed since the waves rose higher than our boat, but still it would be suicide to go outside.<br> Our ship is badly damaged and is taking in more and more water. We can thank the Gods if we survive this black night with our lives." ]] ..
   "</p></rt>",
   width=400,
   height=350,
   posy=1,
}

diary_page_3 = {
   title=_"Survivor",
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=2F9131>" ..
   _"Diary of Lutius" ..
   "</p></rt><rt image=map:Lutius.png><p line-spacing=3 font-size=12><br>" ..
   _[[ "Finally, the Gods were with us!<br> We landed on an unknown coast and found peaceful rest under the palm trees growing at the shore. But that is about as good as it gets. The truth is:<br> when I woke up this morning, I saw nothing more than sand around us.<br> It really seems as if we landed under the only palm trees existing in this far-away sandy desert. I fear we won't find the help we need to get our ship repaired in good time." ]] ..
   "</p></rt>",
   width=400,
   height=350,
   posy=1,
}

saledus_1 = {
   title=_"A foreboding",
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8F9131>" ..
   _"Saledus looks around nervously..." ..
   "</p></rt><rt image=map:Saledus.png><p line-spacing=3 font-size=12><br>" ..
   _[[ "Sire, I fear we are not safe in this foreign land. Who knows what terrible creatures live beyond this forest, in that mighty desert? What if these creatures enter the woods and await the time to attack us?<br> Well, perhaps my fear is misguided, but it can't be wrong to keep watch in the forests - so that we can be sure to see any potential enemies before they can see us.<br> We really should build a barracks in the eastern portion of the forest. Then one of us can keep watch in the darkness and keep us safe from these creatures."<br><br>-- NEW OBJECTIVE --<br> ]] ..
   _"* Build a barracks at the red point on the east side of the forests, to the right of your headquarters." ..
   "</p></rt>",
   width=400,
   height=350,
}


saledus_2 = {
   title=_"Safe for now",
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8F9131>" ..
   _"Saledus speaks with a sigh of relief..." ..
   "</p></rt><rt image=map:Saledus.png><p line-spacing=3 font-size=12><br>" ..
   _[[ "Sire, I saw that the construction of the barracks was complete, so I assigned one of my best soldiers to it to keep watch on the desert.<br> This is a good step forward. Now we can feel a bit safer and can look forward to repair our ship." ]] ..
   "</p></rt>",
   width=400,
   height=350,
}
amalea_1 = {
   title=_"Young Amalea",
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=AF7511>" ..
   _"Amalea smiles..." ..
   "</p></rt><rt image=map:Amalea.png><p line-spacing=3 font-size=12><br>" ..
   _[[ "Hey Lutius, I just met Saledus outside.<br> He told me about the barracks. Well, I am not one of those girls who think we need more barracks - instead, we might concentrate on other things now.<br> It is absolutely clear that we need wood to repair our ship. So, I walked through the forest yesterday and found a nice place for a lumberjack's house, south of our headquarters."<br><br>-- NEW OBJECTIVE --<br> ]] ..
   _(
   "* Build a lumberjack's house at the red point, south of your headquarters. " ..
   "Cutting down enough trees for more buildings will take a while. You can " ..
   "speed the game up using PAGE UP and slow it down again with PAGE DOWN."
   ) .. 

   "</p></rt>",
   width=400,
   height=350,
}

amalea_2 = {
   title=_"Young Amalea",
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=AF7511>" ..
   _"Amalea recommends..." ..
   "</p></rt><rt image=map:Amalea.png><p line-spacing=3 font-size=12><br>" ..
   _[[ "Hey Lutius, I got the message that our first lumberjack started his work today. Perhaps it would be a good idea to wait until he cleans enough space for building up two further lumberjack's houses, so that we can harvest the trunks faster.<br> But unfortunately the trunks are useless for repairing our ship - we need refined lumber, like every ship and every bigger building needs, too. So we should build up a sawmill - as soon as we have enough space for this."<br><br>-- NEW OBJECTIVE --<br> ]] ..
   _"* Build two more lumberjack's houses and a sawmill as soon as there is enough space for them." ..
   "</p></rt>",
   width=400,
   height=350,
}

amalea_3 = {
   title=_"Young Amalea",
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=AF7511>" ..
   _"Amalea comes in..." ..
   "</p></rt><rt image=map:Amalea.png><p line-spacing=3 font-size=12><br>" ..
   _[[ "Hi Lutius, I've got two important things to talk about... First the good news:<br> I saw that the construction of the sawmill is complete, so we can begin to refine the trunks that the lumberjacks are harvesting into lumber.<br> But that's just about the only positive thing I know to talk about. The bad news is that our lumberjacks harvest at an unbelievable speed. There are almost no trees left on this island.<br> Lutius, these trees provide shelter from the sandstorms from the desert, shade on the hot days and they are the home of so many gentle animals. We shouldn't leave this island a complete desert.<br> Lutius, I beg you: find someone who takes care about planting new trees."<br><br>-- NEW OBJECTIVE --<br> ]] ..
   _"* Build a forester's house to preserve the wood resources of this island." ..
   "</p></rt>",
   width=400,
   height=350,
}

saledus_3 = {
   title=_"Thinking about the future",
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8F9131>" ..
   _"Saledus notes..." ..
   "</p></rt><rt image=map:Saledus.png><p line-spacing=3 font-size=12><br>" ..
   _[[ "Hey, my good old friend. I just thought about the rocks standing on the south shore of this land. Perhaps we could cut out some useful hard stones and beautiful marble.<br> The repair of our ship will take a few weeks, anyway, and the resources we harvest now might be the base of strong and big buildings in another land."<br><br>-- NEW OBJECTIVE --<br> ]] ..
   _"* Build a quarry in the south to cut some stones and marble out of the rocks.<br> These might be used for future buildings." ..
   "</p></rt>",
   width=400,
   height=350,
}

saledus_4 = {
   title=_"A step forward",
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8F9131>" ..
   _"Saledus looks promising..." ..
   "</p></rt><rt image=map:Saledus.png><p line-spacing=3 font-size=12><br>" ..
   _[[ "Hey Lutius, today we got a lot closer to our first castle. The quarry to the south began its work today and will soon provide us with stones and beautiful marble.<br> Now we truly can look forward to settling down on another island." ]] ..
   "</p></rt>",
   width=400,
   height=350,
}

diary_page_4 = {
   title=_"Mission Complete",
   body="<rt image=map:Lutius.png><p line-spacing=3 font-size=12><br>" ..
_[[ "Today I got the message that our ship is completely repaired. At the moment my people are loading everything onto our newly repaired ship, hoping it will serve us better than last time.<br> Tonight will be our last night on this island. Tomorrow morning we will leave, searching for a new place for our exile.<br> I still wonder if we will ever see Fremil again.<br><br>--------------------- VICTORY! ----------------------<br><br> You have completed this mission. You may continue playing if you wish, otherwise move on to the next mission." ]] ..
"</p></rt>",
   posy=1
}

safe_peninsula = {
   title=_"Safe Peninsula",
   body="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=2F9131>" ..
   _"Saledus speaks:" ..
   "</p></rt><rt image=map:Saledus.png><p line-spacing=3 font-size=12><br>" ..
   _[[ "You have made this peninsula a very secure place. It is time to move on!" ]] ..
   "</p></rt>"
}

-- ===========
-- objectives
-- ===========
obj_build_barracks = {
   name = "build_barracks",
   title=_"Build a barracks",
   body= "<rt><p>" ..
   _"* Build a barracks at the red point on the east side of the forests, to the right of your headquarters." ..
   "</p></rt>"
}
obj_build_lumberjack = {
   name = "build_lumberjack",
   title=_"Build a lumberjack's house",
   body= "<rt><p>" ..
   _(
   "* Build a lumberjack's house at the red point, south of your headquarters. " ..
   "Cutting down enough trees for more buildings will take a while. You can " ..
   "speed the game up using PAGE UP and slow it down again with PAGE DOWN."
   ) .. 
   "</p></rt>"
}
obj_build_sawmill_and_lumberjacks = {
   name = "build_sawmill_and_lumberjacks",
   title=_"Build two lumberjack's houses and a sawmill",
   body="<rt><p>" ..
   _"* Build two more lumberjack's houses and a sawmill as soon as there is enough space for them." ..
   "</p></rt>"
}
obj_build_forester = {
   name = "build_forester",
   title=_"Build a forester's house",
   body="<rt><p>" ..
   _"* Build a forester's house to preserve the wood resources of this island." ..
   "</p></rt>"
}
obj_build_quarry = {
   name = "build_quarry",
   title=_"Build a quarry",
   body="<rt><p>" ..
   _"* Build a quarry in the south to cut some stones and marble out of the rocks.<br> These might be used for future buildings." ..
   "</p></rt>"
}

