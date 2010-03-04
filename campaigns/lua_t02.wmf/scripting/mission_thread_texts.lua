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


-- =======================================================================
--                                OBJECTIVES                                
-- =======================================================================
obj_claim_northeastern_rocks = {
   name = "mission quarry",
   title =_"Expand north-east and build a quarry",
   body = "<rt><p>" ..
_"* Build military buildings (like sentries and strongholds) to expand your territory.<br> Get to the rocks northeast from you and build a quarry there." ..
"</p></rt>"
}

obj_build_mines = {
name="mission mines",
title=_"Start building mines on the mountain",
body="<rt><p>" .. 
_"* Build a coal mine and an iron ore mine.<br> Place a flag up on the mountain's flank to the east (on mountain terrain though, not mountain meadow). When you click at implied flag you can send Geologists there. Because the flag is on mountains, the Geologists will search for ores; otherwise, they will search for water. Then build a mine for both kinds of resource he will find, choosing the mine to be built appropriately:<br> Black - coal<br> Red - iron ore<br> Yellow - gold<br> Grey - granite<br> Mines may only be built on mountain terrain. Suitable places for mines are displayed as hammer and pick." .. 
"</p></rt>"
}

-- TODO: objectives
obj_basic_food = {
   name="basic food began",
   title=_"Provide your miners with food",
   body="<rt><p>" .. 
_"* Build a hunter, a gamekeeper and a tavern.<br> In order to work, your miners need food.<br> A hunter can hunt down animals, while a gamekeeper prevents them from becoming extinct. The meat is then processed in a tavern into lunches for your miners.<br> This is only the first example of a ware which has to be refined before being used in a secondary building - others will follow." ..
"</p></rt>"
}

obj_begin_farming = {
name="farming began",
title=_"Bake bread",
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
body="<rt><p>" ..
_"* Enhance the coal mine and the tavern to the better versions of their buildings respectively.<br> A normal mine can only dig up about one third of all resources lying beneath it; then it must be enhanced to a deep mine in order to keep it working properly. To enhance a building, choose it and then click the appropriate button in the appearing menu.<br> Workers gain experience by successful work. With enough experience they become more advanced workers, who are necessary to operate the enhanced buildings. Do not enhance a building before you have enough advanced workers to operate the advanced building! Such buildings usually have greater demands than the basic kind of that building - for instance, deep mines need snacks instead of rations. You will have to enhance your tavern to an inn in order to produce snacks out of pitta bread AND a second kind of food (meat or fish). You may of course enhance all mines to deep mines instantly - bigger mines work a bit faster, smaller mines need cheaper food. It's up to you which strategy you prefer." ..
"</p></rt>"
}

-- TODO rest is not yet done
-- [Better Material I]
-- name=_"Build a wood hardener"
-- descr=_""<rt><p>"
-- _"* Build a wood hardener.<br> Bigger and better buildings - including all military ones - require better building materials. They cannot be built of simple trunks - the wood has to be refined to blackwood by a wood hardener first. Always remember to build a wood hardener before you run out of blackwood, as without it you cannot expand."
-- "</p></rt>""
-- visible=false
-- trigger=Built Hardener
--
-- [Better Material II]
-- name=_"Build a lime kiln and coal economy"
-- descr=_""<rt><p>"
-- _"* Build a lime kiln, a well and a charcoal burner or coal mine.<br> Better buildings may require also other improved materials besides blackwood. One of those is grout, which is produced out of raw stone, water and coal by a lime-burner. You can obtain water by building a well upon a water source, which your Geologists can discover when you send them to any flag that is not on a mountain, by clicking on the implied flag and the Geologist-symbol in the consequently appearing menu. Coal can be obtained by building a charcoal burner or a coal mine."
-- "</p></rt>""
-- visible=false
-- trigger=Mission Grindstone
--
-- [Better Material III]
-- name=_"Build a fernery"
-- descr=_""<rt><p>"
-- _"* Build a fernery.<br> The third material necessary for improved buildings is thatch reed, used to cover roofs. Thatch reed is planted by a fernery, around the building."
-- "</p></rt>""
-- visible=false
-- trigger=Mission Fernery
--
