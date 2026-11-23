-- =================
-- Helper functions
-- =================

-- Check if player seen any field of given array of fields
function any_field_seen(player, fields)
   for k,v in pairs(fields) do
      if player:seen_field(v) then
         return true
      end
   end
   return false
end

-- Try to ensure that only one of the given building type can exist.
-- Does not prevent starting multiple constructions before the first one is finished.
function wait_build_one(player, building_name)
   local sites = {}
   while #sites == 0 do
      sleep(500)
      sites = player:get_buildings(building_name)
   end
   sites[1].destruction_blocked = true
   player:forbid_buildings({building_name})
   return sites[1]
end

-- Count heroes in sentries
function heroes_in_sentries(player)
   local n = 0
   for i,b in ipairs(player:get_buildings("barbarians_sentry")) do
      n = n + b:get_soldiers({3,5,0,2})
   end
   return n
end

-- Fill the given buildings' inputs to maximum capacity
function fill_inputs(buildings)
   for i,b in ipairs(buildings) do
      local max = b.valid_inputs
      for ware,num in pairs(max) do
         b:set_desired_fill(ware, num)
      end
      b:set_inputs(max)
   end
end

-- Check that the trainingsite windows are open and the soldiers tabs are active
function check_training_windows(buildings)
   local nr_ok = 0
   for i,b in ipairs(buildings) do
      local window = mv.windows[string.bformat("building_window_%u", b.serial)]
      if window and window.tabs.soldiers.active then
         nr_ok = nr_ok + 1
      end
   end
   return nr_ok == #buildings
end

-- Check that the trainingsites have the desired preferences
function check_training_prefs(buildings, pref)
   if pref == nil then
      return true
   end

   local nr_ok = 0
   for i,b in ipairs(buildings) do
      if b.soldier_preference == pref then
         nr_ok = nr_ok + 1
      end
   end
   return nr_ok == #buildings
end

-- Check that the trainingsites have enough soldiers
function check_training_caps(buildings)
   local nr_ok = 0
   for i,b in ipairs(buildings) do
      if b.capacity == 4 then
         nr_ok = nr_ok + 1
      end
   end
   return nr_ok == #buildings
end
