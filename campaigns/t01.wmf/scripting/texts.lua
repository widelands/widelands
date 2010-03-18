-- =======================================================================
--                            Mission Objectives                            
-- =======================================================================

function start_lumberjack_01(p) 
   return p:add_objective("lumberjack01",
   _ "Build a lumberjack's hut", 
"<rt><p>" ..
_"* Press SPACE to see where you can build.<br> The red house symbols show you where a small building - like a lumberjack's hut - may be built.<br>" ..
_"* Build a lumberjack's hut at the red house symbol just right of your headquarters by clicking on it and selecting a lumberjack's hut.<br>" ..
_"* Build a road between your headquarters and the construction site of the lumberjack's hut.<br> When you place the lumberjack's hut, a flag is created for it. You need to connect it to another flag by building a road between them.<br> If you click on a flag and select to build a road, little symbols will appear around it to show you in which directions you can build it.<br> The colour of these symbols indicats how steep this part of the road will be. Green is for flat, yellow means steep and red stands for very steep. The steeper the road is, the harder it will be for your people to walk on it (and your wares will be transported more slowly).<br> You can click on any of the symbols to build the first part of the road there. New symbols will appear to show you where the next part of the road can go, and so on. However, you can also click farer away from the flag to build several steps at once. Click on the flag at the headquarters to finish the road." ..
"</p></rt>")
end

function start_lumberjack_02(p)
   return p:add_objective("lumberjack02",
     _ "Build another lumberjack's hut",
     "<rt><p>" ..
_"* Place a flag in the middle of the road.<br> There is a flag symbol in the middle of the way you just built. You create a flag there by clicking on the symbol and then choosing the flag button in the menu that opens up. This divides the way the wood is transported into two parts with one carrier each.<br> When much wares are being transported between two points, additional flags in between make them arrive at their destination faster, thus improving your infrastructure.<br>" ..
_"* Place a lumberjack's hut just south of the flag you just placed on the road. Connect the flag you just raised with the flag of the new lumberjack's hut construction site afterwards.<br> Note that instead of building a road step by step, you may also click directly at its destination to build the entire road at once." ..
"</p></rt>")
end

function start_ranger(p)
   return p:add_objective("ranger", _ "Build a ranger's hut",
"<rt><p>" ..
_"* Build a ranger's hut to the east of the first lumberjack's hut." ..
"</p></rt>")
end

function start_quarries(p)
   return p:add_objective("quarries", _ "Build two quarries",
"<rt><p>" ..
_"* Build two quarries southwest of your headquarters." ..
"</p></rt>")
end

-- =======================================================================
--                              Kankruths Texts                             
-- =======================================================================

khantrukh_1="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8080FF>" ..
_"An old man says..." ..
"</p></rt><rt image=map:khantrukh.png><p line-spacing=3 font-size=12>" ..
_[[ "Hail, chieftain. I am Khantrukh and have seen many winters pass. Please allow me to aid you with my counsel through these darkened days." ]] ..
"</p></rt>" 

khantrukh_2= "<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8080FF>" ..
_"Khantrukh continues..." ..
"</p></rt><rt image=map:khantrukh.png><p line-spacing=3 font-size=12>" ..
_[[ "None but the gods know how long we have to remain hidden here. The warriors hope we may march back gloriously any day now, but I strongly doubt that will happen soon. And the days are already getting shorter and colder...<br> We should prepare, in case we have to face the winter in these harsh lands. I believe to the east of our campside are a few places where we could raise a lumberjack's hut."<br><br>-- NEW OBJECTIVES --<br> ]] .. 
_"* Press SPACE to see where you can build.<br> The red spots show you where a small building - like a lumberjack's hut - may be built.<br>" ..
_"* Build a lumberjack's hut at the red spot directly right of your headquarters by clicking at it and selecting lumberjack's hut.<br>" ..
"</p></rt>" 


khantrukh_3 = "<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8080FF>" ..
_"The elder remarks..." ..
"</p></rt><rt image=map:khantrukh.png><p line-spacing=3 font-size=12>" ..
_[[ "Obviously, it would be too draining for our people to walk all the way to the lumberjack's hut through the wilderness. We will have to make a path between our hall and the lumberjack's hut. It's not a hard guess that the men would appreciate a way as short and even as possible - the wood surely will be heavy enough."<br><br>-- NEW OBJECTIVE --<br> ]] ..
_"* Build a road between your headquarters and the lumberjack's hut.<br> When you place the lumberjack's hut, a flag is created for it. You should connect it to another flag with a road.<br> If you click on a flag, little symbols will appear around it to show you in which directions you can build the road.<br> The colour of those symbols indicate how steep the road will be. Green is flat, yellow is steep and red is very steep. The steeper the step is, the harder it will be for your people to walk on it.<br> You can click on any of the symbols to build the first step of the road there. New symbols will appear to show you where the next step can go, and so on. However, you can also click further away to build several steps at once. Click on the flag at the headquarters to finish the road." ..
"</p></rt>"

