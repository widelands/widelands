include "scripting/coroutine.lua"

-- Save the game so that reloading does not skip
function stable_save(game, savename)
   local mapview = wl.ui.MapView()
   local old_speed = game.desired_speed
   game.desired_speed = 1000
   sleep(100)
   game:save(savename)
   game.desired_speed = 1000
     -- Give the loaded game a chance to catch up
   sleep(200)
   while mapview.average_fps < 20 do sleep(500) end
   sleep(1000)
   game.desired_speed = old_speed
   sleep(1000)
end
