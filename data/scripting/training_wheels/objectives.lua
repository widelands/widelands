-- Welcome and teach objectives

include "scripting/coroutine.lua"
include "scripting/messages.lua"
include "scripting/richtext_scenarios.lua"
include "scripting/ui.lua"
include "scripting/training_wheels/utils/lock.lua"
include "scripting/training_wheels/utils/ui.lua"

local training_wheel_name = training_wheel_name_from_filename(__file__)

run(function()
   sleep(10)

   local mapview = wl.ui.MapView()
   local player = wl.Game().players[wl.Game().interactive_player]
   wait_for_lock(player, training_wheel_name)

   push_textdomain("training_wheels")

   local objectives_message = {
      title = _"Objectives",
      position = "bottomright",
      h = 280,
      w = 260,
      modal = false,
      body = (
         -- No intro text, the objective itself is enough
         ""
      )
   }

   local objectives_message_objective = {
   name = "objectives_message_objective",
      title = _"Objectives",
      number = 1,
      body = (
         objective_text(_"Open the Objectives window",
         li_image("images/wui/training_wheels_arrow.png", _"Click on the button that the arrow is pointing to, or press ‘%s’ to see the current game's objectives.":bformat('o')))
      )
   }

   local obj_initial_close_objectives_window = {
      name = "obj_initial_close_objectives_window",
      title=_"Close this window",
      number = 1,
      body = objective_text(_"Closing this window",
         li_image("images/wui/menus/objectives.png", _[[ You can open and close this window by clicking on the ‘Objectives’ button in the toolbar on the bottom of the screen.]]) ..
         li_arrow(_[[Like any other window, you can also close the ‘Objectives’ window by right-clicking on it.]])
      )
   }

   pop_textdomain()

   -- TODO(GunChleoc): This fails when the objective is written and the game saved before completing it, and then the game loaded again.
   -- We need to add the active training wheel to the savegame data so we won't run it twice.

   local o2 = add_campaign_objective(obj_initial_close_objectives_window)

   mapview.buttons.objectives:indicate(true)

   local o1 = campaign_message_with_objective(objectives_message, objectives_message_objective, 0)
   set_objective_done(o1)

   while not mapview.windows.objectives do sleep(100) end

   close_story_messagebox()
   mapview.buttons.objectives:indicate(false)
   sleep (100)

   while mapview.windows.objectives do sleep(100) end
   set_objective_done(o2)

   player:mark_training_wheel_as_solved(training_wheel_name)
end)
