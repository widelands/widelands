-- =======================================================================
--                      Texts for the tutorial mission
-- =======================================================================

-- =========================
-- Some formating functions
-- =========================

include "scripting/formatting.lua"
include "scripting/format_scenario.lua"

-- =============
-- Texts below
-- =============
scould_player = {
   title = _"Nice And Easy Does It All the Time",
   body = rt(
      p(_[[I am sorry, but will I have to tear this down again. We might need the space here later on. If I am too slow for you, you might want to play a real game and just find everything out for yourself. Otherwise, please bear with me, I am not the youngest and quickest anymore.]]
      )
   ),
   h = 300,
   show_instantly = true
}

initial_message_01 = {
   title = _"Welcome to the Widelands Tutorial!",
   body = rt(
      h1(_"Welcome to Widelands!") ..
      p(_[[Widelands is a slow-paced build-up strategy game with an emphasis on construction rather than destruction. This tutorial will guide you through the basics of the game.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Dismiss this box by left-clicking on the button below.]])
   ),
   h = 300,
   w = 400
}
initial_message_02 = {
   title = _"Diving In",
   position = "topright",
   field = sf,
   body = rt(
      h1(_"Let’s dive right in!") ..
      p(_[[There are three different tribes in Widelands: the Barbarians, the Empire and the Atlanteans. All tribes have a different economy, strength and weaknesses, but the general gameplay is the same for all. We will play the Barbarians for now.]]) ..
      p(_[[You will usually start the game with one headquarters. This is the big building with the blue flag in front of it. The headquarters is a warehouse that stores wares, workers and soldiers. Some wares are needed for building houses, others for making other wares. Obviously, the wares in the headquarters will not last forever, so you must make sure to replace them. The most important wares in the early game are the basic construction wares: logs and granite. Let’s make sure that we do not run out of logs. For this, we need a lumberjack and a hut for him to stay in.]]) ..
      p(_[[We need to find a nice place for the lumberjack’s hut. To make this easier, we can activate ‘Show Building Spaces’. There are two ways you can do this, either by clicking on the ‘Show Building Spaces’ button at the bottom of the screen, which is the fourth one from the left. Or you can use the SPACE key to toggle it.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Left-click the ‘OK’ button to close this box and then try it.]])
   ),
   obj_name = "enable_buildhelp",
   obj_title = _"Enable the showing of building spaces",
   obj_body = rt(
      h1(_"Show Building Spaces") ..
      p(_[[It is easier to understand what type of buildings can be built on which field when the symbols for the building spaces are enabled.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Do so now, either by pressing SPACE or by clicking the fourth button from the left at the very bottom of the screen. Right-click on this window and then give it a try.]])
   )
}

lumberjack_message_01 = {
   title = _"Lumberjack’s Spot",
   position = "topright",
   field = first_lumberjack_field,
   body = rt(
      p(_[[There you go. I will explain about all those symbols in a minute. First, let me show you how to make a lumberjack’s hut and how to connect it with a road. There is a sweet spot for a lumberjack right next to those trees. I’ll describe the steps I will take and then ask you to click on the ‘OK’ button for me to demonstrate.]])
   ),
   h = 300,
   w = 350
}

lumberjack_message_02 = {
   title = _"Building the Lumberjack",
   position = "topright",
   body = rt(
      p(_[[First, I’ll left-click on the symbol where I want the lumberjack’s hut to be built. A window will appear where I can choose between buildings. Because I’ll click a yellow house symbol – which means that its field can house medium and small buildings – I am presented with all the medium buildings that I can build. The lumberjack’s hut is a small building, so I will go on to select the small buildings tab. Then I’ll choose the lumberjack’s hut.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Click the ‘OK’ button to watch me. I’ll go really slowly: I will click – then select the tab – and finally I’ll choose the building.]])
   ),
   h = 300
}

lumberjack_message_03a = {
   title = _"Building a Connecting Road",
   position = "topright",
   body = rt(
      p(_[[That won’t do yet. I still need to connect the lumberjack’s hut to the rest of my road network. After ordering the construction site, I was automatically put into road building mode, so all I have to do is click on the blue flag in front of my headquarters.]])
   ),
   show_instantly = true,
   h = 300,
   w = 350
}

