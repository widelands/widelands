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

function h2(s)
   return "<p font=FreeSerif font-size=12 font-weight=bold font-color=D1D1D1>"
      ..  s .. "<br></p><p font-size=4> <br></p>"
end

-- Simple flowing text. One Paragraph
function p(s)
   return "<p line-spacing=3 font-size=12>" .. s .. "<br></p>" ..
      "<p font-size=8> <br></p>"
end

-- Direct speech by one of the persons that appear in the map
function speech(img, clr, g_title, g_text)
   local title, text = g_title, g_text
   if not text then
      title = nil
      text = g_title
   end
     
   local s = ""
   if title then
      s = rt("<p font-size=20 font-weight=bold font-face=FreeSerif " .. 
         ("font-color=%s>"):format(clr) .. title ..
         "</p><p font-size=8> <br></p>"
      )
   end

   return s .. rt(("image=%s"):format(img), p(text))
end

function thron(title, text)
   return speech("map:chieftain.png", "2F9131", title, text)
end
function khantrukh(title, text)
   return speech("map:khantrukh.png", "8080FF", title, text)
end
function boldreth(title, text)
   return speech("map:boldreth.png", "FF0000", title, text)
end

function obj_text(heading, body)
   return rt(h2(heading) .. p(body))
end

function new_objectives(...) 
   local s = rt(
   "<p font=FreeSerif font-size=18 font-weight=bold font-color=D1D1D1>"
   .. _"New Objectives" .. "</p>")

   for idx,obj in ipairs{...} do
      s = s .. obj.body
   end
   return s
end


-- =======================================================================
--                                OBJECTIVES                                
-- =======================================================================
obj_claim_northeastern_rocks = {
   name = "mission quarry",
   title =_"Expand north-east and build a quarry",
   body = obj_text(_"Expand north-east to the stones", _
[[Build military buildings (like sentries and strongholds) to expand your
territory. Get to the rocks northeast from you and build a quarry there.]]
   ),
}

obj_build_mines = {
name="mission mines",
title=_"Start building mines on the mountain",
body= obj_text(_"Build coal and iron ore mines", _
[[Build a coal mine and an iron ore mine. To do so, place a flag up on the
mountain's flank to the east (on mountain terrain though, not mountain meadow).
When you click at the new flag you can send geologists there. Because the flag
is on mountains, the geologists will search for ores; otherwise, they will
search for water. Then build a mine for both kinds of resource he will find,
choosing the mine to be built appropriately:<br> Black - coal<br> Red - iron
ore<br> Yellow - gold<br> Grey - granite<br> Mines may only be built on
mountain terrain. Suitable places for mines are displayed as hammer and pick.]]
   )
}

obj_basic_food = {
   name="basic food began",
   title=_"Provide your miners with food",
   body= obj_text(_"Build a hunter, a gamekeeper and a tavern", _
[[In order to work, your miners need food.<br> A hunter can hunt down animals,
while a gamekeeper prevents them from becoming extinct. The meat is then
processed in a tavern into lunches for your miners.<br> This is only the
first example of a ware which has to be refined before being used in a
secondary building - others will follow.]]
   )
}

obj_begin_farming = {
name="farming began",
title=_"Bake bread",
body=obj_text(_ "Build a well, a farm and a bakery", _
[[Other ways to produce food - different kinds of food - are fishers and
hunters. The wheat of the farms has to be processed with water in a bakery
before it becomes edible. You can obtain water by building a well at a spot
where your geologists find a water source.]]
   )
}

obj_refine_ores = {
name="refining began",
title=_"Refine your mined resources",
body= obj_text(_ "Build a smelting works", _
[[The iron ore your miners bring up is not usable yet - it has to be melted
into iron first.]]
   )
}

obj_enhance_buildings = {
name="enhance buildings",
title=_"Enhance buildings and build a micro brewery",
body= obj_text(_
[[Enhance the coal mine and the tavern to the better versions of their
buildings respectively. Also build a micro brewery.]], _
[[A normal mine can only dig up about one third of all resources lying beneath
it; then it must be enhanced to a deep mine in order to keep it working
properly. To enhance a building, choose it and then click the appropriate
button in the appearing window.<br> Workers gain experience by successful work.
With enough experience they become more advanced workers, who are necessary to
operate the enhanced buildings. Do not enhance a building before you have
enough advanced workers to operate the advanced building! Such buildings
usually have greater demands than the basic kind of that building - for
instance, deep mines need snacks instead of rations. You will have to enhance
your tavern to an inn in order to produce snacks out of pitta bread AND a
second kind of food (meat or fish) AND beer. You may of course enhance all
mines to deep mines instantly given you have the workers - bigger mines work a
bit faster, smaller mines need cheaper food. It's up to you which strategy you
prefer.]]
   )
}

