-- ======================
-- Messages by Khantrukh 
-- ======================

-- TODO: gebabbel vom haeuptling
-- TODO: missionsziele
-- TODO: internationalization

function exist_buildings(region, t) 
   carr = {}
   for idx,f in ipairs(region) do
      if f.immovable then
         if carr[f.immovable.name] == nil then
            carr[f.immovable.name] = 1
         else
            carr[f.immovable.name] = carr[f.immovable.name] + 1
         end
      end
   end
   for house,count in pairs(t) do
      if carr[house] == nil or carr[house] < count then
         return false
      end
   end
   return true
end

function tutorial_thread()
text="<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8080FF>" ..
_"An old man says..." ..
"</p></rt><rt image=map:khantrukh.png><p line-spacing=3 font-size=12>" ..
_[[ "Hail, chieftain. I am Khantrukh and have seen many winters pass. Please allow me to aid you with my counsel through these darkened days." ]] ..
"</p></rt>" 
window_title=_"Somebody comes upon to you"
p:send_message(window_title, text, { popup=true })

text= "<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8080FF>" ..
_"Khantrukh continues..." ..
"</p></rt><rt image=map:khantrukh.png><p line-spacing=3 font-size=12>" ..
_[[ "None but the gods know how long we have to remain hidden here. The warriors hope we may march back gloriously any day now, but I strongly doubt that will happen soon. And the days are already getting shorter and colder...<br> We should prepare, in case we have to face the winter in these harsh lands. I believe to the east of our campside are a few places where we could raise a lumberjack's hut."<br><br>-- NEW OBJECTIVES --<br> ]] .. 
_"* Press SPACE to see where you can build.<br> The red spots show you where a small building - like a lumberjack's hut - may be built.<br>" ..
_"* Build a lumberjack's hut at the red spot directly right of your headquarters by clicking at it and selecting lumberjack's hut.<br>" ..
"</p></rt>" 
window_title=_"The advisor"
p:send_message(window_title, text, { popup=true })

text= "<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8080FF>" ..
_"The elder remarks..." ..
"</p></rt><rt image=map:khantrukh.png><p line-spacing=3 font-size=12>" ..
_[[ "Obviously, it would be too draining for our people to walk all the way to the lumberjack's hut through the wilderness. We will have to make a path between our hall and the lumberjack's hut. It's not a hard guess that the men would appreciate a way as short and even as possible - the wood surely will be heavy enough."<br><br>-- NEW OBJECTIVE --<br> ]] ..
_"* Build a road between your headquarters and the lumberjack's hut.<br> When you place the lumberjack's hut, a flag is created for it. You should connect it to another flag with a road.<br> If you click on a flag, little symbols will appear around it to show you in which directions you can build the road.<br> The colour of those symbols indicate how steep the road will be. Green is flat, yellow is steep and red is very steep. The steeper the step is, the harder it will be for your people to walk on it.<br> You can click on any of the symbols to build the first step of the road there. New symbols will appear to show you where the next step can go, and so on. However, you can also click further away to build several steps at once. Click on the flag at the headquarters to finish the road." ..
"</p></rt>"
window_title=_"The advisor"
p:send_message(window_title, text, { popup=true })

-- Wait till the hut is build.
while not exist_buildings(wl.map.Field(15,11):region(2), {lumberjacks_hut = 1}) do
   coroutine.yield(wl.game.get_time() + 5000)
end

text = "<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8080FF>" ..
_"Khantrukh remembers..." ..
"</p></rt><rt image=map:khantrukh.png><p line-spacing=3 font-size=12>" ..
_[[ "There is an old saying:<br> 'A burden divided is easier to endure.'<br> A crossroads on the way between our site and the lumberjack's hut would ease the work of our carriers."<br><br>-- NEW OBJECTIVE --<br> ]] ..
_"* Place a flag in the middle of the road.<br> There is a flag symbol in the mid of the way you just built. You place a flag there by clicking on the symbol and then clicking on the flag button in the consequently appearing menu. This splits the work of carrying the wood between two carriers.<br> When much wares are being transported between two points, additional flags in between make them get faster to their destination, thus improving your infrastructure.<br>" ..
"</p></rt>"
window_title = _ "The advisor"
p:send_message(window_title, text, { popup=true })

text= "<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8080FF>" ..
_"The old man looks to the sky..." ..
"</p></rt><rt image=map:khantrukh.png><p line-spacing=3 font-size=12>" ..
_[[ "The northern winds tell us that the coming winter will be a long and cruel one. One lumberjack's hut will not be enough to still the needs of our people. More trees await our axes south of our new crossroads - let us raise a second lumberjack's hut there!"<br><br>-- NEW OBJECTIVE --<br> ]] ..
_"* Place a lumberjack's hut directly south of the flag you just placed on the road. Connect the implied flag with the new lumberjack's hut afterwards.<br> Note that instead of building a road step by step, you may also directly click at the destination to build the entire road at once." ..
"</p></rt>"
window_title= _"The advisor"
p:send_message(window_title, text, { popup=true })

-- Wait till the hut is build.
while not exist_buildings(wl.map.Field(12,13):region(2), {lumberjacks_hut = 1}) do
   coroutine.yield(wl.game.get_time() + 5000)
end

text= "<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8080FF>" ..
_"Khantrukh nods..." ..
"</p></rt><rt image=map:khantrukh.png><p line-spacing=3 font-size=12>" ..
_[[ "Of course, if we intended to build a bigger settlement, much more lumberjack's huts would be advisable, but this should do for now.<br> But never forget - these forests are our legacy, entrusted to us by our ancestors. We must always respect and nourish them. What we take, we must give back again.<br> So, in order to replace the trees we chop down, we should build a ranger's hut, preferably close to the lumberjack's hut."<br><br>-- NEW OBJECTIVE --<br> ]] ..
_"* Build a ranger's hut to the east of the first lumberjack's hut." ..
"</p></rt>"
window_title= _"The advisor"
p:send_message(window_title, text, { popup=true })
p:allow_buildings{"ranger", "quarry"}


text= "<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=8080FF>" ..
_"The elder speaks..." ..
"</p></rt><rt image=map:khantrukh.png><p line-spacing=3 font-size=12>" ..
_[[ "I am well aware, chieftain, that neither you nor your warriors have the desire to stay in this forsaken place for long. But even so, our war to conquer back our home might take a long time and it would seem wise to make preparations.<br> I would advise that we obtain stones for the future - who knows what fortifications we might be forced to build very soon. And - like a gift of destiny - there are indeed great rocks just nearby..."<br><br>-- NEW OBJECTIVE --<br> ]] ..
_"* Build two quarries southwest of your headquarters." ..
"</p></rt>"
window_title= _"The advisor"
p:send_message(window_title, text, { popup=true })
while not exist_buildings(wl.map.Field(8,13):region(3), {quarry = 2}) do
   coroutine.yield(wl.game.get_time() + 5000)
end

text=_ "<rt image=map:khantrukh.png><p line-spacing=3 font-size=12>" ..
_[[ "Very well done, chieftain. We now have all we need to face the winter and may prepare ourselves for the battles ahead.<br><br>--------------------- VICTORY! ----------------------<br><br> You may continue if you wish. Otherwise, move on to the next mission." ]] ..
"</p></rt>"
window_title=_ "Mission Complete"
p:send_message(window_title, text, { popup=true })
-- TODO: reveal entry

end

wl.game.run_coroutine(coroutine.create(tutorial_thread))
