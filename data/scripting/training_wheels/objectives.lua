-- Welcome and teach objectives

-- NOCOM document
include "scripting/coroutine.lua"
include "scripting/messages.lua"
include "scripting/richtext_scenarios.lua"
include "scripting/ui.lua"

run(function()
   sleep(200)

   local mapview = wl.ui.MapView()

   push_textdomain("training_wheels")

   local welcome_message = {
      title = _"Welcome to Widelands!",
      h = 160,
      w = 360,
      body = (
         li_image("images/logos/wl-ico-64.png", h1(_"Welcome to Widelands!")) ..
         li_image("images/wui/training_wheels_arrow.png", _"Follow the arrows to learn how to play.")
      )
   }

   local objectives_message = {
      title = _"Objectives",
      position = "bottomright",
      h = 280,
      w = 260,
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
         li_image("images/wui/training_wheels_arrow.png", _"Click on the button that the arrow is pointing to, or press ‘%s’ to see the current game's objectives":bformat('o')) ..
         li(_"You'll need to close this message window first."))
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

   local o2 = add_campaign_objective(obj_initial_close_objectives_window)

   campaign_message_box(welcome_message, 100)

   mapview.buttons.objectives:indicate(true)

   local o1 = campaign_message_with_objective(objectives_message, objectives_message_objective, 0)
   set_objective_done(o1)

   while not mapview.windows.objectives do sleep(100) end
   sleep (100)
   mapview.buttons.objectives:indicate(false)
   while mapview.windows.objectives do sleep(100) end
   set_objective_done(o2)

   wl.Game().players[1]:mark_training_wheel_as_solved("objectives")
end)