khantrukh_4 = "<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8080FF>" ..
_"Khantrukh remembers..." ..
"</p></rt><rt image=map:khantrukh.png><p line-spacing=3 font-size=12>" ..
_[[ "There is an old saying:<br> 'A burden divided is easier to endure.'<br> A crossroads on the way between our site and the lumberjack's hut would ease the work of our carriers."<br><br>-- NEW OBJECTIVE --<br> ]] ..
_"* Place a flag in the middle of the road.<br> There is a flag symbol in the mid of the way you just built. You place a flag there by clicking on the symbol and then clicking on the flag button in the consequently appearing menu. This splits the work of carrying the wood between two carriers.<br> When much wares are being transported between two points, additional flags in between make them get faster to their destination, thus improving your infrastructure.<br>" ..
"</p></rt>"

khantrukh_5 = "<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8080FF>" ..
_"The old man looks to the sky..." ..
"</p></rt><rt image=map:khantrukh.png><p line-spacing=3 font-size=12>" ..
_[[ "The northern winds tell us that the coming winter will be a long and cruel one. One lumberjack's hut will not be enough to still the needs of our people. More trees await our axes south of our new crossroads - let us raise a second lumberjack's hut there!"<br><br>-- NEW OBJECTIVE --<br> ]] ..
_"* Place a lumberjack's hut directly south of the flag you just placed on the road. Connect the implied flag with the new lumberjack's hut afterwards.<br> Note that instead of building a road step by step, you may also directly click at the destination to build the entire road at once." ..
"</p></rt>"

khantrukh_6 = "<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8080FF>" ..
_"Khantrukh nods..." ..
"</p></rt><rt image=map:khantrukh.png><p line-spacing=3 font-size=12>" ..
_[[ "Of course, if we intended to build a bigger settlement, much more lumberjack's huts would be advisable, but this should do for now.<br> But never forget - these forests are our legacy, entrusted to us by our ancestors. We must always respect and nourish them. What we take, we must give back again.<br> So, in order to replace the trees we chop down, we should build a ranger's hut, preferably close to the lumberjack's hut."<br><br>-- NEW OBJECTIVE --<br> ]] ..
_"* Build a ranger's hut to the east of the first lumberjack's hut." ..
"</p></rt>"


khantrukh_7 = "<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8080FF>" ..
_"The elder speaks..." ..
"</p></rt><rt image=map:khantrukh.png><p line-spacing=3 font-size=12>" ..
_[[ "I am well aware, chieftain, that neither you nor your warriors have the desire to stay in this forsaken place for long. But even so, our war to conquer back our home might take a long time and it would seem wise to make preparations.<br> I would advise that we obtain stones for the future - who knows what fortifications we might be forced to build very soon. And - like a gift of destiny - there are indeed great rocks just nearby..."<br><br>-- NEW OBJECTIVE --<br> ]] ..
_"* Build two quarries southwest of your headquarters." ..
"</p></rt>"

khantrukh_8 =_ "<rt image=map:khantrukh.png><p line-spacing=3 font-size=12>" ..
_[[ "Very well done, chieftain. We now have all we need to face the winter and may prepare ourselves for the battles ahead.<br><br>--------------------- VICTORY! ----------------------<br><br> You may continue if you wish. Otherwise, move on to the next mission." ]] ..
"</p></rt>"


-- =======================================================================
--                  General story texts from the Chieftain                  
-- =======================================================================
story_msg1 = {
   title = _"In the night",
   msg ="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=2F9131>" ..
_"Thron speaks..." ..
"</p></rt><rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
_ [[ "In another sleepless night, I went up to the hill and gazed to the north. The fires are still burning, stilling their hunger upon my father's legacy.<br> They are a constant reminder why we had to hide here... and why we must return in the end!" ]] ..
"</p></rt>", 
loc = al_thunran,
}

story_msg2= {
   title = _"At the grave",
   msg = "<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=2F9131>" .. 
_"Thron speaks..." ..
"</p></rt><rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
_[[ "Once again, I went up to my father's tomb, in a sacred grove at the foot of the great spire of Kal'mavrath. I just stood there and felt neither the hours pass nor the cold rain pouring down from a darkened sky...<br> Somehow it felt like a farewell. For the first time, I wondered what the future might hold for me..." ]] ..
"</p></rt>", 
loc = grave,
}

story_msg3 = {
  title=_ "The other day",
  msg = "<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=2F9131>" ..
_"Thron says thoughtfully..." ..
"</p></rt><rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
_[[ "Some time ago, Boldreth came to me. His advice was to move to a place closer to home - to strike at the first sign of my brother's forces wavering.<br> But I look over the forests and I still see black smoke rising to the sky. I know - it is too early yet, and what he hopes for will not happen any time soon. He might still be right, but I fear the bloodshed that too rash a return would cause both on our and the other side..." ]] ..
"</p></rt>"
}

story_msg4 = {
title =_ "The oath",
msg = "<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=2F9131>" ..
_"Thron looks furious..." ..
"</p></rt><rt image=map:chieftain.png><p line-spacing=3 font-size=12>" ..
_[[ "Today, my warriors picked up an old man, wandering sick and wounded through the dark forest. We listened in horror when he told us of the atrocities taking place in Al'thunran.<br> I hereby renew my oath - I will stop this madness at any cost!" ]] ..
"</p></rt>"
}

