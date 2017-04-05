-- Helper functions for campaign emp03

function count_buildings(plr, tbl)
   -- return overall amount of buildings:
   -- plr : Player to count for
   -- tbl : Table in form of {"building_name1", "building_name2", ...}
   local amount = 0
   for idx, bldng in ipairs(tbl) do
      amount = amount + #plr:get_buildings(bldng)
   end
   return amount
end

function count_in_warehouses(ware)
   local whs = array_combine(
      p1:get_buildings("empire_headquarters_shipwreck"),
      p1:get_buildings("empire_warehouse"),
      p1:get_buildings("empire_port")
   )
   local rv = 0
   for idx,wh in ipairs(whs) do
      rv = rv + wh:get_wares(ware)
   end
   return rv
end

function concentric_reveal(plr, center, max_radius, delay)
   if not delay then delay = 100 end
   local steps = 0
   while steps < max_radius do
      plr:reveal_fields(center:region(steps))
      steps = steps + 1
      sleep(delay)
   end
end

function concentric_hide(plr, center, max_radius, delay)
   if not delay then delay = 100 end
   while max_radius > 0 do
      local to_hide = center:region(max_radius, max_radius - 1)
      plr:hide_fields(to_hide, true)
      sleep(delay)
      max_radius = max_radius -1
   end
   -- Hide the remaining field
   plr:hide_fields({center},true)
end

function random_reveal(plr, region, time)
   -- if no time is given the default '1000' (1 sec) is used
   if not time then time = 1000 end
   -- Calculate the sleep as integer
   delay = math.floor(time / #region)
   -- Create a table with randomized fields
   -- This is done for efficience reasons
   rand_tbl = {}
   while #region > 0 do
      f = math.random(1, #region)
      table.insert(rand_tbl, region[f])
      table.remove(region, f)
   end
   --reveal field by field:
   for i, f in ipairs(rand_tbl) do
      plr:reveal_fields({f})
      sleep(delay)
   end
end

function random_hide(plr, region, time)
   if not time then time = 1000 end
   delay = math.floor(time / #region)
   while #region > 0 do
      f = math.random(1, #region)
      plr:hide_fields({region[f]}, true)
      table.remove(region, f)
      sleep(delay)
   end
end

