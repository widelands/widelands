-- =======================================================================
--                     TEXTS - No logic inside this file
-- =======================================================================

-- Removes whitespace at the beginning or the end and translates
-- newlines into spaces.
function reflow(s)
   s = s:gsub("\r", ""):gsub("\n", " ")
   return s:match'^%s*(.*%S)' or ''
end

-- ===========
-- Objectives
-- ===========
function _obj(t)
   return
      "<rt><p line-spacing=3 font-size=12>" .. reflow(t) .. "</p></rt>"
end
function add_obj(o)
   return p1:add_objective(o.name, o.title, _obj(reflow(o.body)), o)
end

obj_capture_ancient_castle = {
   name = "capture_ancient_castle",
   title =_ "Capture the ancient castle",
   body = _"* Defeat Lanissa and capture the ancient castle.",
}

obj_defeat_erwyn = {
   name = "defeat_erwyn",
   title =_ "Defeat Erwyn",
   body = _[[
* Defeat Erwyn. He commands the strongest opposing soldiers left on this
island.
]],
}
obj_defeat_jomo = {
   name = "defeat_jomo",
   title =_ "Defeat Jomo",
   body = _
      "* Defeat Jomo. He takes care about the soldiers food supply."
   ,
}

-- =====================
-- Messages to the User
-- =====================
function _msg(title, text, ...)
   local s = 
"<rt><p font-size=24 font-face=FreeSerif font-weight=bold font-color=3333FF>"
   .. title ..  "</p></rt><rt>" .. 
   "<rt><p line-spacing=3 font-size=12>" .. reflow(text)
   objs = {...}
   if #objs > 0 then
      s = s ..  "<br><br>" ..  _"-- NEW OBJECTIVES --" .. "<br>"
      local i = 1
      while i < #objs do
         s = s .. reflow(objs[i].body) .. "<br>"
         i = i+1
      end
      s = s .. reflow(objs[#objs].body) 
   end
   return s .. "</p></rt>"
end
function send_msg(t)
   p1:message_box(t.title, t.body, t)
end

briefing_1_the_forbidden_island = {
   title =_ "First Briefing",
   width = 400, 
   height = 300, 
   posy = 1,
   body = _msg( _"The forbidden island", _[[
Finally! Our first step to victory was just done! Last night, we landed on
the forbidden island and defeated the few guards that sentineled the north
western part of the island. I am quite sure, that none of the other warlords
has detected us so far, so we should keep quiet and build up our
infrastructure. Soon we will be strong enough to raid their positions.
]]),
}

briefing_2_found_ancient_castle = { 
   title =_ "Second Briefing", 
   width = 400, 
   height = 300,
   posy = 1,
   body = _msg(_"An ancient castle", _[[
By the Gods! One of our scouts found a mighty castle at the middle of the old
plateau. The castle must be quite old and seems to be build in a foreign
style. It's quite obvious, that this is not barbarian craft.<br>Let's hope
Lanissa - the warlord holding command over that castle - still has not observed
us. Perhaps we have a chance to conquer that mighty building without a bigger
fight!<br>However we really have to capture it. It will be our key for our
reign over this island!
]], obj_capture_ancient_castle),
}

briefing_3_captured_ancient_castle = {
   title = _ "Second Briefing",
   width = 400,
   height = 300,
   posy = 1,
   body = _msg(_"Ancient castle captured", _[[
Wonderful! Our troops finally defeated Lanissa and her soldiers. The ancient
castle is ours!<br> It is amazing, how far one can see from the highest tower
of the castle. We can watch the whole island. So now I wonder, why Lanissa has
not see us and thus did not prepare. But well, some things will never come to
the light of day. The only important things at the moment are the left opposing
troops. We observed enemy's positions from Erwyn north and eastwards of the
castle. He is known to be a strong warlord and surely commands the strongest
warriors. In the south we captured some guarded food infrastructures of Jomo -
a younger warlord - we should take care about that infrastructures and cut of
the food supply.
]], obj_defeat_erwyn, obj_defeat_jomo),
}

briefing_erwyn_defeated = {
   title=_ "Another Briefing", 
   width = 400,
   height = 300,
   posy = 1,
   body = _msg( _"Erwyn defeated", _[[ 
Great! Erwyn gave up, when he saw his last buildings burning down. One strong
warlord less on this island!
]]), 
}

briefing_jomo_defeated = {
   title=_ "Another Briefing", 
   width = 400, 
   height = 300, 
   posy = 1, 
   body= _msg( _"Jomo defeated", _[[
Great! Jomo gave up, when he saw his last buildings burning down. One warlord
less on this island!
]]), 
}

last_briefing_victory = {
   title=_ "Last Briefing",
   width=400,
   height=300,
   posy=1,
   body = _msg( _"Victory!", _[[
Finally! The complete island is ours. Now we just have to defend it better than
the warlords did.
]] .. "<br><br>" .. _[[
Congratulations! You mastered this scenario. You may play on if you like!
]]),
}

