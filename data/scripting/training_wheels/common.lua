-- Common objectives used by more than one training wheel.
-- Placed in this directory so we can pick up te translations.

include "scripting/coroutine.lua"
include "scripting/messages.lua"
include "scripting/richtext_scenarios.lua"
include "scripting/ui.lua"
include "scripting/training_wheels/utils/lock.lua"
include "scripting/training_wheels/utils/ui.lua"

function teach_flags_on_road(target_field)
   -- Define our messages
   push_textdomain("training_wheels")

   local msg_click_flag_symbol = {
      title = _"Flags",
      position = "topright",
      body = (
         p(_"Your carriers can transport your wares faster if they share the load.") ..
         li_image("images/wui/overlays/set_flag.png", _"Click on the flag symbol.")
      ),
      h = 120,
      w = 260,
      modal = false
   }

   local msg_click_flag_button = {
      title = _"Flags",
      position = "topright",
      body = (
         li_image("images/wui/fieldaction/menu_build_flag.png", _"Click on the flag button.")
      ),
      h = 120,
      w = 260,
      modal = false
   }

   local msg_click_more_flags = {
      title = _"Flags",
      position = "topright",
      body = (
         li_image("images/wui/overlays/set_flag.png", _"Now check if you can place even more flags on your roads.") ..
         p("Click ‘OK’ to continue.")
      ),
      h = 120,
      w = 260,
      modal = false
   }


   pop_textdomain()

   local mapview = wl.ui.MapView()

   -- Force buildhelp so we can show the flag position
   mapview.buildhelp = true
   target_field:indicate(true)
   campaign_message_box(msg_click_flag_symbol)

   wait_for_field_action_tab("roads")
   while not mapview.windows.field_action.tabs["roads"].active do
      sleep(100)
      if not mapview.windows.field_action.tabs["roads"].active then
         mapview.windows.field_action.tabs["roads"]:indicate(true)
      end
      wait_for_field_action_tab("roads")
   end
   target_field:indicate(false)
   mapview.windows.field_action.buttons["build_flag"]:indicate(true)
   close_story_messagebox()
   campaign_message_box(msg_click_flag_button)

   while mapview.windows.field_action do sleep(100) end
   mapview:indicate(false)

   close_story_messagebox()
   campaign_message_box(msg_click_more_flags)
end
