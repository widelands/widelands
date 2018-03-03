include "scripting/coroutine.lua"

-- Save the game so that reloading does not skip
function stable_save(game, savename)
   local old_speed = game.desired_speed
   game.desired_speed = 1000
   sleep(100)
   game:save(savename)
   game.desired_speed = 1000
   sleep(2000)  -- Give the loaded game a chance to catch up
   game.desired_speed = old_speed
   sleep(1000)
end
