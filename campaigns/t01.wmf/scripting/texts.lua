use("aux", "formatting")
use("aux", "format_scenario")

-- Append an objective text with a header to a dialog box in a nice fashion.
function new_objectives(...)
   local s = ""
   for idx,obj in ipairs{...} do
	s = objective_header(ngettext("New Objective", "New Objectives", obj.number) , obj.body)
   end
   return s
end

function thron(title, text)
   return speech("map:chieftain.png", "2F9131", title, text)
end
function khantrukh(title, text)
   return speech("map:khantrukh.png", "8080FF", title, text)
end

-- =======================================================================
--                            Mission objectives                            
-- =======================================================================

start_lumberjack_01 = {
   name = "start_lumberjack_01",
   title = _ "Lumberjack's hut",
   number = 2,
   body = objective_text(_"Build a lumberjack's hut", _
"* Press SPACE to see where you can build.<br> The red house symbols show you where a small building - like a lumberjack's hut - may be built." .. "<br>" ..
_"* Build a lumberjack's hut at the red house symbol just right of your headquarters by clicking on it and selecting a lumberjack's hut." .. "<br>" ..
_"* Build a road between your headquarters and the construction site of the lumberjack's hut.<br> When you place the lumberjack's hut, a flag is created for it. You need to connect it to another flag by building a road between them.<br> If you click on a flag and select to build a road, little symbols will appear around it to show you in which directions you can build it.<br> The colour of these symbols indicates how steep this part of the road will be. Green is for flat, yellow means steep and red stands for very steep. The steeper the road is, the harder it will be for your people to walk on it (and your wares will be transported more slowly).<br> You can click on any of the symbols to build the first part of the road there. New symbols will appear to show you where the next part of the road can go, and so on. However, you can also click farther away from the flag to build several steps at once. Click on the flag at the headquarters to finish the road."
   ),
}

start_lumberjack_01_01 = {
   name = "start_lumberjack_01_01",
   title = _ "Lumberjack's hut",
   number = 1,
   body = objective_text(_"Build a lumberjack's hut", _
"* Press SPACE to see where you can build.<br> The red house symbols show you where a small building - like a lumberjack's hut - may be built." .. "<br>" ..
_"* Build a lumberjack's hut at the red house symbol just right of your headquarters by clicking on it and selecting a lumberjack's hut."
   ),
}

start_lumberjack_01_02 = {
   name = "start_lumberjack_01_02",
   title = _ "Lumberjack's hut",
   number = 1,
   body = objective_text(_"Build a road to the lumberjack's hut", _
"* Build a road between your headquarters and the construction site of the lumberjack's hut.<br> When you place the lumberjack's hut, a flag is created for it. You need to connect it to another flag by building a road between them.<br> If you click on a flag and select to build a road, little symbols will appear around it to show you in which directions you can build it.<br> The colour of these symbols indicates how steep this part of the road will be. Green is for flat, yellow means steep and red stands for very steep. The steeper the road is, the harder it will be for your people to walk on it (and your wares will be transported more slowly).<br> You can click on any of the symbols to build the first part of the road there. New symbols will appear to show you where the next part of the road can go, and so on. However, you can also click farther away from the flag to build several steps at once. Click on the flag at the headquarters to finish the road."
   ),
}

start_lumberjack_02 = {
   name = "start_lumberjack_02",
   title = _ "Lumberjack's hut",
   number = 2,
   body = objective_text(_"Build another lumberjack's hut", _
"* Place a flag in the middle of the road.<br> There is a flag symbol in the middle of the way you just built. You create a flag there by clicking on the symbol and then choosing the flag button in the menu that opens up. This divides the way the wood is transported into two parts with one carrier each.<br> When many wares are being transported between two points, additional flags in between make them arrive at their destination faster, thus improving your infrastructure." .. "<br><br>" ..
_"* Place a lumberjack's hut just south of the flag you just placed on the road. Connect the flag you just raised with the flag of the new lumberjack's hut construction site afterwards.<br> Note that instead of building a road step by step, you may also click directly at its destination to build the entire road at once."
   ),
}

start_lumberjack_02_01 = {
   name = "start_lumberjack_02_01",
   title = _ "Lumberjack's hut",
   number = 1,
   body = objective_text(_"Build another lumberjack's hut", _
"* Place a flag in the middle of the road.<br> There is a flag symbol in the middle of the way you just built. You create a flag there by clicking on the symbol and then choosing the flag button in the menu that opens up. This divides the way the wood is transported into two parts with one carrier each.<br> When many wares are being transported between two points, additional flags in between make them arrive at their destination faster, thus improving your infrastructure."
   ),
}

start_lumberjack_02_02 = {
   name = "start_lumberjack_02_02",
   title = _ "Lumberjack's hut",
   number = 1,
   body = objective_text(_"Build another lumberjack's hut", _
"* Place a lumberjack's hut just south of the flag you just placed on the road. Connect the flag you just raised with the flag of the new lumberjack's hut construction site afterwards.<br> Note that instead of building a road step by step, you may also click directly at its destination to build the entire road at once."
   ),
}

