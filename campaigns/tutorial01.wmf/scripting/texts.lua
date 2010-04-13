-- =======================================================================
--                      Texts for the tutorial mission
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

-- Simple flowing text. One Paragraph
function p(s)
   return "<p line-spacing=3 font-size=12>" .. s .. "<br></p>" ..
      "<p font-size=8> <br></p>"
end

-- =============
-- Textes below
-- =============
initial_message_01 = {
   title = _ "Welcome to the Widelands tutorial!",
   body = rt(
      h1(_"Welcome to Widelands!") ..
      p(_
[[Widelands is a slow paced build-up strategy with an emphasis on construction,
 not destruction. This tutorial will get you through the basics of the game.]]
      ) .. p(_
[[You can dismiss this box by left-clicking on the button below.]]
      )
   ),
}
initial_message_02 = {
   title = _ "Diving in",
   pos = "topright",
   field = plr.starting_field,
   body = rt(
      h1(_"Let's dive right in!") ..
      p(_
[[There are three different tribes in widelands: the barbarians, the empire and 
 the atlanteans. All tribes have a different economy, strength and weaknesses,
 but the general gameplay is the same for all. We play the barbarians for now.]]
      ) .. p(_
[[You usually start the game with one headquarter. This is the big building with
 the blue flag in front. The headquarters is a ware house that stores wares,
 workers and soldiers. Some wares are needed for building houses, others for
 making other wares. Obviously, the wares in the headquarters will not last 
 forever, so you must make sure to reproduce them. The most important wares in
 the early game are the basic build wares: trunks and raw stone. Let's make sure
 that we do not run out of trunks. For this, we need to build a lumberjack.]] 
      ) .. p(_
[[We need to find a nice place for the lumberjack. To make this easier, we
 activate the build help. To do this, either click on the build help button
 on the bottom of the screen which is the fourth one from the left. Or you
 could use the SPACE key to toggle it on.]]
      )
   ),
}

lumberjack_message_01 = {
   title = _ "Lumberjack spot", 
   pos = "topright", 
   field = first_lumberjack_field, 
   body = rt(p(_
[[There you go. I will explain about all those symbols in a minute. First, let
 me show you how to make a lumberjack's hut and how to connect it. There is a
 sweet sport for a lumberjack right next to those trees.]]
      )
   )
}

lumberjack_message_02 = {
   title = _ "Building the lumberjack",
   pos = "topright",
   field = first_lumberjack_field, 
   body = rt(p(_
[[First, I'll left-click on the symbol were I want the lumberjack to be built. A
 window will appear were I can choose between buildings - you won't see this
 window when I do it, because I am lightning fast. I left click on the
 lumberjack. This places a construction site at the given location.]]
      )
   )
}

lumberjack_message_03 = {
   title = _ "Building a connecting road", 
   pos = "topright", 
   body = rt(p(_
[[That won't do yet. I still need to connect the lumberjack's hut to the 
 rest of my road network. After I ordered a construction site, I am
 automatically in road building mode, so all I have to do is click on the flag
 in front of my headquarters]]
       )
   )
}

lumberjack_message_04 = {
   title = _ "Waiting for the lumberjack to go up", 
   pos = "topright", 
   body = rt(p(_
[[Now watch closely as a builder leaves the headquarters and goes to the
 construction site. Also a carrier will take position in between the two flags
 and carry wares from one flag to the other.]]
      )
   )
}

lumberjack_message_05 = {
   title = _ "Placing another flag.", 
   pos = "topright", 
   body = rt(p(_
[[Nice how they work, isn't it? But the poor carrier has a very long way to go.
 We can make it easier on him (and more efficient for us) when we place another
 flag in between the road. You do it this time: click on the yellow flag symbol
 in between the two blue flags we just placed, then click on the]]
      )) .. rt("image=pics/menu_build_flag.png", p(_ "build flag symbol.")
   )
}

lumberjack_message_06 = {
   title = _ "Waiting for the lumberjack to be finished.", 
   pos = "topright", 
   body = rt(p(_
[[Well done! Let's wait till the lumberjack is finished. If you want things to
 go faster, simply use the PAGE UP key on your keyboard to increase the game
 speed. You can use PAGE DOWN to make the game slower again.]]
      )
   )
}

lumberjack_message_07 = {
   title = _ "Lumberjack is done.", 
   pos = "topright", 
   body = rt(p(_
[[Excellent. The lumberjack's hut is done. A lumberjack will now move in and
 start chopping down trees, so our trunks income is secured for now. Now on
 to raw stone.]]
      )
   )
}

-- TODO: there is a pile of raw stone to the left of your headquarter.
-- TODO: moving around
-- TODO: road building

-- TODO: this needs to come back in
initial_message_89 = {
   title = _"Choose a spot",
   pos = "topright",
   field = plr.starting_field,
   body = rt(p(_
[[You can build four things on fields in widelands: Flags, small houses, medium
 houses and big houses. But not every field is suitable for all. The build help
 eases recognition:]]
   )) .. rt("image=pics/big.png", p(_
[[Everything can be build on a green house symbol.]]
   )) .. rt("image=pics/medium.png", p(_
[[Everything except for big houses can be build on a yellow house symbol.]]
   )) .. rt("image=pics/small.png", p(_
[[Red building symbols can only house small buildings and flags.]]
   )) .. rt("image=pics/set_flag.png", p(_
[[And finally the yellow flag symbol only allows for flags.]]
   )) .. rt(p(_
[[If you place something on a field, the surrounding fields might have less
possibilities to house buildings, so choose your fields wisely.]]
      )
   )
}





