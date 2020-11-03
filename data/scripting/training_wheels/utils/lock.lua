-- TODO(GunChleoc): Document more fully

-- Sleep until we acquire permission to run
function wait_for_lock(player, training_wheel_name)
   local success = false
   repeat
      success = player:acquire_training_wheel_lock(training_wheel_name)
      if not success then
         sleep(1000)
      end
   until success == true
   -- Safeguard against concurrent locking, just in case
   sleep(10)
   if not player:acquire_training_wheel_lock(training_wheel_name) then
      wait_for_lock(player, training_wheel_name)
   end
end

-- Strip directory and '.lua' file extension
function training_wheel_name_from_filename(filename)
   local basename = path.basename(filename)
   return basename:sub(0, #basename - 4)
end

function run_training_wheel_in_scenario(player, name, force)
   player:run_training_wheel(name, force)
   wait_for_lock(player, "ficticious_training_wheel_name")
   player:release_training_wheel_lock()
end

-- Detect the interactive player if any
function get_interactive_player()
   local player_number = wl.Game().interactive_player
   for p_idx, player in ipairs(wl.Game().players) do
      if player.number == player_number then
         return player
      end
   end
   return nil
end