start_ranger = {
   name = "start_ranger",
   title = _ "Ranger's hut",
   number = 1,
   body = objective_text(_"Build a ranger's hut", _
"* Build a ranger's hut to the east of the first lumberjack's hut."
   ),
}

start_quarries = {
   name = "start_quarries",
   title = _ "Quarries",
   number = 1,
   body = objective_text(_"Build two quarries", _
"* Build two quarries southwest of your headquarters."
   ),
}

-- =======================================================================
--                              Kankruth's texts                             
-- =======================================================================

khantrukh_1= khantrukh(_"An old man says...",
_[[Hail, chieftain. I am Khantrukh and have seen many winters pass. Please allow me to aid you with my counsel through these darkened days.]])

khantrukh_2= khantrukh(_"Khantrukh continues...",
_[[None but the gods know how long we have to remain hidden here. The warriors hope we may march back gloriously any day now, but I strongly doubt that will happen soon. And the days are already getting shorter and colder...<br> We should prepare, in case we have to face the winter in these harsh lands. I believe to the east of our campsite are a few places where we could raise a lumberjack's hut.]])
.. new_objectives(start_lumberjack_01_01)

khantrukh_3 = khantrukh(_"The elder remarks...",
_[[Obviously, it would be too exhausting for our people to walk all the way to the lumberjack's hut through the wilderness. We will have to build a road between our hall and the lumberjack's hut. It's not a hard guess that the men would appreciate a way as short and plain as possible - the wood will surely be heavy enough, nevertheless.]])
.. new_objectives(start_lumberjack_01_02)

khantrukh_4 = khantrukh(_"Khantrukh remembers...",
_[[There is an old saying:<br> 'A burden divided is easier to endure.'<br> A crossroads on the way between our headquarters and the lumberjack's hut would ease the work of our carriers.]])
.. new_objectives(start_lumberjack_02_01)

khantrukh_5 = khantrukh(_"The old man looks to the sky...",
_[[The northern winds tell us that the coming winter will be a long and cruel one. One lumberjack's hut will not be enough to fit the needs of our people. More trees wait for our axes just south of our new crossroads - let us raise a second lumberjack's hut there!]])
.. new_objectives(start_lumberjack_02_02)

khantrukh_6 = khantrukh(_"Khantrukh nods...",
_[[Of course, if we intended to build a bigger settlement, many more lumberjack's huts would be advisable, but this should suffice for now.<br> But never forget - these forests are our heritage, entrusted upon us by our ancestors. We must always respect and care for them. What we take, we must give back again.<br> So, in order to replace the trees we chop down, we should build a ranger's hut, preferably close to the lumberjack's hut.]])
.. new_objectives(start_ranger)

khantrukh_7 = khantrukh(_"The elder speaks...",
_[[I am well aware, chieftain, that neither you nor your warriors have the desire to stay in this forsaken place for long. But even so, our war to reconquer our home might take a long time and it would seem wise to make preparations.<br> I would advise that we obtain stones for the future - who knows what fortifications we might be forced to build very soon. And - like a gift of destiny - there are indeed great rocks just nearby...]])
.. new_objectives(start_quarries)

khantrukh_8 = khantrukh(_[[Very well done, chieftain. We now have all we need to face the winter and may prepare ourselves for the battles ahead.]])
 .. rt("<p font-size=10> <br></p>" ..
      h1(_"Victory") ..
      p(_[[You may continue to play this map if you wish. Otherwise, move on to the next mission.]])
   )

-- =======================================================================
--                  General story texts by the chieftain                  
-- =======================================================================
story_msg1 = {
   title = _"In the night",
   msg = thron(_"Thron speaks...",
_[[In another sleepless night, I went up to the hill and gazed to the north. The fires are still burning, satisfying their hunger upon my father's legacy.<br> They are a constant reminder why we have to hide here... and why we must return in the end!]]), 
field = al_thunran,
}

story_msg2= {
   title = _"At the tomb",
   msg = thron(_"Thron speaks...",
_[[Once again, I went up to my father's tomb, in a sacred grove at the foot of the great spire of Kal'mavrath. I just stood there and felt neither the hours pass nor the cold rain pouring down from a darkened sky...<br> Somehow it felt like a farewell. For the first time, I wondered what the future might hold for me...]]), 
field = grave,
}

story_msg3 = {
  title=_ "The other day",
  msg = thron(_"Thron says thoughtfully...",
_[[Some time ago, Boldreth came to me. His advice was to move to a place closer to home - to strike at the first sign of my brothers' forces wavering.<br> But I look over the forests and I can still see black smoke rising to the sky. I know - it is too early yet, and what he hopes for will not happen any time soon. He might still be right, but I fear the bloodshed that returning too fast would cause both on our and the other side...]])
}

story_msg4 = {
title =_ "The oath",
msg = thron(_"Thron looks furious...",
_[[Today, my warriors picked up an old man, wandering sick and wounded through the dark forest. We listened in horror when he told us of the atrocities taking place in Al'thunran.<br> I hereby renew my oath - I will stop this madness at any cost!]])
}

