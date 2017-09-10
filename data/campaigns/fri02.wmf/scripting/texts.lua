-- =========================
-- Some formating functions
-- =========================

include "scripting/formatting.lua"
include "scripting/format_scenario.lua"

function reebaud(title, text)
   return speech("map:reebaud.png", "55BB55", title, text)
end
function hauke(title, text)
   return speech("map:hauke.png", "7799BB", title, text)
end
function murilius(title, text)
   return speech("map:murilius.png", "888811", title, text)
end
function luwcihar(title, text)
   return speech("map:luwcihar.png", "990000", title, text)
end

-- =======================================================================
--                           Texts - No logic here
-- =======================================================================

-- ===========
-- objectives
-- ===========

obj_new_home = {
   name = "new_home",
   title=_"Build a basic economy",
   number = 1,
   body = objective_text(_"Take care of supplies of all basic materials",
      listitem_bullet(_[[Build a quarry, a reed farm, a clay pit, a well, and houses for a brick burner, a woodcutter and a forester.]])
   ),
}
obj_expand_south = {
   name = "expand_south",
   title=_"Expand to the South",
   number = 1,
   body = objective_text(_"Expand your territory to explore the South",
      listitem_bullet(_[[The North looks not very useful to us, but who knows what we might find in the South?]])
   ),
}
obj_defeat_barbarians = {
   name = "defeat_barbarians",
   title=_"Defeat Luw´Ci-Har!",
   number = 1,
   body = objective_text(_"Defeat the barbarians",
      listitem_bullet(_[[Defeat Luw´Ci-Har and his hostile tribe!]])
   ),
}

-- ==================
-- Texts to the user
-- ==================

intro_1 = {
   title =_ "Welcome back!",
   body=reebaud(_"A new home",
      -- TRANSLATORS: Reebaud – Introduction 1
      _([[I began to believe the journey would never end. I have no idea how long we were on sea, tossed about by envious storms… it must have been months.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – Introduction 1
      _([[But we are here. Finally, we arrived at the mouth of a calmer fjord. We sailed upriver, and arrived here.]])),
}
intro_2 = {
   title =_ "Welcome back!",
   body=reebaud(_"A new home",
      -- TRANSLATORS: Reebaud – Introduction 2
      _([[This place looks bleak and barren, my companions say, but I think it looks… beautiful.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – Introduction 2
      _([[We will settle here. My scouts report no sign of living creatures anywhere nearby. We will start building our new home here.]]))
      .. new_objectives(obj_new_home),
}
expand_s_1 = {
   title =_ "Go South",
   body=reebaud(_"Expand and conquer",
      -- TRANSLATORS: Reebaud – Expand South 1
      _([[We have built a home in this wondeful landscape. But as our tribe grows, we need more space. We should start expanding our territory.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – Expand South 1
      _([[The scouts reported that the lands North of us are rugged hills, beautiful to behold but not suited for buildings. Let´s find out what we can discover in the South.]]))
      .. new_objectives(obj_expand_south),
}