lumberjack_message_03b = {
   title = _"Building a Connecting Road",
   position = "topright",
   body = rt(
      p(_[[That won’t do yet. I still need to connect the lumberjack’s hut to the rest of my road network. You have disabled the option ‘Start building road after placing a flag’ (to change that, choose ‘Options’ in the Widelands main menu). Therefore, I have entered the road building mode manually. I will tell you later how to do that. To build the road, all I have to do now is click on the blue flag in front of my headquarters.]])
   ),
   show_instantly = true,
   h = 300,
   w = 350
}

lumberjack_message_04 = {
   title = _"Waiting for the Lumberjack to Go Up",
   position = "topright",
   body = rt(
      p(_[[Now watch closely while a builder leaves the headquarters and goes to the construction site. Also, a carrier will take position in between the two blue flags and carry wares from one blue flag to the other.]])
   ),
   h = 300,
   w = 350
}

lumberjack_message_05 = {
   title = _"Placing Another Flag",
   position = "topright",
   body = rt(
      p(_[[Nice how they are working, isn’t it? But the poor carrier has a very long way to go. We can make it easier for him (and more efficient for us) by placing another blue flag on the road.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[You try it this time: click on the yellow flag symbol in between the two blue flags we just placed and then click on the]])
   ) ..
   rt("image=pics/menu_build_flag.png", p(_"build flag symbol.")
   ),
   h = 300,
   obj_name = "build_flag_on_road_to_lumberjack",
   obj_title = _"Build a flag to divide the road to the lumberjack",
   obj_body = rt(
      h1(_"Build a Flag on the Road") ..
      p(_[[The shorter your road segments are, the faster your wares will be transported. You should therefore make sure that your roads have as many flags as possible.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Build a blue flag now in the middle of the road that connects your headquarters to your lumberjack’s hut.]])
   )
}

lumberjack_message_06 = {
   title = _"Waiting For the Hut to be Finished",
   position = "topright",
   body = rt(
      p(_[[Well done! Let’s wait till the hut is finished.]]) ..
      p(_[[If you want things to go faster, simply use the PAGE UP key on your keyboard to increase the game speed. You can use PAGE DOWN to make the game slower again.]])
   ),
   h = 300,
   w = 350
}

flag_built = {
   title = _"Waiting for the Hut to be Finished",
   position = "topright",
   body = rt(
      p(_[[I wanted to teach you how to build new flags, but it seems you have already found out on your own. Well done!]]) ..
      p(_[[Now you have split the road in two parts with a carrier each. This means less work for him and higher efficiency for us. You should therefore always place as many flags as possible on your roads.]]) ..
      p(_[[Now we only have to wait till the hut is finished.]]) ..
      p(_[[If you want things to go faster, simply use the PAGE UP key on your keyboard to increase the game speed. You can use PAGE DOWN to make the game slower again.]])
   ),
   h = 350
}

construction_site_window = {
   title = _"The Construction Site",
   body = rt(
      h1(_"Let's see the progress") ..
      p(_[[If you click on the construction site, a window opens. You can see the wares that are still missing grayed out. You can also see the progress of this construction site.]]) ..
      -- The player doesn't know about the statistics yet. First things first.
      p(_[[To close the window, simply right-click on it. All windows in Widelands can be closed that way, except the ones with instructions, like this one. Try it out!]])
   ),
   h = 300,
   w = 350
}

lumberjack_message_07 = {
   title = _"Lumberjack is Done",
   position = "topright",
   body = rt(
      p(_[[Excellent. The lumberjack’s hut is done. A lumberjack will now move in and start chopping down trees, so our log income is secured for now. Now on to the granite.]])
   ),
   h = 300,
   w = 350
}

inform_about_rocks = {
   title = _"Some Rocks Were Found",
   body = rt(h1(_"Getting a Quarry Up")) ..
   rt(
      p(_[[Granite can be mined in granite mines, but the easier way is to build a quarry next to some rocks lying around. As it happens, there is a pile of them just to the west (left) of your headquarters. I will teach you now how to move your view over there.]]) ..
      paragraphdivider() ..
      listitem_arrow(_[[There are three ways to move your view. The first one is using the cursor keys on your keyboard. Go ahead and try this out.]]) ..
      listitem_bullet(_[[Click the ‘OK’ button and then move the view using the cursor keys]])
   ),
   h = 350,
   obj_name = "move_view_with_cursor_keys",
   obj_title = _"Move your view with the cursor keys",
   obj_body = rt(
      h1(_"Moving Your View") ..
      p(_[[Moving your view is essential to get a complete overview of your whole economy. There are three ways to move your view in Widelands.]]) ..
      paragraphdivider() ..
      listitem_arrow(_[[The first one is to use the cursor keys on your keyboard.]]) ..
      listitem_arrow(_[[The second one is the more common and faster one: press-and-hold the right mouse button anywhere on the map, then move your mouse around and you’ll see the view scroll.]]) ..
      listitem_arrow(_[[The third one is to use the minimap. It is especially useful for traveling big distances.]])
   )
}

tell_about_right_drag_move = {
   title = _"Other Ways to Move the View",
   body = rt(
      p(_[[Excellent. Now there is a faster way to move, using the mouse instead:]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Simply right-click-and-hold anywhere on the map, then drag the mouse and instead of the cursor, the view will be moved. Try it.]])
   ),
   h = 300,
   w = 350,
   obj_name = "move_view_with_mouse",
   obj_title = _"Move your view with the mouse",
   obj_body = inform_about_rocks.obj_body,
}

