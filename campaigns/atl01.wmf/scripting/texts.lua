-- =======================================================================
--                 Texts for the Atlantean tutorial mission
-- =======================================================================

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

function princess(title, text)
   return speech("map:princess.png", "2F9131", title, text)
end

-- Nice formatting for objective texts: A header and one paragraph
-- of text
function obj_text(heading, body)
   return rt(h2(heading) .. p(body))
end

-- Append an objective text to a dialog box in a nice fashion.
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
--                                Objectives
-- =======================================================================
obj_ensure_build_wares_production = {
   name = _ "obj_build_wares",
   title = _ "Construct buildings to ensure build wares supply",
   body = obj_text(_"Ensure build wares supply", _
[[Build a quarry, two woodcutters house, a  ]]
   ),
}

obj_build_environment = {
   name = "build_environment",
   title = _ "Build 3 wood cutters, 3 foresters, sawmill and quarry",
   body = obj_text(_"Build environment", _
[[Build 3 wood cutters, 3 foresters, sawmill and quarry]]
   ),
}

-- =======================================================================
--                                  Texts
-- =======================================================================

-- This is an array or message descriptions
initial_messages = {
{
   title = _ "Proudest to the death",
   body = rt(
      h1(_"This is a caption. And it is translated") ..
      p(_
[[On the hidden and lost island of Atlantis, a proud tribe settled since the
 world was very young. Ruled by the bloodline of King Ajanthul - the first human
 to be empowered by the sea god Lutas to breath above the sea level - and the
 wise clerics keeping the connection to Lutas they prospered and became
 civilized.]]
      ) .. p(
[[This story happens during the regency of King Askandor, the 43th successor of
 King Ajanthul. He has been a good king, ruling Atlantis with wisdom and
 foresight. But with age, he became afraid of dying and so he began looking for
 a cure to death even though most clerics warned him. Some said, endless life
 was only for the gods and the seek for it was forbidden.]]
      )
   ),
},
{
   title = _ "The god's disgrace",
   body = rt(
      h1("The god's punishment") ..
      p(_
[[But all seemed well, and only the horses seemed to feel something.
In the nights, the went crazy and were full of fear. Not before long, the
horsebreeder Xydra figured out what was wrong with them: The sea level before
their stable was rising in an ever accelerating speed.]]
   ) .. p(_
[[The clerics went into their meditation and the reason for the rising water
 was soon to be found: The god lost faith in the atlanteans because of the
 boldness of their king. He decided to withdraw the rights that were
 granted to King Ajanthul and his children. And so, he calls them now back
 below the sea.]]
   )
)
},
{
   title = _ "Uproar and confusion",
   body = rt(
      h1("Chaos emerges...") ..
      p(_
[[Guilt ridden, the king suicided. Without monarch, the people turned to the
 clerics. But those had no substantial help to offer: most accepted their
 fate, others tried to change the god's mind by offering animals in his
 temple. But to no avail...]]
   ) .. p(_
[[Jundlina, the late king's daughter and the highest priestess of the god 
 was the most determined cleric. As countless offerings didn't change the
 situation, she convinced herself, that the god needed an offer of big 
 personal value to be soothed. So she offered him her most beloved: 
 her husband, father of her only child.]]
   ) .. p(_
[[But not even this changed the mind of the god. The water kept on rising.
 Nearly driven crazy by guilt, pain and anger, Jundlina became a heretic:
 Secretly, she gathered people of the common folk who were not in line with the
 decision of the clerics to accept the god's will. Together with them, she set
 the temple on fire and stole a ship to flee from the god's influence of
 Atlantis. This small group started praying to Satul, the fire god and the worst
 enemy of Lutas.]]
   ) .. p(_
[[Leaving the dying Atlantis and their past behind, they started a quest to
 find a place sheltered by the fire and protected from the sea.]]
   )
)
},
} -- end of initial messages.

first_briefing_messages = {
{
   title = _ "The princess orders",
   body = princess(_ "The princess speaks", _
[[Build environment first]]
   ) .. new_objectives(obj_build_environment)
}
}


-- =======================================================================
--                         Leftover buildings found
-- =======================================================================
first_leftover_building_found = {
{
   title = _ "Strange buildings",
   body = princess(_ "I say",
[[Maybe we are not alone here..]]
   )
}
}

second_leftover_building_found = {
{
   title = _ "Strange buildings",
   body = princess(_ "I say",
[[Yet another of those]]
   )
}
}

third_leftover_building_found = {
{
   title = _ "Strange buildings",
   body = princess(_ "I say",
[[And Yet another of those. But i am confident that we are alone on
this island as we have already conquered and seen so much of it.]]
   )
}
}



