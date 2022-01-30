-- Activate the building spaces

include "scripting/coroutine.lua"
include "scripting/messages.lua"
include "scripting/richtext_scenarios.lua"
include "scripting/ui.lua"
include "scripting/training_wheels/utils/lock.lua"
include "scripting/training_wheels/utils/ui.lua"

local training_wheel_name = training_wheel_name_from_filename(__file__)

local solved = false

run(function()
   sleep(10)

   local mapview = wl.ui.MapView()
   local player = get_interactive_player()
   while mapview.buildhelp do sleep(100) end
   wait_for_lock(player, training_wheel_name)

   push_textdomain("training_wheels")

   local msg_open_menu = {
      title = _("Building Spaces"),
      position = "bottomright",
      h = 180,
      w = 260,
      modal = false,
      body = (
         li_image("images/wui/menus/showhide.png", _("We will want to place buildings so that we can produce wares.")) ..
         p(_("So, let’s find out where we have space for them.")) ..
         li_image("images/wui/training_wheels_arrow.png", _("Click on the ‘Show / Hide’ menu."))
      )
   }

   local msg_select_item = {
      title = _("Building Spaces"),
      position = "topright",
      h = 120,
      w = 260,
      modal = false,
      body = (
         li_image("images/wui/menus/toggle_buildhelp.png", _("Select ‘Show Building Spaces’."))
      )
   }

   local msg_finished = {
      title = _("Building Spaces"),
      position = "topright",
      h = 160,
      w = 260,
      body = (
         li_image("images/wui/overlays/small.png", _("Now we can see where it is possible to place buildings and flags.")) ..
         li(_("Click ‘OK’ to continue."))
      )
   }

   pop_textdomain()

   campaign_message_box(msg_open_menu)

   -- Check whether buildhelp is on in separate thread so the player can complete this via keyboard shortcut
   run(function()
      while not mapview.buildhelp do sleep(200) end
      solved = true
   end)

   mapview.dropdowns["dropdown_menu_showhide"]:indicate(true)

   while not mapview.dropdowns["dropdown_menu_showhide"].expanded and not solved do sleep(200) end

   if not solved then
      -- Player did not use keyboard shortcut, continue explaining
      close_story_messagebox()
      campaign_message_box(msg_select_item)

      mapview.dropdowns["dropdown_menu_showhide"]:indicate_item(1)

      -- Wait for buildhelp to come on
      while not solved do sleep(200) end
   end

   mapview:indicate(false)

   close_story_messagebox()
   campaign_message_box(msg_finished)

   -- We might still have some indicators and messages boxes left over from unexpected player actions
   clean_up_message_boxes_and_indicators()

   player:mark_training_wheel_as_solved(training_wheel_name)
end)