tell_about_minimap = {
   title = _"Use the minimap",
   body = rt(
      p(_[[Very good. And now about the minimap. You can open it by clicking on the]])
   ) ..
   rt("image=pics/menu_toggle_minimap.png", p(_[[minimap button at the bottom of the screen or simply by using the keyboard shortcut ‘m’.]])
   ) ..
   rt(
      p(_[[The minimap shows the complete map in miniature. You can directly jump to any field by left-clicking on it. You can also toggle buildings, roads, flags and player indicators on and off inside the minimap.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Try it out. Open the minimap, click on a few buttons and try moving around. Close it when you have experimented enough.]])
   ),
   h = 350,
   obj_name = "use_minimap",
   obj_title = _"Learn to use the minimap",
   obj_body = rt(
      paragraphdivider() ..
      listitem_bullet(_[[Open the minimap by using the third button from the left on the bottom of your screen or the ‘m’ key.]]) ..
      listitem_bullet(_[[Play around a bit with the different overlays (roads, flags, etc.)]]) ..
      listitem_bullet(_[[Close the minimap when you are ready to continue by using the same button or ‘m’ again. Of course, a right-click also works.]])
   )
}

congratulate_and_on_to_quarry = {
   title = _"Onward to the Quarry",
   body = rt(p(_[[Great. Now about that quarry…]])),
   h = 200,
   w = 250
}

order_quarry_recap_how_to_build = {
   field = first_quarry_field,
   position = "topright",
   title = _"How to Build a Quarry",
   body = rt(
      p(_[[Build a quarry next to those rocks here. Remember how I did it earlier?]]) ..
      p(_[[Make sure that you are showing the building spaces, then just click on the space were you want the building to be, choose it from the window that appears, and it is placed. Maybe this is a good time to explain about all those building space symbols we activated earlier.]]) ..
      p(_[[You can build four things on fields in Widelands: flags, small houses, medium houses and big houses. But not every field can hold everything. The build space symbols ease recognition:]])
   ) ..
   rt("image=pics/big.png", p(_[[Everything can be built on the green house symbol.]])) ..
   rt("image=pics/medium.png", p(_[[Everything except for big buildings can be built on a yellow house symbol.]])) ..
   rt("image=pics/small.png", p(_[[Red building symbols can only hold small buildings and flags.]])) ..
   rt("image=pics/set_flag.png", p(_[[And finally, the yellow flag symbol only allows for flags.]])) ..
   rt(
      p(_[[If you place something on a field, the surrounding fields might have less space for holding buildings, so choose your fields wisely.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Now go ahead, try it. The quarry is a small building, so if you click on a medium or big building symbol, you will have to select the small buildings tab first to find it. Go on, check it out!]])
   ),
   obj_name = "build_a_quarry",
   obj_title = _"Build a quarry next to the rocks",
   obj_body = rt(
      h1(_"Build a Quarry") ..
      paragraphdivider() ..
      listitem_bullet(_[[There are some rocks to the west of your headquarters. Build a quarry right next to them.]]) ..
      listitem_arrow(_[[The quarry is a small building like the lumberjack’s hut. You can therefore build it on any field that shows a red, yellow or green house when the building spaces symbols are enabled (Press SPACE for that).]]) ..
      listitem_arrow(_[[Just click on any house symbol next to the rocks, select the small buildings tab in the window that opens up, then click on the quarry symbol.]])
   )
}

