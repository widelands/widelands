-- =======================================================================
--                      Texts for the tutorial mission
-- =======================================================================

-- ================
-- General messages
-- ================

local close_story_window_instructions = _[[Click on the ‘OK’ button or press the ‘Enter ⏎’ key on the keyboard to close this window.]]

scold_player = {
   title = _"Nice And Easy Does It All the Time",
   body = (
      p(_[[I am sorry, but will I have to tear this down again. We might need the space here later on. If I am too slow for you, you might want to play a real game and just find everything out for yourself. Otherwise, please bear with me, I am not the youngest and quickest anymore.]]
      )
   ),
   h = 150,
   show_instantly = true
}

-- Teaching basic UI controls

-- ==============
-- Starting Infos
-- ==============

obj_initial_close_story_window = {
   name = "initial_close_story_window",
   title=_"Close this window",
   number = 1,
   body = objective_text(_"Close this window",
      li(close_story_window_instructions)
   )
}
initial_message_01 = {
   title = _"Welcome to the Widelands Tutorial!",
   body = (
      h1(_"Welcome to Widelands!") ..
      li_image("images/logos/wl-ico-64.png",
         _[[Widelands is a slow-paced build-up strategy game with an emphasis on construction rather than destruction. This tutorial will guide you through the basics of the game.]]) ..
      li_arrow(_[[You will be guided through this tutorial by objectives]]) ..
      li(_[[Follow the intructions in the objective below so that I can show you where to find them.]])
   )
}

obj_initial_close_objectives_window = {
   name = "obj_initial_close_objectives_window",
   title=_"Objectives and how to close this window",
   number = 1,
   body = objective_text(_"Closing this window",
      p(_[[This is the ‘Objectives’ window. You can return to this window for instructions at any time.]]) ..
      li_image("images/wui/menus/objectives.png", _[[ You can open and close this window by clicking on the ‘Objectives’ button in the toolbar on the bottom of the screen.]]) ..
      li_arrow(_[[Like any other window, you can also close the ‘Objectives’ window by right-clicking on it.]]) ..
      li_arrow(_[[When you have accomplished an objective, it will disappear from the list above.]]) ..
      li(_[[Try it out.]])
   )
}

obj_initial_toggle_building_spaces = {
   name = "initial_toggle_building_spaces",
   title=_"Show building spaces",
   number = 1,
   body = objective_text(_"Show building spaces",
      p(_[[We need to find a nice place for the lumberjack’s hut. To make this easier, we can activate ‘Show Building Spaces’. There are two ways you can do this:]]) ..
      li_image("images/wui/menus/showhide.png", _[[Press the Space bar to toggle them, or select ‘Show Building Spaces’ in the ‘Show / Hide’ menu.]]) ..
      li(_[[Show the building spaces now.]])
   )
}

