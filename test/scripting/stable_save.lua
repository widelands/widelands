include "scripting/coroutine.lua"

local saved_version = get_build_id()

-- Save the game so that reloading does not skip
function stable_save(game, savename, desired_speed)
   local last_save_time = game.last_save_time
   print("###### stable_save: last save time " .. last_save_time)

   game.desired_speed = 1000
   if lunit and lunit.stats and lunit.stats.run and
         lunit.stats.run + lunit.stats.notrun < lunit.stats.tests then
      -- lunit is currently running (it is loaded and started running and did not not finish)
      print("WARNING: a testcase is probably running while saving. This might fail!")
   end

   sleep(1000)

   local tries = 0
   local saved_game = false

   while true do
      if game.allow_saving then
         game:save(savename)

         game.desired_speed = 1000

         -- Wait until save was finished
         repeat
            sleep(200)
         until game.last_save_time ~= last_save_time
         print("###### stable_save: new save time " .. game.last_save_time)

         saved_game = true
         break
      else
         tries = tries + 1
         if tries < 5 then
            print("Saving game is not allowed, retry after 1s...")
            sleep(1000)
         else
            break
         end
      end
   end

   if saved_game then
      sleep(1000)

      -- Give the loaded game a chance to catch up
      local mapview = wl.ui.MapView()
      local counter = 0
      while mapview.average_fps < 20 and counter < 100 do
         sleep(200)
         counter = counter + 1
      end

      sleep(1000)

   else
      print("WARNING: stable_save() skipped because saving the game is not allowed!")
   end

   game.desired_speed = desired_speed
   sleep(100)
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