talk_about_roadbuilding_00a = {
   position = "topright",
   field = wl.Game().map:get_field(9,12),
   title = _"Road Building",
   body = rt(
      p(_[[Excellent! Directly after placing the building, you have been switched into road building mode. The new road will start at the flag in front of your newly placed construction site. You can enter road building mode for any flag by left-clicking on a flag and selecting]])
      ) ..
   rt("image=pics/menu_build_way.png", p(_[[the road building symbol.]])) ..
   rt(
      p(_[[If you decide you do not want to build a road at this time, you can cancel road building by clicking on the starting flag of the road and selecting]])) ..
   rt("image=pics/menu_abort.png", p(_[[the abort symbol.]])) ..
   rt(
      p(_[[Now, about this road. Remember: we are already in road building mode since you just ordered the quarry. You can either make it longer by one field at a time by left-clicking multiple times on neighboring fields for perfect control over the route the road takes, like so:]])
   ),
   show_instantly = true
}

talk_about_roadbuilding_00b = {
   position = "topright",
   field = road_building_field,
   title = _"Road Building",
   body = rt(
      p(_[[Excellent! To enter road building mode for any flag, left-click on a flag and select]])
      ) ..
   rt("image=pics/menu_build_way.png", p(_[[the road building symbol.]])) ..
   rt(
      p(_[[If you decide that you do not want to build a road at this time, you can cancel road building by clicking on the starting flag of the road and selecting]])) ..
   rt("image=pics/menu_abort.png", p(_[[the abort symbol.]])) ..
   rt(
      p(_[[Now, about this road. I’ll enter the road building mode and then make it longer by one field at a time by left-clicking multiple times on neighboring fields for perfect control over the route the road takes, like so:]])
   ),
   show_instantly = true
}

talk_about_roadbuilding_01 = {
   position = "topright",
   field = road_building_field,
   title = _"Road Building",
   body = rt(p(_[[Or, you can directly click the flag where the road should end, like so:]])),
   h = 200,
   w = 250
}

talk_about_roadbuilding_02 = {
   position = "topright",
   title = _"Road Building",
   body = rt(
      p(_[[One more thing: around the field where your road would end, you can see different markers. They have the following meaning:]])
   ) ..
   rt("image=pics/roadb_green.png", p(_[[The terrain is flat here. Your carriers will be very swift on this terrain.]])) ..
   rt("image=pics/roadb_yellow.png", p(_[[There is a small slope to climb to reach this field. This means that your workers will be faster walking downhill than they will be walking uphill.]])) ..
   rt("image=pics/roadb_red.png", p(_[[The connection between the fields is extremely steep. The speed increase in one direction is huge while the slowdown in the other is also substantial.]])) ..
   rt(
      p(_[[Keep the slopes in mind while placing roads and use them to your advantage. Also, try to keep roads as short as possible and always remember to place as many flags as you can on road segments to share the load better. If you hold Ctrl or Shift+Ctrl while you finish the road, flags are placed automatically.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Now please rebuild the road between your quarry and your headquarters.]])
   ),
   h = 450,
   obj_name = "build_road_to_quarry",
   obj_title = _"Connect the quarry to the headquarters",
   obj_body = rt(
      h1(_"Connect Your Construction Site") ..
      p(_[[Connect your quarry construction site to your headquarters with a road. You would have been put directly into road building mode after ordering a new site. But now, you aren’t.]]) ..
      paragraphdivider() ..
      listitem_arrow(_[[To build a completely new road, just click on the flag in front of your construction site, click on the build road icon and then click on the flag in front of your headquarters. Wait for the completion of the quarry.]])
   )
}

quarry_not_connected = {
   title = _"Quarry not Connected",
   body = rt(
      p(_[[Your workers do not like to walk across country. You have to build a road from your headquarters to the construction site so that carriers can transport wares. The simplest way is to click on the construction site’s flag, choose ‘Build road’, and then click on the destination flag (the one in front of your headquarters), just like I’ve demonstrated.]])
   ),
   w = 350,
   h = 250
}

quarry_illegally_destroyed = {
   title = _"You Destroyed the Construction Site!",
   body = rt(
      p(_[[It seems like you destroyed a construction site for a quarry we wanted to build. Since we need the stones, I suggest you reload the game from a previous savegame. Luckily, these are created from time to time. To do so, you have to go back to the main menu and choose ‘Single Player’ → ‘Load Game’. And please be a bit more careful next time.]])
   ),
   w = 350,
   h = 250
}

