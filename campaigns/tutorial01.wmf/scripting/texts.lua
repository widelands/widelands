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
[[Widelands is a slow-paced build-up strategy with an emphasis on construction,
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
[[There are three different tribes in Widelands: the barbarians, the empire and
 the atlanteans. All tribes have a different economy, strength and weaknesses,
 but the general gameplay is the same for all. We play the barbarians for now.]]
      ) .. p(_
[[You usually start the game with one headquarters. This is the big building
 with the blue flag in front of it. The headquarters is a ware house that stores
 wares, workers and soldiers. Some wares are needed for building houses, others
 for making other wares. Obviously, the wares in the headquarters will not last
 forever, so you must make sure to reproduce them. The most important wares in
 the early game are the basic build wares: trunks and raw stone. Let's make sure
 that we do not run out of trunks. For this, we need to build a lumberjack.]]
      ) .. p(_
[[We need to find a nice place for the lumberjack. To make this easier, we
 activate the build help. To do this, either click on the build help button
 at the bottom of the screen which is the fourth one from the left. Or you
 can use the SPACE key to toggle it.]]
      )
   ),
}

lumberjack_message_01 = {
   title = _ "Lumberjack's spot",
   pos = "topright",
   field = first_lumberjack_field,
   body = rt(p(_
[[There you go. I will explain about all those symbols in a minute. First, let
 me show you how to make a lumberjack's hut and how to connect it with a road.
 There is a sweet spot for a lumberjack right next to those trees.]]
      ) .. p(_
[[First, I'll left-click on the symbol were I want the lumberjack to be built. A
 window will appear where I can choose between buildings. ]]
      )
   )
}

lumberjack_message_02 = {
   title = _ "Building the lumberjack",
   pos = "topright",
   body = rt(p(_
[[Now, I left-click on the lumberjack. This places a construction site at the
 given location.]]
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
 in front of my headquarters.]]
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
   title = _ "Placing another flag",
   pos = "topright",
   body = rt(p(_
[[Nice how they work, isn't it? But the poor carrier has a very long way to go.
 We can make it easier for him (and more efficient for us) when we place another
 flag on the road. You try it this time: click on the yellow flag symbol
 in between the two blue flags we just placed and then click on the]]
      )) .. rt("image=pics/menu_build_flag.png", p(_ "build flag symbol.")
   )
}

lumberjack_message_06 = {
   title = _ "Waiting for the hut to be finished",
   pos = "topright",
   body = rt(p(_
[[Well done! Let's wait till the hut is finished. If you want things to
 go faster, simply use the PAGE UP key on your keyboard to increase the game
 speed. You can use PAGE DOWN to make the game slower again.]]
      )
   )
}

lumberjack_message_07 = {
   title = _ "Lumberjack is done",
   pos = "topright",
   body = rt(p(_
[[Excellent. The lumberjack's hut is done. A lumberjack will now move in and
 start chopping down trees, so our trunks income is secured for now. Now on to
 the raw stone.]]
      )
   )
}

inform_about_stones = {
   title = _ "Some stones were found",
   body = rt(h1(_ "Getting a quarry up.")) ..
   rt(p(_
[[Stones can be mined in granite mines, but the easier way is to build a quarry
 next to some stones laying around. As it happens, there is a pile of them
 laying right to the west of your headquarters. I will teach you now how to
 move your view over there]]
      ) .. p(_
[[There are two ways to move your view. The first one is using the cursor keys
 on your keyboard. Go ahead and try this out, move the view using the cursor
 keys]]
      )
   )
}

tell_about_right_drag_move = {
   title = _ "Other ways to move the view",
   body = rt(p(_
[[Excellent. Now there is a faster way to move, using the mouse instead: Simply
 right-click-and-hold anywhere on the map, then drag the mouse and instead
 of the cursor, the view will be moved. Try it.]]
      )
   )
}

congratulate_and_on_to_quarry = {
   title = _ "Onward to the quarry",
   body = rt(p(_
[[Great. Now about the quarry...]]
      )
   )
}

order_quarry_recap_how_to_build = {
   field = first_quarry_field,
   pos = "topright",
   title = _ "How to build a quarry",
   body = rt(p(_
[[Build a quarry next to those stones here. Remember how I did it earlier?
 Just click the place were you want the building to be, choose it from the
 window that appears and it is placed. Maybe it is a good time to explain about
 all those build help symbols we activated earlier.]]
   ) .. p(_
[[You can build four things on fields in Widelands: Flags, small houses, medium
 houses and big houses. But not every field can hold anything. The build help
 eases recognition:]]
   )) .. rt("image=pics/big.png", p(_
[[Everything can be build on the green house symbol.]]
   )) .. rt("image=pics/medium.png", p(_
[[Everything except for big buildings can be build on a yellow house symbol.]]
   )) .. rt("image=pics/small.png", p(_
[[Red building symbols can only hold small buildings and flags.]]
   )) .. rt("image=pics/set_flag.png", p(_
[[And finally the yellow flag symbol only allows for flags.]]
   )) .. rt(p(_
[[If you place something on a field, the surrounding fields might have less
 space for holding buildings, so choose your fields wisely.]]
   )) .. rt(p(_
[[Now go ahead, try it. The quarry is a small building, so if you click on a
 medium or big building symbol, you will have to select the small buildings
 tab first to find it. Go on, check it out!]]
      )
   )
}

