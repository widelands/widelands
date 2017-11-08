include "scripting/coroutine.lua"
include "scripting/lunit.lua"

game = wl.Game()
map = game.map


-- Save the game so that reloading does not skip
function stable_save(safename)
   local old_speed = game.desired_speed
   game.desired_speed = 1000
   sleep(100)
   game:save(safename)
   game.desired_speed = 1000
   sleep(2000)  -- Give the loaded game a chance to catch up
   game.desired_speed = old_speed
   sleep(1000)
end