build_second_quarry = {
   position = "topright",
   title = _"Build a second quarry",
   body = rt(
      p(_[[When there are many rocks, you can consider building another quarry. This will make the granite production faster.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Build a second quarry near the rocks and connect it to your road network.]])
   ),
   obj_name = "build_the_second_quarry",
   obj_title = _"Build another quarry",
   obj_body = rt(
      h1(_"Build another quarry") ..
      p(_[[Build a second quarry next to the rocks. Do not forget to connect it to another flag.]]) ..
      paragraphdivider() ..
      listitem_arrow(_[[You can connect the new road to any flag of your existing road network. You can create junctions everywhere, not only in front of buildings.]])
   ),
   h = 300,
   w = 350
}

census_and_statistics_00 = {
   title = _"Census and Statistics",
   body = rt(
      p(_[[While we wait, I’ll quickly show you another useful feature. All construction sites look the same, and some buildings look alike. It is sometimes hard to tell them apart. Widelands offers a feature to show label texts over the buildings. They are called the ‘census’ and you can toggle them via the ‘c’ key or via the button on the ‘Watch’ tab of any field.]]) ..
      p(_[[Similar to this are the building statistics, which are also toggled via a button on the ‘Watch’ tab of any field. The hotkey for it is ‘s’. This will display information about the productivity of buildings or the progress of construction sites.]]) ..
      p(_[[Let me quickly enable these two for you. Remember: ‘c’ and ‘s’ are the keys. Alternatively, you can click on any field without a building on it, select the watch tab and then click on the corresponding buttons.]])
   )
}

census_and_statistics_01 = {
   title = _"Census and Statistics",
   body = rt(p(_[[Now we know what’s going on. Let’s wait for the quarries to finish.]])),
   h = 200,
   w = 250
}

teaching_about_messages = {
   popup = true,
   title = _"Introducing Messages",
   body = rt(
      h1(_"Messages") ..
      p(_[[Hi, it’s me again! This time, I have sent you a message. Messages are sent to you by Widelands to inform you about important events: empty mines, attacks on your tribe, won or lost military buildings, resources found…]]) ..
      p(_[[The message window can be toggled by the button on the very right at the bottom of the screen. This button will also change appearance whenever new messages are available, but there is also a bell sound played whenever you receive a new message.]]) ..
      p(_[[You have two messages at the moment. This one, which you are currently reading, and the one that informed you that a new headquarters was added to your economy. Let’s learn how to archive messages: You can check them off in your inbox so that they get a tick-symbol in front of them. Then, you can click the]])
   ) ..
   rt("image=pics/message_archive.png", p(_[[archive message button to move them into your archive.]])) ..
   rt(
      paragraphdivider() ..
      listitem_bullet(_[[Archive all messages that you currently have in your inbox, including this one.]])
   ),
   obj_name = "archive_all_messages",
   obj_title = _"Archive all messages in your inbox",
   obj_body = rt(
      h1(_"Archive Your Inbox Messages") ..
      p(_[[The message window is central to fully controlling your tribe’s fortune. However, you will get a lot of messages in a real game. To keep your head straight, you should try to keep the inbox empty.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Archive all your messages in your inbox now.]]) ..
      listitem_arrow(_[[To do so, open the message window by pressing ‘n’ or clicking the rightmost button at the very bottom of the screen. Then mark all messages by checking the check box in front of them. Then, click the ‘Archive All’ button.]])
   )
}

