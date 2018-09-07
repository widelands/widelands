-- Test custom scenario hooks

run(function()
   local hook_was_triggered = false

   wl.Game():add_scenario_hook("custom_scenario_hook")
   -- Instantiate scenario hook
   hooks.custom_scenario_hook = {
      hook_function = function()
         print("Scenario hook was triggered")
         hook_was_triggered = true
         return
      end,
   }

   assert_false(hook_was_triggered)

   -- Sleep for longer than Widelands::kStatisticsSampleTime
   sleep(30010)

   assert(hook_was_triggered)

   print("Done.")
   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
