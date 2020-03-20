include "scripting/coroutine.lua"

-- Save the game so that reloading does not skip
function stable_save(game, savename, desired_speed)
   local mapview = wl.ui.MapView()

   game.desired_speed = 1000
   sleep(100)
   game:save(savename)
   game.desired_speed = 1000

   -- Give the loaded game a chance to catch up
   sleep(200)
   local counter = 0
   while mapview.average_fps < 20 and counter < 20 do
      sleep(500)
      counter = counter + 1
   end

   game.desired_speed = desired_speed
   sleep(1000)
end