talk_about_roadbuilding_00 = {
   pos = "topright",
   title = _ "Road building",
   body = rt(p(_
[[Excellent! Directly after you placed a building, you are in road building
 mode. The new road will start at the flag in front of your newly placed
 construction site. You can enter road building mode for any flag by
 left-clicking on a flag and selecting]]
      )) .. rt("image=pics/menu_build_way.png", p(_
[[the road building symbol.]]
      )) .. rt(p(_
[[If you decide you do not want to build a road at this time, you can cancel
 road building by clicking on the starting flag of the road and selecting]]
      )) .. rt("image=pics/menu_abort.png", p(_
[[the abort symbol.]]
      )) .. rt(p(_
[[Now, you can either make it longer by one field at a time by left-clicking
 multiple times on neighbouring fields for perfect control over the route the
 road takes like so:]]
      ))
}

talk_about_roadbuilding_01 = {
   pos = "topright",
   title = _ "Road building",
   body = rt(p(_
[[Alternatively, you can directly click the flag where
 the road should end like so.]]
   ))
}

talk_about_roadbuilding_02 = {
   pos = "topright",
   title = _ "Road building",
   body = rt(p(_
[[One more thing: around the field where your road would end you can see
 different markers. They have the following meaning:]]
      )) .. rt("image=pics/roadb_green.png", p(_
[[The terrain is flat here. Your carriers will be very swift on this terrain.]]
   )) .. rt("image=pics/roadb_yellow.png", p(_
[[There is a small slope to climb to reach this field. This means you're
 workers are faster walking downwards than they are walking upwards.]]
   )) .. rt("image=pics/roadb_red.png", p(_
[[The connection between the fields is extremely steep. The speed increase in
 one direction is huge while the slowdown in the other is also substantial.]]
   )) .. rt(p(_
[[Keep the slopes in mind while placing roads and use them to your advantage.
 Also try to keep roads as short as possible and always remember to place as
 many flags as you can on road segments to share the load better.]]
   )) .. rt(p(_
[[Now please rebuild the road between your quarry and your headquarters. 
 We'll wait until the quarry is completed.]]
   ))
}

teaching_about_messages = {
   popup = true,
   title = _ "Introducing messages",
   body = rt(h1(_"Messages") ..
      p(_
[[Hi, it's me again! This time, I sent you a message. Messages are sent to you
 by Widelands to inform you about important events: empty mines, attacks on your
 tribe, won or lost military buildings, resources found...]]
      ) .. p(_
[[The message window can be toggled by the button on the very right at the
 bottom of the screen. This button also changes appearance if new messages are
 available, but there is also a bell sound played whenever you receive a new
 message.]]
      ) .. p(_
[[Currently, you have two messages. This one which you are currently reading and
 the one that informed you that a new headquarters was added to your economy.
 Let's learn how to archive messages: You can check them off in your inbox so
 that they get a tick-symbol in front of them. Then, you can click the]]
      )) .. rt("image=pics/message_archive.png", p(_
[[archive message button to move them into your archive.]]
      )) .. rt(p(_
[[Archive all messages, including this one, that you currently have in your
 inbox.]]
      )
   )
}

closing_msg_window = {
   pos = "topright",
   field = first_quarry_field,
   title = _"Closing windows",
   body = rt(p(_
[[Excellent. By the way: closing windows in Widelands is as easy as
 right-clicking on them. This works with all windows except for story message
 windows like this one.]]
      ) .. p(_
[[Let's see how messages work in the real game, shall we? For this, I'll take
 all stones away from the poor stonemason in the quarry. He will then send a
 message that he can't find any in his working area]]
      )
   )
}

conclude_messages = {
   pos = "topright",
   title = _ "Message arrived!",
   body = rt(p(_
[[A message has been sent to you. See how the button at the bottom of the
 screen has changed appearance? You should now burn this quarry down as it is
 no longer of any use and is just blocking space.]]
   ))
}

introduce_expansion = {
   title = _ "Expanding your territory!",
   body = rt(p(_
[[There is one more thing I'd like to teach you now: Expanding your territory.
 The place that we start with around our headquarters is barely enough for a
 basic build infrastructure and we do not have access to mountains which we
 need to mine minerals and coal. So we have to expand our territory]]
      ) .. p(_
[[Expanding is as simple as building a military building at the corner of
 your territory. The barbarians have a few different military buildings:
 sentries, barriers, donjons, strongholds, fortresses and citadels. The bigger
 the building, the more expensive it is to build, the more land it conquers
 around itself and the more soldiers can be stationed there. The buildings also
 vary in their vision range: buildings with a tower see farther than others.]]
      ) .. p(_
[[As soon as a military building is manned, it extends your land. You can then
 burn it down again if you need the place. But note that your land is then
 vulnerable: any military site from another player can conquer the land. You
 therefore need military sites to keep military influence over your land.]]
      ) .. p(_
[[Let's try it out now: Build a military building on your eastern border.]]
      )
   )
}

conclude_tutorial = {
   title = _ "Conclusion",
   body = rt(h1(_"Conclusion") ..
      p(_
[[This concludes the tutorial. There is some stuff we have not covered: mining,
 training of soldiers, enhancing buildings, warfare, the statistics, the
 mini-map. We have not even build a single producing building even though
 producing wares is the most important thing in Widelands. But you can learn
 these while you go through the individual tribe's introduction campaigns. Each
 consists of some scenarios introducing the tribes and their economy while
 introducing the background story of Widelands.]]
      ) .. p(_
[[You can continue playing this map or you can end this game whenever you like.
 To leave this game and return to the main menu click on the]]
      )) .. rt("image=pics/menu_options_menu.png", p(_
[[options menu button on the very left at the bottom of the screen.
 Then click the]]
      )) .. rt("image=pics/menu_exit_game.png", p(_
[[exit game button.]]
      )) .. rt(p(_
[[Thanks for playing this tutorial. Enjoy Widelands and remember
 to visit us at]]
      )) .. rt("text-align=center",
   "<p font-size=24 font-decoration=underline>http://www.widelands.org</p>"
   )
}



