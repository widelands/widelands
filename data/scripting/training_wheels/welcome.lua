-- Welcome and teach the arrow

include "scripting/coroutine.lua"
include "scripting/messages.lua"
include "scripting/richtext_scenarios.lua"
include "scripting/ui.lua"
include "scripting/training_wheels/utils/lock.lua"
include "scripting/training_wheels/utils/ui.lua"

local training_wheel_name = training_wheel_name_from_filename(__file__)

run(function()
   sleep(10)
   local player = get_interactive_player()
   wait_for_lock(player, training_wheel_name)

   push_textdomain("training_wheels")

   local welcome_message = {
      title = _("Welcome to Widelands!"),
      h = 180,
      w = 360,
      body = (
         li_image("images/logos/wl-ico-64.png", h1(_("Welcome to Widelands!"))) ..
         li_image("images/wui/training_wheels_arrow.png", _("Follow the arrows to learn how to play.")) ..
         li(_("Click ‘OK’ to continue."))
      )
   }

   pop_textdomain()

   campaign_message_box(welcome_message, 100)
   player:mark_training_wheel_as_solved(training_wheel_name)
end)