obj_better_material_1 = {
name="built hardener",
title=_"Build a wood hardener",
body= obj_text(_ "Build a wood hardener", _
[[Bigger and better buildings - including all military ones - require better
building materials. They cannot be built of simple trunks - the wood has to be
refined to blackwood by a wood hardener first. Always remember to build a wood
hardener before you run out of blackwood, as without it you cannot expand.]]
   )
}

obj_better_material_2 = {
name="mission grindstone",
title=_"Build a lime kiln and coal economy",
body=obj_text(_"Build a lime kiln, a well and a charcoal burner or coal mine", _
[[Better buildings may also require other improved materials besides blackwood.
One of those is grout, which is produced out of raw stone, water and coal by a
lime-burner. You can obtain water by building a well upon a water source, which
your geologists can discover when you send them to any flag that is not on a
mountain. Coal can be obtained by building a charcoal burner or a coal mine.]]
   )
}

obj_better_material_3 = {
name="mission fernery",
title=_"Build a fernery",
body= obj_text(_"Build a fernery", _
[[The third material necessary for improved buildings is thatch reed, used to
cover roofs. Thatch reed is planted by a ferner around his building, the
fernery.]]
   )
}

obj_build_cattlefarm = {
   name = "mission cattlefarm", 
   title = _ "Build a cattlefarm",
   body = obj_text(_"Build a cattlefarm", _
[[When roads are under heavy load for a long time, one carrier is usually not
enough to transport goods swiftly. Traffic jams are the consequence. Such roads
therefore employ a second carrier: An ox that helps to carry the wares. This
doubles the transport capacity.<br>Oxen are bred in cattle farms
out of wheat and water.]]
   )
}


-- =======================================================================
--                     Texts for the Main Mission Thread                    
-- =======================================================================
briefing_msg_1 = {
   title =_ "The story continues",
   body = thron(_"Thron shakes his head...",
_[[ "The war goes on. More and more of our brothers and sisters flee the brutal war raging in the capital beneath the trees.<br> The stories they tell about the deeds of our kin are sad to hear. I've spent nights lying awake, restless, more tired I ever believed one could be. Yet whenever I close my eyes, I see the fortress my father built consumed by flames. The Throne Among the Trees, the symbol of unity and peace among our kin, became the wedge that drives us apart." ]]
   )
}

briefing_msg_2 = {
   title =_ "The story continues",
   body = thron(
_ [[ "Today my hunters brought men, women and little children who had hidden out in the forests, trying to escape the war, hate and revenge that rage among the tribes fighting each other like in old times, when we were no more but wild beasts driven and controlled by instincts. None of my brothers will ever gain and hold control over the wooden throne, none of the tribes will be strong enough to subdue the other. There will be no end to this slaughtering, unless... is this it? As father told me?<br> Rise against whoever threatens our very existence, may it even be one of your blood or mind?" ]]
   )
}


briefing_msg_3 = {
   title =_ "The story continues",
   body = thron(
_[[ "Boldreth seems more and more torn as the days go by. The spirits of my fellows sink as the cold season approaches, and we are still living in no more than huts and barracks. I never intended to stay out here in the wilderness for so long - but I never thought my brothers would engage in this senseless battle for so long either.<br> Perhaps it's time to make ourselves feel a little more at ease here. Perhaps it's time to give those who still live and think united a new home, replacing what is now lost to us? Until we can return to the place we once called our home..." ]]
   )
}


order_msg_1 = {
   title=_"Your loyal companion",
   body = boldreth(_"Brave Boldreth steps aside you...",
_[[ "So you want to prepare us for a longer stay in these forests, Thron? Well, I see the wisdom in doing so, yet my heart is yearning for a glorious return to Al'thunran. Too long have we waited while our brethren have fought on the battlefield, and our axes and swords have turned rusty over time." ]]
   )
}


