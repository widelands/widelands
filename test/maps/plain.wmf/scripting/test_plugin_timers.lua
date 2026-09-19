run(function()
   game.desired_speed = 1000
   sleep(2000)

   mv = wl.ui.MapView()
   assert_equal(0, #mv.plugin_timers)
   assert_nil(mv:get_plugin_timer("timer_action_one"))

   g_timer_one_counter = 1000
   function timer_action_1()
      g_timer_one_counter = g_timer_one_counter + 1
   end

   local timer1 = mv:add_plugin_timer("timer_action_1()", 500, "timer_action_one", 3)
   local timer2 = mv:add_plugin_timer("this lua snippet would raise a syntax error", 1000, "timer_action_two", 0, false)
   timer2.active = false

   assert_equal(2, #mv.plugin_timers)

   assert_equal(3, timer1.remaining_count)
   assert_true(timer1.active)
   assert_equal(1000, g_timer_one_counter)
   sleep(5000)
   assert_equal(0, timer1.remaining_count)
   assert_false(timer1.active)
   assert_equal(1003, g_timer_one_counter)

   assert_not_nil(mv:get_plugin_timer("timer_action_one"))
   assert_equal(500, mv:get_plugin_timer("timer_action_one").interval)

   local removed = mv:remove_plugin_timer("helloworld")
   assert_equal(0, removed)
   assert_equal(2, #mv.plugin_timers)
   removed = mv:remove_plugin_timer("timer_action_two", true)
   assert_equal(1, removed)
   assert_equal(1, #mv.plugin_timers)

   timer1.remaining_count = 8
   timer1.active = true
   sleep(10000)
   assert_equal(0, timer1.remaining_count)
   assert_false(timer1.active)
   assert_equal(1011, g_timer_one_counter)
   removed = mv:remove_plugin_timer()
   assert_equal(1, removed)
   assert_equal(0, #mv.plugin_timers)

   assert_error("can't remove one arbitrary timer", function() mv:remove_plugin_timer(nil, true) end)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
