include "scripting/coroutine.lua"

-- Save the game so that reloading does not skip
function stable_save(game, savename, desired_speed)
   local mapview = wl.ui.MapView()
   local last_save_time = game.last_save_time
   print("###### stable_save: last save time " .. last_save_time)

   game.desired_speed = 1000
   sleep(1000)
   game:save(savename)
   game.desired_speed = 1000

   -- Wait until save was finished
   repeat
      sleep(200)
   until game.last_save_time ~= last_save_time
   print("###### stable_save: new save time " .. game.last_save_time)

   -- Give the loaded game a chance to catch up
   local counter = 0
   while mapview.average_fps < 20 and counter < 100 do
      sleep(200)
      counter = counter + 1
   end

   sleep(1000)
   game.desired_speed = desired_speed
   sleep(100)
end