initial_message_02 = {
   title = _"Building Spaces",
   position = "topright",
   field = sf,
   body = (
      h1(_"Let’s dive right in!") ..
      li_image("tribes/initialization/barbarians/images/icon.png",
         _[[There are four different tribes in Widelands: the Barbarians, the Empire, the Atlanteans and the Frisians. All tribes have a different economy, strength and weaknesses, but the general gameplay is the same for all. We will play the Barbarians for now.]]) ..
      li_object("barbarians_headquarters", _[[You will usually start the game with one headquarters. This is the big building with the blue flag in front of it. The headquarters is a warehouse that stores wares, workers and soldiers. Some wares are needed for building houses, others for making other wares. Obviously, the wares in the headquarters will not last forever, so you must make sure to replace them. The most important wares in the early game are the basic construction wares: logs and granite. Let’s make sure that we do not run out of logs. For this, we need a lumberjack and a hut for him to stay in.]], plr.color) ..
      p(_[[We need to find a nice place for the lumberjack’s hut. To make this easier, we can activate ‘Show Building Spaces’.]]) ..
      li(_[[Left-click the ‘OK’ button to close this window. I'll show you where the menu is and where to show and hide the building spaces.]]) ..
      li_arrow(_[[Note that you cannot close this window by right-clicking on it. I have blocked this so that you will not close it by accident and miss important information.]])
   )
}

initial_message_03 = {
   title = _"Building Spaces",
   position = "topright",
   field = sf,
   body = (
      h1(_"Let’s dive right in!") ..
      li_object("barbarians_lumberjacks_hut", _[[Now that I have shown you where to show and hide the building spaces, please switch them on so that we can place our first building.]], plr.color)
   )
}

-- ==========
-- Lumberjack
-- ==========

lumberjack_message_01 = {
   title = _"Lumberjack’s Spot",
   position = "topright",
   field = first_lumberjack_field,
   body = (
      li_object("barbarians_lumberjacks_hut",
         _[[There you go. I will explain about all those symbols in a minute. First, let me show you how to make a lumberjack’s hut and how to connect it with a road. There is a sweet spot for a lumberjack right next to those trees. I’ll describe the steps I will take and then ask you to click on the ‘OK’ button for me to demonstrate.]], plr.color)
   ),
   h = 250,
   w = 350
}

lumberjack_message_02 = {
   title = _"Building the Lumberjack",
   position = "topright",
   body = (
      li_object("barbarians_lumberjacks_hut",
         _[[First, I’ll left-click on the symbol where I want the lumberjack’s hut to be built. A window will appear where I can choose between buildings. Because I’ll click a yellow house symbol – which means that its field can house medium and small buildings – I am presented with all the medium buildings that I can build. The lumberjack’s hut is a small building, so I will go on to select the small buildings tab. Then I’ll choose the lumberjack’s hut.]], plr.color) ..
      li(_[[Click the ‘OK’ button to watch me. I’ll go really slowly: I will click – then select the tab – and finally I’ll choose the building.]])
   ),
   h = 300,
   w = 350
}

lumberjack_message_03a = {
   title = _"Building a Connecting Road",
   position = "topright",
   body = (
      li_image("images/wui/fieldaction/menu_tab_buildroad.png", _[[That won’t do yet. I still need to connect the lumberjack’s hut to the rest of my road network. After ordering the construction site, I was automatically put into road building mode, so all I have to do is click on the blue flag in front of my headquarters.]]) ..
      li(close_story_window_instructions)
   ),
   show_instantly = true,
   h = 200,
   w = 350
}

lumberjack_message_03b = {
   title = _"Building a Connecting Road",
   position = "topright",
   body = (
      li_image("images/wui/fieldaction/menu_tab_buildroad.png",
         _[[That won’t do yet. I still need to connect the lumberjack’s hut to the rest of my road network. You have disabled the option ‘Start building road after placing a flag’ (to change that, choose ‘Options’ in the Widelands main menu). Therefore, I have entered the road building mode manually. I will tell you later how to do that. To build the road, all I have to do now is click on the blue flag in front of my headquarters.]]) ..
         li(close_story_window_instructions)
   ),
   show_instantly = true,
   h = 250,
   w = 350
}

lumberjack_message_04 = {
   title = _"Waiting for the Lumberjack to Go Up",
   position = "topright",
   body = (
      li_object("barbarians_builder",
         _[[Now watch closely while a builder leaves the headquarters and goes to the construction site. Also, a carrier will take position in between the two blue flags and carry wares from one blue flag to the other.]], plr.color) ..
         li(close_story_window_instructions)
   ),
   h = 200,
   w = 350
}

obj_lumberjack_place_flag = {
   name = "obj_lumberjack_place_flag",
   title=_"Build a flag to divide the road to the lumberjack",
   number = 1,
   body = objective_text(_"Build a Flag on the Road",
      p(_[[The shorter your road segments are, the faster your wares will be transported. You should therefore make sure that your roads have as many flags as possible.]]) ..
      li(_[[Build a blue flag now in the middle of the road that connects your headquarters to your lumberjack’s hut.]]) ..
      li_image("images/wui/fieldaction/menu_build_flag.png",_[[To build the flag, click on the yellow flag symbol in between the two blue flags we just placed and then click on the build flag symbol.]])
   ),
   h = 300,
   w = 350
}
lumberjack_message_05 = {
   title = _"Placing Another Flag",
   position = "topright",
   body = (
      li_object("barbarians_carrier",
         _[[Nice how they are working, isn’t it? But the poor carrier has a very long way to go. We can make it easier for him (and more efficient for us) by placing another blue flag on the road. You try it this time.]], plr.color)
   ),
   h = 450,
   w = 350
}

lumberjack_message_06 = {
   title = _"Waiting for the Lumberjack to Go Up",
   position = "topright",
   body = (
      li_image("images/wui/fieldaction/menu_tab_buildroad.png",
         _[[I wanted to teach you how to build new flags, but it seems you have already found out on your own. Well done!]]) ..
      p(_[[Now you have split the road in two parts with a carrier each. This means less work for him and higher efficiency for us. You should therefore always place as many flags as possible on your roads.]]) ..
      li(close_story_window_instructions)
   ),
   h = 250,
   w = 350
}

obj_lumberjack_progress = {
   name = "obj_lumberjack_progress",
   title=_"Let’s see the progress",
   number = 1,
   body = objective_text(_"Let’s see the progress",
      li(_[[Click on the construction site to have a look at it.]])
   ),
   h = 300,
   w = 350
}
lumberjack_message_07a = {
   title = _"The Construction Site",
   position = "topright",
   body = (
      h1(_"Let’s see the progress") ..
      li_object("barbarians_builder", _[[If you click on the construction site, a window will open. You can see the wares that are still missing grayed out. You can also see the progress of this construction site.]], plr.color)
   ),
   h = 450,
   w = 350
}

lumberjack_message_07b = {
   title = _"The Construction Site",
   position = "topright",
   body = (
      li_image("images/wui/buildings/toggle_workarea.png",
         _[[This button toggles showing the building’s work area. When the lumberjack leaves the building to look for trees to fell, he will only look for them within this area. Let me show you how perfect this spot is.]]) ..
      p(_[[Close the construction site window when you have seen enough.]]) ..
      li(close_story_window_instructions)
   ),
   h = 250,
   w = 350
}

lumberjack_message_08 = {
   title = _"Waiting for the Lumberjack to Go Up",
   position = "topright",
   body = (
      p(_[[Well done! Let’s wait till the hut is finished.]]) ..
      li_image("images/wui/menus/gamespeed.png",
         _[[If you want things to go faster, simply use the Page Up key on your keyboard to increase the game speed. You can use Page Down to make the game slower again.]]) ..
         p(_[[Change is rapid with pressed Ctrl and smooth with pressed Shift.]])
   ),
   h = 200,
   w = 350
}

lumberjack_message_09 = {
   title = _"The Lumberjack’s Hut is Done",
   position = "topright",
   body = (
      li_object("barbarians_lumberjacks_hut", _[[Excellent. The lumberjack’s hut is done. A lumberjack will now move in and start chopping down trees, so our log income is secured for now. Now on to the granite.]], plr.color)
   ),
   h = 200,
   w = 350
}

-- ==================
-- Moving the mapview
-- ==================

local moving_view_instructions =
   h2(_"Moving Your View") ..
   p(_[[Moving your view is essential to get a complete overview of your whole economy. There are three ways to move your view in Widelands.]]) ..
   li_arrow(_[[The first one is to use the cursor keys on your keyboard.]]) ..
   li_arrow(_[[The second one is the more common and faster one: press-and-hold the right mouse button anywhere on the map, then move your mouse around and you’ll see the view scroll.]]) ..
   li_arrow(_[[The third one is to use the minimap. It is especially useful for traveling big distances.]])


obj_moving_keyboard = {
   name = "move_view_with_cursor_keys",
   title=_"Move your view with the cursor keys",
   number = 1,
   body = objective_text(_"Move your view with the cursor keys",
      li(_[[There are three ways to move your view. The first one is using the cursor keys on your keyboard. Go ahead and try this out.]]) .. moving_view_instructions
   ),
}
tell_about_keyboard_move = {
   title = _"Some Rocks Were Found",
   body = (
      h1(_"Getting a Quarry Up") ..
      li_object("greenland_rocks6",
         _[[Granite can be mined in granite mines, but the easier way is to build a quarry next to some rocks lying around. As it happens, there is a pile of them just to the west (left) of your headquarters. I will teach you now how to move your view over there.]])
   ),
   h = 450,
}

obj_moving_right_drag = {
   name = "move_view_with_mouse",
   title=_"Move your view with the mouse",
   number = 1,
   body = objective_text(_"Move your view with the mouse",
      li(_[[Simply right-click-and-hold anywhere on the map, then drag the mouse and instead of the cursor, the view will be moved. Try it.]]) .. moving_view_instructions
   ),
}
tell_about_right_drag_move = {
   title = _"Moving Your View",
   body = (
      h1(_"Other Ways to Move the View") ..
      li_image("images/ui_basic/cursor_click.png",
         _[[Excellent. Now there is a faster way to move, using the mouse instead.]])
   ),
   h = 450,
}

obj_moving_minimap = {
   name = "use_minimap",
   title=_"Learn to use the minimap",
   number = 1,
   body = objective_text(_"Learn to use the minimap",
      li(_[[Try moving around by clicking on the minimap]]) ..
      li(_[[Play around a bit with the different overlays (roads, flags, etc.)]]) ..
      li(_[[When you are ready to continue, close the minimap by selecting ‘Hide Minimap’ in the ‘Map View’ menu or by pressing ‘m’. Of course, a right-click also works.]])
   ),
}
tell_about_minimap_1 = {
   title = _"Moving Your View",
   body = (
      h1(_"Using the Minimap") ..
      li_image("images/wui/menus/toggle_minimap.png",
         p(_[[Very good. And now about the minimap. ]]) ..
         -- TRANSLATORS it = the minimap
         p(_[[You can open it by selecting the ‘Show Minimap’ entry in the ‘Map View’ menu at the bottom of the screen or simply by using the keyboard shortcut ‘m’.]])) ..
      -- TRANSLATORS it = the minimap
      li_arrow(_[[I will open it for you.]])
   ),
   w = 350,
   h = 250,
}

tell_about_minimap_2 = {
   title = _"Moving Your View",
   body = (
   h1(_"Using the Minimap") ..
      li_image("images/wui/menus/toggle_minimap.png",
         _([[The minimap shows the complete map in miniature. ]]
         .. [[You can directly jump to any field by left-clicking on it. ]]
         .. [[You can also toggle buildings, roads, flags and player indicators on and off inside the minimap.]]))
   ),
   h = 450,
}

congratulate_and_on_to_quarry = {
   title = _"Onward to the Quarry",
   body = li_object("greenland_rocks6",_[[Great. Now about that quarry…]]),
   h = 200,
   w = 250
}

-- ======
-- Quarry
-- ======

obj_build_a_quarry = {
   name = "build_a_quarry",
   title=_"Build a quarry next to the rocks",
   number = 1,
   body = objective_text(_"Build a Quarry",
      li(_[[There are some rocks to the west of your headquarters. Build a quarry right next to them.]]) ..
      li_image("images/wui/overlays/small.png", _[[The quarry is a small building like the lumberjack’s hut. You can therefore build it on any field that shows a red, yellow or green house when the building spaces symbols are enabled (Press Space for that).]]) ..
      li_arrow(_[[Just click on any house symbol next to the rocks, select the small buildings tab in the window that opens up, then click on the quarry symbol.]])
   ),
}
order_quarry_recap_how_to_build = {
   field = first_quarry_field,
   position = "topright",
   title = _"How to Build a Quarry",
   body = (
      li_object("barbarians_quarry",
         p(_[[Build a quarry next to those rocks here. Remember how I did it earlier?]]) ..
         p(_[[Make sure that you are showing the building spaces, then just click on the space where you want the building to be, choose it from the window that appears, and it is placed. Maybe this is a good time to explain about all those building space symbols we activated earlier.]]) ..
         p(_[[You can build four things on fields in Widelands: flags, small houses, medium houses and big houses. But not every field can hold everything. The build space symbols ease recognition:]]), plr.color) ..
      li_image("images/wui/overlays/big.png", _[[Everything can be built on the green house symbol.]]) ..
      li_image("images/wui/overlays/medium.png", _[[Everything except for big buildings can be built on a yellow house symbol.]]) ..
      li_image("images/wui/overlays/small.png", _[[Red building symbols can only hold small buildings and flags.]]) ..
      li_image("images/wui/overlays/set_flag.png", _[[And finally, the yellow flag symbol only allows for flags.]]) ..
      p(_[[If you place something on a field, the surrounding fields might have less space for holding buildings, so choose your fields wisely.]])
   )
}

local explain_abort_roadbuilding = li_image("images/wui/menu_abort.png", _[[If you decide you do not want to build a road at this time, you can cancel road building by clicking on the starting flag of the road and selecting the abort symbol.]])

talk_about_roadbuilding_00a = {
   position = "topright",
   field = wl.Game().map:get_field(9,12),
   title = _"Road Building",
   body = (
      li_image("images/wui/fieldaction/menu_tab_buildroad.png", _[[Excellent! Directly after placing the building, you have been switched into road building mode. The new road will start at the flag in front of your newly placed construction site.]]) ..
      li_image("images/wui/fieldaction/menu_build_way.png", _[[You can enter road building mode for any flag by left-clicking on a flag and selecting the road building symbol.]]) ..
      explain_abort_roadbuilding ..
      p(_[[Now, about this road. Remember: we are already in road building mode since you just ordered the quarry. You can either make it longer by one field at a time by left-clicking multiple times on neighboring fields for perfect control over the route the road takes, like so:]])
   ),
   h = 300,
   show_instantly = true
}

talk_about_roadbuilding_00b = {
   position = "topright",
   field = road_building_field,
   title = _"Road Building",
   body = (
      li_image("images/wui/fieldaction/menu_build_way.png", _[[ Excellent! To enter road building mode for any flag, left-click on a flag and select the road building symbol.]]) ..
      explain_abort_roadbuilding ..
      p(_[[Now, about this road. I’ll enter the road building mode and then make it longer by one field at a time by left-clicking multiple times on neighboring fields for perfect control over the route the road takes, like so:]])
   ),
   h = 300,
   show_instantly = true
}

talk_about_roadbuilding_01 = {
   position = "topright",
   field = road_building_field,
   title = _"Road Building",
   body = li_object("barbarians_flag", _[[Or, you can directly click the flag where the road should end, like so:]], plr.color),
   h = 200,
   w = 250
}

obj_build_road_to_quarry = {
   name = "build_road_to_quarry",
   title=_"Connect the quarry to the headquarters",
   number = 1,
   body = objective_text(_"Connect Your Construction Site",
      li(_[[Connect your quarry construction site to your headquarters with a road. You would have been put directly into road building mode after ordering a new site. But now, you aren’t.]]) ..
      li_arrow(_[[To build a completely new road, just click on the flag in front of your construction site, click on the build road icon and then click on the flag in front of your headquarters. Wait for the completion of the quarry.]]) ..
      li_arrow(_[[If you hold Ctrl or Shift+Ctrl while you finish the road, flags are placed automatically.]])
   ),
}
talk_about_roadbuilding_02 = {
   position = "topright",
   title = _"Road Building",
   body = (
      li_image("images/wui/fieldaction/menu_tab_buildroad.png", _[[One more thing: around the field where your road would end, you can see different markers. They have the following meaning:]]) ..
      li_image("images/wui/overlays/road_building_green.png", _[[The terrain is flat here. Your carriers will be very swift on this terrain.]]) ..
      li_image("images/wui/overlays/road_building_yellow.png", _[[There is a small slope to climb to reach this field. This means that your workers will be faster walking downhill than they will be walking uphill.]]) ..
      li_image("images/wui/overlays/road_building_red.png", _[[The connection between the fields is extremely steep. The speed increase in one direction is huge while the slowdown in the other is also substantial.]]) ..
      p(_[[Keep the slopes in mind while placing roads and use them to your advantage. Also, try to keep roads as short as possible and always remember to place as many flags as you can on road segments to share the load better.]])
   ),
   h = 450
}

quarry_not_connected = {
   title = _"Quarry not Connected",
   body = (
      li_object("barbarians_carrier", _[[Your workers do not like to walk across country. You have to build a road from your headquarters to the construction site so that carriers can transport wares. The simplest way is to click on the construction site’s flag, choose ‘Build road’, and then click on the destination flag (the one in front of your headquarters), just like I’ve demonstrated.]], plr.color)
   ),
   w = 350,
   h = 250
}

quarry_illegally_destroyed = {
   title = _"You Destroyed the Construction Site!",
   body = (
      li_object("barbarians_quarry", _[[It seems like you destroyed a construction site for a quarry we wanted to build. Luckily, we still have enough logs left this time, so you can simply build another one.]], plr.color) ..
      li_arrow(_[[You can also reload the game from a previous savegame. Luckily, these are created from time to time. To do so, you will have to go back to the main menu and choose ‘Single Player’ → ‘Load Game’. And please be a bit more careful next time.]])
   ),
   w = 350,
   h = 250
}

obj_build_the_second_quarry = {
   name = "build_the_second_quarry",
   title=_"Build another quarry",
   number = 1,
   body = objective_text(_"Build another quarry",
      li(_[[Build a second quarry near the rocks and connect it to your road network.]]) ..
      li_arrow(_[[You can connect the new road to any flag of your existing road network. You can create junctions everywhere, not only in front of buildings.]])
   ),
}
build_second_quarry = {
   position = "topright",
   title = _"Build a second quarry",
   body = (
      li_object("barbarians_quarry", _[[When there are many rocks, you can consider building another quarry. This will make the granite production faster.]], plr.color)
   ),
   h = 300
}

-- ===================
-- Census & Statistics
-- ===================

census_and_statistics_00 = {
   title = _"Census and Statistics",
   body = (
      li_image("images/wui/menus/toggle_census.png", _[[While we wait, I’ll quickly show you another useful feature. All construction sites look the same, and some buildings look alike. It is sometimes hard to tell them apart. Widelands offers a feature to show label texts over the buildings. They are called the ‘census’.]]) ..
      li_arrow(_[[In order to show or hide the building census labels, you can select the ‘Show Census’ / ‘Hide Census’ entry from the ‘Show / Hide’ menu on the bottom, or press the ‘c’ key on the keyboard.]]) ..
      p(_[[Let me enable the census for you.]])
   ),
   position = "topright",
   h = 300,
   w = 350
}

obj_show_statistics = {
   name = "show_statistics",
   title=_"Show the building statistics",
   number = 1,
   body = objective_text(_"Show the building statistics",
      li(_[[Show the building statistics labels, so that we can check the progress of our quarry construction more easily.]]) ..
      li_arrow(_[[In order to show or hide the building statistics labels, you can select the ‘Show Statistics’ entry from the ‘Show / Hide’ menu on the bottom, or press the ‘s’ key on the keyboard.]])
   )
}
census_and_statistics_01 = {
   title = _"Census and Statistics",
   body = (
      li_image("images/wui/menus/toggle_statistics.png",
         p(_[[Now, wouldn’t it be nice to check on our quarries’ progress without having to open their windows?]]) ..
         p(_[[In addition to the buildings’ census, you can also activate statistics labels on them. This will display information about the productivity of buildings or the progress of construction sites.]]))
   ),
   position = "topright",
   h = 400,
   w = 350
}

census_and_statistics_02 = {
   title = _"Census and Statistics",
   body = (
      li_object("barbarians_quarry", _[[Now we know what’s going on. Let’s wait for the quarries to finish.]], plr.color)
   ),
   position = "topright",
   h = 150,
   w = 250
}

-- ========
-- Messages
-- ========

obj_archive_all_messages = {
   name = "archive_all_messages",
   title=_"Archive all messages in your inbox",
   number = 1,
   body = objective_text(_"Archive Your Inbox Messages",
      li(_[[Archive all your messages in your inbox now.]]) ..
      li_image("images/wui/messages/message_archive.png", _[[Keep clicking the ‘Archive selected message’ button until all messages have been archived and the list is empty.]]) ..
      li_arrow(_[[Once you have archived a message, another message will be selected automatically from the list.]]) ..
      li_arrow(_[[You can also hold down the Ctrl or Shift key to select multiple messages, or press Ctrl + A to select them all.]]) ..
      li_arrow(_[[You can toggle the message window by pressing ‘n’ or clicking the second button from the right at the very bottom of the screen. The newest message will be marked for you automatically.]]) ..
      li_arrow(_[[The message window is central to fully controlling your tribe’s fortune. However, you will get a lot of messages in a real game. To keep your head straight, you should try to keep the inbox empty.]])
   )
}
teaching_about_messages = {
   popup = true,
   title = _"Messages",
   heading = _"Introducing Messages",
   body = (
      li_image("images/wui/menus/message_new.png",_[[Hi, it’s me again! This time, I have sent you a message. Messages are sent to you by Widelands to inform you about important events: empty mines, attacks on your tribe, won or lost military buildings, resources found…]]) ..
      p(_[[The message window can be toggled by the second button from the right at the bottom of the screen. This button will also change appearance whenever new messages are available, but there is also a bell sound played whenever you receive a new message.]]) ..
      p(_[[You have two messages at the moment. This one, which you are currently reading, and the one that informed you that a new headquarters was added to your economy. Let’s learn how to archive messages:]]) ..
      new_objectives(obj_archive_all_messages)
   )
}

obj_close_message_window = {
   name = "close_message_window",
   title=_"Close the messages window",
   number = 1,
   body = objective_text(_"Close the Messages Window",
      p(_[[All windows in Widelands can be closed by right-clicking into them. Some windows can also be toggled with the buttons and menus at the very bottom of the screen.]]) ..
      li(_[[Close the messages window now by right-clicking into it.]])
   )
}
closing_msg_window_00 = {
   position = "topright",
   field = first_quarry_field,
   title = _"Closing Windows",
   body = (
      li_image("images/wui/menus/message_old.png",_[[Excellent. Do you remember how to close windows? You simply have to right-click on them. This will work with all windows except for story message windows like this one. Go ahead and try it.]])
   ),
   h = 400,
   w = 350
}

closing_msg_window_01 = {
   position = "topright",
   field = first_quarry_field,
   title = _"Closing Windows",
   body = (
      li_object("barbarians_quarry", _[[Well done! Let’s see how messages work in a real game, shall we? For this, I’ll take all rocks away from the poor stonemasons in the quarries. They will then send a message each that they can’t find any in their work areas the next time they try to do some work.]], plr.color)
   ),
   h = 250,
   w = 350
}

obj_destroy_quarries = {
   name = "destroy_quarries",
   title=_"Destroy the two quarries",
   number = 1,
   body = objective_text(_"Destroy the Quarries",
      li(_[[Since our quarries are useless now, you can destroy them and reuse the space later on.]]) ..
      p(_[[There are two different ways of destroying a building: burning down and dismantling. Try them both out on your quarries.]]) ..
      li_image("images/wui/buildings/menu_bld_bulldoze.png", _[[Burning down the quarry: This is the fastest way of clearing the space. While the worker abandons the building, the wares are lost.]]) ..
      li_image("images/wui/buildings/menu_bld_dismantle.png", _[[Dismantling the quarry: A builder will walk from the headquarters to dismantle the quarry piece by piece. Thereby, you regain some of the resources you used for the construction.]])
   )
}
destroy_quarries_message = {
   position = "topright",
   title = _"Messages Arrived!",
   body = (
      li_image("images/wui/menus/message_new.png", _[[You received some messages. See how the button at the bottom of the screen has changed appearance?]])
   ),
   h = 400,
   w = 350
}

-- =========
-- Expansion
-- =========

obj_expand_territory = {
   name = "expand_territory",
   title=_"Expand your territory",
   number = 1,
   body = objective_text(_"Make your Territory Grow",
      li(_[[Build a military building on your border.]]) ..
      li_arrow(_[[In Widelands, it is necessary to build many buildings, which take up a lot of space. To expand your territory, you have to build military buildings next to your border. Every tribe has several military buildings.]]) ..
      li_arrow(_[[The Barbarians have four different military buildings you can build: the sentry (small), the barrier and the tower (both medium) and the fortress (big). Just choose the one you like most.]]) ..
      li_arrow(_[[The sentry is the only military site that fits on a small building plot. If your lumberjack has cleared enough space, you can also build another military building.]]) ..
      li_arrow(_[[Remember that big buildings (green icon) cannot be built on small (red) or medium (yellow) building plots, but buildings can be built on a building plot that provides more space than they need. You should always keep that in mind when you search for a suitable place.]])
   )
}
introduce_expansion = {
   title = _"Expanding Your Territory!",
   body = (
      li_object("barbarians_sentry",
         p(_[[There is one more thing I’d like to teach you now: Expanding your territory. The place that we started with around our headquarters is barely enough for a basic building infrastructure, and we do not have access to mountains, which we need to mine minerals and coal. So, we have to expand our territory.]]) ..
         p(_[[Expanding is as simple as building a military building at the edge of your territory. The Barbarians have a selection of different military buildings: sentries, barriers, towers, fortresses and citadels. The bigger the building, the more expensive it is to build, but the more land it will conquer around itself and the more soldiers can be stationed there. The buildings also vary in their vision range: buildings with a tower see farther than others.]]) ..
         p(_[[As soon as a military building is manned, it will extend your land. I will tell you more about military buildings in another tutorial.]]), plr.color)
   )
}

military_building_finished = {
   title = _"Military Site Occupied",
   body = (
      h1(_"Your territory has just grown!") ..
      li_object("barbarians_tower",
         p(_[[Great. Do you see how your territory has grown since your soldiers entered your new military building?]]) ..
         p(_[[Every military building has a certain conquer area – the more expensive the building, the more land it conquers.]]), plr.color)
   ),
   h = 300,
   w = 350
}

-- ==========
-- Conclusion
-- ==========

conclude_tutorial = {
   title = _"Conclusion",
   body = (
      h1(_"Conclusion") ..
      li_image("images/logos/wl-ico-64.png",
         p(_[[This concludes the first tutorial. In order to learn more about the game, I suggest to play one of the other tutorials. Each of them covers a different topic.]]) ..
         p(_[[However, since you now know how to control Widelands, you can also start a game (or continue this one) and discover more by yourself.]])) ..
      p(_[[To leave this game and return to the main menu:]]) ..
      li_image("images/wui/menus/main_menu.png", _[[Click on the ‘Main Menu’ button on the very left at the bottom of the screen.]]) ..
      li_image("images/wui/menus/exit.png", _[[Then click on the ‘Exit Game’ entry.]]) ..
      p(_[[Thanks for playing this tutorial. Enjoy Widelands and remember to visit us at]]) ..
      h1(p("align=center", u("widelands.org")))
   ),
   h = 450
}
