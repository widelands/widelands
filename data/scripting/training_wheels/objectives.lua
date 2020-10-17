-- NOOCM document
include "scripting/coroutine.lua"
include "scripting/messages.lua"
include "scripting/richtext_scenarios.lua"
include "scripting/ui.lua"

run(function()
   sleep(2000)
   -- NOCOM push_textdomain("scenario_tutorial01_basic_control.wmf")
   print("This is objectives.lua")
   local close_story_window_instructions = _[[Click on the ‘OK’ button or press the ‘Enter ⏎’ key on the keyboard to close this window.]]

   -- Welcome and teach objectives
   local obj_initial_close_story_window = {
      name = "initial_close_story_window",
      title=_"Close this window",
      number = 1,
      body = objective_text(_"Close this window",
         li(close_story_window_instructions)
      )
   }

   local initial_message_01 = {
      title = _"Welcome to the Widelands Tutorial!",
      body = (
         h1(_"Welcome to Widelands!") ..
         li_image("images/logos/wl-ico-64.png",
            _[[Widelands is a slow-paced build-up strategy game with an emphasis on construction rather than destruction. This tutorial will guide you through the basics of the game.]]) ..
         li_arrow(_[[You will be guided through this tutorial by objectives]]) ..
         li(_[[Follow the intructions in the objective below so that I can show you where to find them.]])
      )
   }

   local obj_initial_close_objectives_window = {
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

   -- NOCOM pop_textdomain()

   local objective_to_explain_objectives = add_campaign_objective(obj_initial_close_objectives_window)
   local o = campaign_message_with_objective(initial_message_01, obj_initial_close_story_window)
   set_objective_done(o, 100)

   wl.ui.MapView().buttons.objectives:click()

   sleep(5000)

   wl.ui.MapView().buttons.objectives:indicate(true)

   while not wl.ui.MapView().windows.objectives do sleep(100) end
   while wl.ui.MapView().windows.objectives do sleep(100) end

   wl.ui.MapView().buttons.objectives:indicate(false)

   set_objective_done(objective_to_explain_objectives)

   wl.Game().players[1]:mark_training_wheel_as_solved("objectives")
end)
