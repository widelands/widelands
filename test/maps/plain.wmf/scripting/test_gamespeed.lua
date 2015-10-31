run(function()
   sleep(1000)

   game.desired_speed = 5000
   assert_equal(game.real_speed, 5000)
   assert_equal(game.desired_speed, 5000)
   assert_equal(game.real_speed, 5000)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)