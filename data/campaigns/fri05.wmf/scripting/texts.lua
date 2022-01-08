-- =========================
-- Some formating functions
-- =========================

include "scripting/richtext_scenarios.lua"

-- TODO(Nordfriese): Need new images for them all
function claus(title, text)
   return speech ("map:claus.png", "1CC200", title, text)
end
function henneke(title, text)
   return speech ("map:henneke.png", "F7FF00", title, text)
end
function reebaud(title, text)
   return speech ("map:reebaud.png", "55BB55", title, text)
end
function ketelsen(title, text)
   return speech ("map:ketelsen.png", "0011AA", title, text)
end
function dane(title, text)
   return speech ("map:dane.png", "AA1100", title, text)
end

-- =======================================================================
--                           Texts - No logic here
-- =======================================================================

-- ===========
-- objectives
-- ===========

obj_north = {
   name = "nocom",
   title=_"NOCOM",
   number = 1,
   body = objective_text(_"NOCOM",
      li(_[[NOCOM.]])
   ),
}

-- ==================
-- Texts to the user
-- ==================

intro_1 = {
   title = _"Introduction",
   body=claus(_"Welcome back!",
      -- TRANSLATORS: Claus Lembeck – Introduction 1
      _([[NOCOM.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Claus Lembeck – Introduction 1
      _([[NOCOM.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Claus Lembeck – Introduction 1
      _([[NOCOM.]])),
}

intro_1 = {
   title = _"Introduction",
   body=claus(_"Welcome back!",
      -- TRANSLATORS: Claus Lembeck – Introduction 1
      _([[NOCOM.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Claus Lembeck – Introduction 1
      _([[NOCOM.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Claus Lembeck – Introduction 1
      _([[NOCOM.]])),
}

intro_1 = {
   title = _"Introduction",
   body=claus(_"Welcome back!",
      -- TRANSLATORS: Claus Lembeck – Introduction 1
      _([[NOCOM.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Claus Lembeck – Introduction 1
      _([[NOCOM.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Claus Lembeck – Introduction 1
      _([[NOCOM.]])),
}

intro_1 = {
   title = _"Introduction",
   body=claus(_"Welcome back!",
      -- TRANSLATORS: Claus Lembeck – Introduction 1
      _([[NOCOM.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Claus Lembeck – Introduction 1
      _([[NOCOM.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Claus Lembeck – Introduction 1
      _([[NOCOM.]])),
}

victory = {
   title = _"Victory",
   body=claus(_"Victory!",
      -- TRANSLATORS: Claus Lembeck – victory
      _([[NOCOM!]]))
      .. objective_text(_"Congratulations",
      _([[You have completed this mission. You may continue playing if you wish, otherwise move on to the next mission.]])),
   allow_next_scenario = true,
}
