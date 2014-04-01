-- =======================================================================
--                     TEXTS - No logic inside this file
-- =======================================================================

include "scripting/formatting.lua"

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
   title =_ "Capture the Ancient Castle",
   body = [[• ]] .. _
[[Defeat Lanissa and capture the ancient castle.]],
}

obj_defeat_erwyn = {
   name = "defeat_erwyn",
   title =_ "Defeat Erwyn",
   body = [[• ]] .. _
[[Defeat Erwyn. He commands the strongest opposing military force left on this island.]],
}
obj_defeat_jomo = {
   name = "defeat_jomo",
   title =_ "Defeat Jomo",
   body = [[• ]] .. _
[[Defeat Jomo. He will take care of the soldiers’ food supply.]]
   ,
}

-- =====================
-- Messages to the User
-- =====================
function _msg(title, text, ...)
   local s =
"<rt><p font-size=24 font-face=DejaVuSerif font-weight=bold font-color=3333FF>"
   .. title ..  "</p></rt><rt>" ..
   "<rt><p line-spacing=3 font-size=12>" .. reflow(text)
   objs = {...}
   if #objs > 0 then
      s = s .. "<br><br></p></rt>" .. rt(h2(_"New Objective")) .. "<rt><p line-spacing=3 font-size=12>"
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
   body = _msg( _"The Forbidden Island", _(
[[Finally! We have just taken our first step towards victory! ]] ..
[[Last night, we landed on the forbidden island and defeated the few guards that were watching the ]] ..
[[north-western part of the island. ]] ..
[[I am quite sure that none of the other warlords has detected us so far, ]] ..
[[so we should keep quiet and build up our infrastructure. Soon we will be strong enough to raid their positions.]])),
}

briefing_2_found_ancient_castle = {
   title =_ "Second Briefing",
   width = 400,
   height = 300,
   posy = 1,
   body = _msg(_"An Ancient Castle", _(
[[By the Gods! One of our scouts has discovered a mighty castle at the center of the old plateau. ]] ..
[[The castle must be quite old and seems to have been built in a foreign style. ]] ..
[[It’s quite obvious that this is not barbarian craft.]]) .. "<br><br>" ..
_([[Let’s hope that Lanissa – the warlord holding sway over that castle – has not discovered our movements yet. ]] ..
[[Perhaps we have a chance of conquering that mighty building without a bigger fight! ]] ..
[[However, it is essential that we capture it. It will be the key to our reign over this island!]]),
   obj_capture_ancient_castle),
}

briefing_3_captured_ancient_castle = {
   title = _ "Second Briefing",
   width = 400,
   height = 300,
   posy = 1,
   body = _msg(_"Ancient Castle Captured", _(
[[Wonderful! Our troops have finally defeated Lanissa and her soldiers. The ancient castle is ours!]]) .. "<br><br>" ..
_([[It is amazing how far one can see from the highest tower of the castle. We can watch the whole island. ]] ..
[[So now I wonder why Lanissa did not see us and thus did not prepare. ]] ..
[[Be that as it may, some things will never come to the light of day. ]] ..
[[The only important issue at the moment are the opposing troops still left. ]] ..
[[We have discovered enemy positions held by Erwyn to the north and east of the castle. ]] ..
[[He is known to be a strong warlord and surely commands the strongest warriors. ]] ..
[[In the south, we caught sight of some food infrastructures guarded by Jomo – a younger warlord – ]] ..
[[and we should take care of those infrastructures so we can take our cut of the food supply.]]), obj_defeat_erwyn, obj_defeat_jomo),
}

briefing_erwyn_defeated = {
   title=_ "Another Briefing",
   width = 400,
   height = 300,
   posy = 1,
   body = _msg( _"Erwyn Defeated", _
[[Great! Erwyn gave up when he saw his last buildings burning down. That’s one less strong warlord on this island!]]),
}

briefing_jomo_defeated = {
   title=_ "Another Briefing",
   width = 400,
   height = 300,
   posy = 1,
   body= _msg( _"Jomo Defeated", _
[[Great! Jomo gave up when he saw his last buildings burning down. That’s one less warlord on this island!]]),
}

last_briefing_victory = {
   title=_ "Last Briefing",
   width=400,
   height=300,
   posy=1,
   body = _msg( _"Victory!", _
[[Finally! The island is completely ours. Now we just have to defend it better than the warlords did.]] .. "<br><br>" ..
_"Congratulations! You have mastered this scenario. You may play on if you like!"
),
}
