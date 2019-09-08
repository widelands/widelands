-- Helper functions for campaign emp04

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
      p1:get_buildings("empire_headquarters"),
      p1:get_buildings("empire_warehouse"),
      p1:get_buildings("empire_port")
   )
   local rv = 0
   for idx,wh in ipairs(whs) do
      rv = rv + wh:get_wares(ware)
   end
   return rv
end

function enemy_seen()
   for x=32, 90 do
      for y=20, 175 do
         local field = map:get_field(x,y)
         if field.owner == p2 and p1:sees_field(field) then
         return field
        end
      end
   end
end