order_msg_2 = {
   title =_ "The advisor",
   body = khantrukh(_"Khantrukh joins in ...",
_[[ "I beg your forgiveness, chieftain, but I have a grave concern to discuss with you. The rocks close to our camp are shrinking fast under our workers' tools, and it is a matter of little time only before they are all gone." ]]
   )
}

order_msg_3 = {
   title=_"Your loyal companion", 
   posx = 0,
   body = boldreth( _"Boldreth jumps up...", 
_[["While it is certainly no worthy task for a warrior, at least it is
something to be done! Thron, what do you say about me taking a few of our most
restless warriors and venturing somewhat closer to the great mountain of
Kal'mavrath - maybe we will find some more big stones to please the old man?"]]
      ) .. new_objectives(obj_claim_northeastern_rocks),
}

order_msg_4 = {
   title =_ "The advisor",
   posx = 0,
   body = khantrukh( _"Khantrukh grumbles...", _
   [[ "Old man? Disrespectful youth!" ]]
   )
}

order_msg_5_quarry = {
   title=_"The advisor",
   body= khantrukh( _"Khantrukh nods satisfied...", _
[[ "At last! These rocks should last for a while." ]]
   )
}

order_msg_6_geologist = {
   title=_"Your loyal companion",
   body=boldreth(_"Boldreth exclaims...", _
[["Just look at that! In the east is the great mountain of Kal'mavrath! I
wonder what treasures nature might have hidden beneath its majestic flanks!<br>
Let's expand to the east. There's a chance for the elderly to become useful
once in a while! Let us send out some of those who understand the stone's
tongue to unravel the mountain's secrets!"]]) .. new_objectives(obj_build_mines)
}

order_msg_7_mines_up = {
   title=_"Your loyal companion",
   body=boldreth(_"Boldreth laughs...", _
[[ "By Chat'Karuth's beard, this is awesome! Just imagine what we can use this coal and iron ore for!<br> It might even be enough to..." ]]
   )
}

order_msg_8_mines_up = {
   title=_"The advisor",
   body =khantrukh(_"Khantrukh interrupts...", _
[[ "While this is true, we should consider it later, for now other obstacles are at hand. The miners will definately demand greater rations for working high up on the mountain in this bitter cold, and providing these supplies may not be easy." ]]
   )
}


order_msg_9_hunter = {
   title=_"Your loyal companion",
   body=boldreth(_"Boldreth nods...", _
[["I have to hand it to you: you are right here, old man.<br> There seem to be
quite many animals in the forests here - we might just hunt down more of them.
And, of course, the people would cheer to a new tavern - hey, we might call it
'Thron's Pride' or so if you want!"]]) .. new_objectives(obj_basic_food)
}

order_msg_10_bread = {
   title=_"The advisor",
   body=khantrukh(_"The elder jumps into the air...", _
[["Wisdom commands to seek variety.<br> North of the great mountain is a large
plain - why don't we use the space Mother Nature gave us and build a farm? I
sure would enjoy a freshly baken pitta bread for a change..."]]) ..
   new_objectives(obj_begin_farming)
}

order_msg_11_basic_food_began = {
   title=_"Your loyal companion",
   body=boldreth(_"Boldreth cheers up...", _
[[ "Our hunters are out in the forests, Thron.<br> I promise you, before the sun sets today you will have a magnificient meal fit for the chieftain of all clans!" ]]
   )
}

order_msg_12_farming_began = {
   title=_"The advisor",
   body=khantrukh(_ "Khantrukh gazes over the plains...", 
_[[ "Isn't it lovely, the view of golden fields, growing peacefully, dancing to the wind? And I believe the baker is just on his way here, with his most delicious bread reserved for our chieftain alone!" ]]
   )
}

order_msg_13_tavern = {
   title=_"A beautiful morning",
   body=thron(_"Thron recognizes...", _
[[ "A tavern opened for our people yesterday. While I am hardly in the mood for celebration, I noticed how much this tiny bit of home meant to my people. Their songs filled the air until the deep night, and they were in higher spirit still the day after.<br> Maybe we actually are slowly creating a place here which we can... call home." ]]
   )
}

order_msg_14_refine_ore = {
   title=_ "Your loyal companion",
   body= boldreth(_"Boldreth remarks...", _
[["So now that we have both the mines and the food to sustain them, we should
bring what our brave miners gain for us into a more useful state. The iron ore
has to be melted into iron so that it is ready to be forged into whatever we
want. And we need the coal to keep the smelter's ovens burning."]]) ..
   new_objectives(obj_refine_ores)
}

order_msg_15_mines_exhausted = {
   title=_ "The advisor", 
   body = khantrukh(_"Khantrukh speaks...", _ 
[["Our miners are digging up less and less by day! We have to go deeper,
closer to the mountain's core, if we want more of its treasures!<br> Of course,
it is dark and cold in such depths and only few venture voluntarily into these
places. We should reward this bravery with greater rations for them. And a pint
of beer or two will keep their spirits high."]]) .. 
   new_objectives(obj_enhance_buildings)
}

order_msg_16_blackwood = {
   title=_"Your loyal companion",
   body= boldreth(_"Boldreth seems concerned...", _
[["As I just discovered, we are running short on blackwood! We cannot put our
warriors into some crumbling huts, and even less so in times as dangerous as
ours!<br> We need a wood hardener, and we need one now!"]]) .. 
   new_objectives(obj_better_material_1)
}

order_msg_17_grindstone = {
   title=_"Your loyal companion",
   body= boldreth(_"Boldreth smiles...", _
[["Well, old friend, this should ensure that our fortifications do not break
down with our foes' first battle cry! Now we can expand safely!<br> Still, it
would not hurt to assemble some grout for our further campaign; thus we could,
in times of need, build a fortress as the world has not seen before!"]]) .. 
   new_objectives(obj_better_material_2)
}

order_msg_18_fernery = {
   title=_"The advisor", 
   body= khantrukh(_"Khantrukh steps in...", _
[["Chieftain, this is a disgrace! It is well that we can produce grout for
mighty fortifications and great buildings now - only this does not prevent our
roofs from becoming leaky! Maybe the young ones like spending their nights in
the rain, but I just can't find any sleep with these raindrops dripping on my
face!<br> Now this is a problem we should do something about!"]]) .. 
   new_objectives(obj_better_material_3)
}

order_msg_19_all_material = {
   title=_"The advisor",
   body=khantrukh(_"Khantrukh seems relieved...", _
[[ "Wonderful! Our roofs are not dripping anymore, and we have all the material we need for, well, whichever monumental building we see fit!" ]]
   )
}

msg_mission_complete = {
   title=_"Mission Complete",
   body= thron( _
[["The other day Boldreth asked me to accompany him to the new inn. It would
cheer me up, he said.<br> When I looked around, I saw faithful faces, trusting
that I could guide them through these dark days. Yet before I could speak any
words of gratitude or encouragement, one of my warriors ran into the inn. He
had been far out in the forest for the past days and I could see how weary he
was.<br> The news he brought changed everything..."]]) .. rt(
      h1(_"Victory") .. p(_
[[You have completed this mission. You may continue playing if you wish,
otherwise move on to the next mission.]]
      )
   )
}

msg_cattlefarm_00 = {
   title = _ "We need oxen!",
   body = khantrukh(_ "Khantrukh speaks", _
[["As our realm gets bigger and bigger, the traffic on the roads gets
overwhelming. Our poor carriers are no longer able to transport the goods as
fast as we need them to. I suggest we give them some support by breeding
oxen.<br> They are amazing animals: The ox is as swift as a human being while
being much stronger and very frugal: all we need is wheat and water to breed
them and they will do their work on the roads loyally and reliably.]]) .. 
   new_objectives(obj_build_cattlefarm)
}

msg_story_1 = {
   title=_"One fullmoon night",
   body=thron(_"Thron speaks...", _
[[ "One night, when the moon shone brightly, I climbed to the peak of Kal'mavrath.<br> When I gazed at the horizon, I still saw crimson lights flicker in the distance. It is incredible how much passion my brethren fight this war with. I fear the moment I see Al'thunran again - will there be anything but ashes and wasted ruins left of our once beautiful capital when we get there?" ]]
   )
}

msg_story_2 = {
    title= _"Another cold day",
    body=thron(_"Thron looks worried...", _
[[ "The winter is upon us. Many of us are suffering from the cold, yet we must endure. The day we may return cannot be far anymore - it must not be far anymore.<br> I prayed it might get warmer again, as I prayed the war would finally come to an end.<br> So far, it seems that neither prayer was fulfilled." ]]
   )
}

msg_village = {
   title=_"A friendly village joined us!",
   body= thron(_"Thron speaks...",_
[[ "We have found a village with friendly and productive people, impressed by our wealth, technology and strength.<br> They have decided to join us!" ]]
   )
}