closing_msg_window_00 = {
   position = "topright",
   field = first_quarry_field,
   title = _"Closing Windows",
   body = rt(
      p(_[[Excellent. Do you remember how to close windows? You simply have to right-click on them. This will work with all windows except for story message windows like this one. Go ahead and try it.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[First, close this window by pressing the button below, then right-click into the messages window to close it.]])
   ),
   h = 300,
   w = 350,
   obj_name = "close_message_window",
   obj_title = _"Close the messages window",
   obj_body = rt(
      h1(_"Close the Messages Window") ..
      p(_[[All windows in Widelands can be closed by right-clicking into them. Some windows can also be toggled with the buttons at the very bottom of the screen.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Close the messages window now by right-clicking into it.]])
   )
}

closing_msg_window_01 = {
   position = "topright",
   field = first_quarry_field,
   title = _"Closing Windows",
   body = rt(
      p(_[[Well done! Let’s see how messages work in a real game, shall we? For this, I’ll take all rocks away from the poor stonemasons in the quarries. They will then send a message each that they can’t find any in their working areas the next time they try to do some work.]])
   ),
   h = 300,
   w = 350
}

destroy_quarries_message = {
   position = "topright",
   title = _"Messages Arrived!",
   body = rt(
      p(_[[You received some messages. See how the button at the bottom of the screen has changed appearance? You can destroy the quarries now as they are no longer of any use and just blocking space. To do so, there are two possibilities:]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Burning down the quarry: this is the fastest way of clearing the space. While the worker abandons the building, the wares are lost.]]) ..
      listitem_bullet(_[[Dismantling the quarry: a builder will walk from the headquarters to dismantle the quarry piece by piece. Thereby, you regain some of the resources you used for the construction.]])
   ),
   h = 300,
   obj_name = "destroy_quarries",
   obj_title = "Destroy the two quarries",
   obj_body = rt(
      p(_[[Since our quarries are useless now, you can destroy them and reuse the space later on.]]) ..
      paragraphdivider() ..
      listitem_arrow(_[[There are two different ways of destroying a building: burning down and dismantling. Try them both out on your quarries.]]) ..
      listitem_arrow(_[[Burning down the quarry: This is the fastest way of clearing the space. While the worker abandons the building, the wares are lost.]]) ..
      listitem_arrow(_[[Dismantling the quarry: A builder will walk from the headquarters to dismantle the quarry piece by piece. Thereby, you regain some of the resources you used for the construction.]])
   )
}

introduce_expansion = {
   title = _"Expanding Your Territory!",
   body = rt(
      p(_[[There is one more thing I’d like to teach you now: Expanding your territory. The place that we started with around our headquarters is barely enough for a basic building infrastructure, and we do not have access to mountains, which we need to mine minerals and coal. So, we have to expand our territory.]]) ..
      p(_[[Expanding is as simple as building a military building at the edge of your territory. The Barbarians have a selection of different military buildings: sentries, barriers, towers, fortresses and citadels. The bigger the building, the more expensive it is to build, but the more land it will conquer around itself and the more soldiers can be stationed there. The buildings also vary in their vision range: buildings with a tower see farther than others.]]) ..
      p(_[[As soon as a military building is manned, it will extend your land. I will tell your more about military buildings in another tutorial.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Let’s try it out now: build a military building on your border.]]) ..
      listitem_arrow(_[[The sentry is the only military site that fits on a small building plot. If your lumberjack has cleared enough space, you can also build another military building.]])
   ),
   obj_name = "expand_territory",
   obj_title = _"Expand your territory",
   obj_body = rt(
      h1(_"Make your territory grow") ..
      p(_[[In Widelands, it is necessary to build many buildings, which take up a lot of space. To expand your territory, you have to build military buildings next to your border. Every tribe has several military buildings.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[The Barbarians have four different military buildings you can build: the sentry (small), the barrier and the tower (both medium) and the fortress (big). Just choose the one you like most.]]) ..
      listitem_arrow(_[[Remember that big buildings (green icon) cannot be built on small (red) or medium (yellow) building plots, but buildings can be built on a building plot that provides more space than they need. You should always keep that in mind when you search for a suitable place.]])
   )
}


military_building_finished = {
   title = _"Military Site Occupied",
   body = rt(
      h1(_"Your territory has just grown!") ..
      p(_[[Great. Do you see how your territory has grown since your soldiers entered your new military building?]]) ..
      p(_[[Every military building has a certain conquer area – the more expensive the building, the more land it conquers.]])
   ),
   h = 300,
   w = 350
}

conclude_tutorial = {
   title = _"Conclusion",
   body = rt(
      h1(_"Conclusion") ..
      p(_[[This concludes the first tutorial. In order to learn more about the game, I suggest to play one of the other tutorials. Each of them covers a different topic.]]) ..
      p(_[[However, since you now know how to control Widelands, you can also start a game (or continue this one) and discover more by yourself.]]) ..
      p(_[[To leave this game and return to the main menu, click on the]])
   ) ..
   rt("image=pics/menu_options_menu.png", p(_[[‘Options’ menu button on the very left at the bottom of the screen. Then click the]])) ..
   rt("image=pics/menu_exit_game.png", p(_[[‘Exit Game’ button.]])) ..
   rt(p(_[[Thanks for playing this tutorial. Enjoy Widelands and remember to visit us at]])) ..
   rt("text-align=center", "<p font-size=24 font-decoration=underline>http://www.widelands.org</p>"),
   h = 450
}
