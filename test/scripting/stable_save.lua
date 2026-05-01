include "scripting/coroutine.lua"

local saved_version = get_build_id()

-- Save the game so that reloading does not skip
function stable_save(game, savename, desired_speed, desired_wait_time)
   local last_save_gametime = game.last_save_gametime
   local initial_gametime = game.time
   print("###### stable_save: last save gametime " .. last_save_gametime .. ", realtime " .. game.last_save_time)

   game.desired_speed = 1000
   if lunit and lunit.stats and lunit.stats.run and
         lunit.stats.run + lunit.stats.notrun < lunit.stats.tests then
      -- lunit is currently running (it is loaded and started running and did not not finish)
      print("WARNING: a testcase is probably running while saving. This might fail!")
   end

   sleep(1000)

   if not game.allow_saving then
     error("stable_save() would hang if saving is not allowed")
   end

   game:save(savename)
   game.desired_speed = 1000

   -- Wait until save was finished and give the game some time to catch up
   wake_me(initial_gametime + (desired_wait_time or 10000))
   while game.last_save_gametime == last_save_gametime do
      print("WARNING: Game save was delayed! Adding extra sleep time")
      sleep(1000)
   end
   print("###### stable_save: new save gametime " .. game.last_save_gametime .. ", realtime " .. game.last_save_time)

   game.desired_speed = desired_speed
   sleep(100)
   print("stable_save: save took " .. (game.time - initial_gametime) .. " ms gametime, FPS is now " .. wl.ui.MapView().average_fps)
end

function same_version()
   return saved_version == get_build_id()
end

function check_reload_version()
   if same_version() then
      print("# All Tests passed.")
      wl.ui.MapView():close()
   else
      print("Game was saved with a different version, not quitting.")
   end
end
