-- Helper functions for campaign emp03

function get_artifact_fields()
   -- Returns a table of all map fields that contain an artifact
   local artifact_fields = {}
   for x=0, map.width-1 do
      for y=0, map.height-1 do
         local field = map:get_field(x,y)
         if field.immovable and field.immovable:has_attribute("artifact") then
            -- This assumes that the immovable has size small or medium, i.e. only occupies one field
            table.insert(artifact_fields, map:get_field(x,y))
         end
      end
   end
   return artifact_fields
end

function count_buildings_by_name(plr, tbl)
   -- return overall amount of buildings:
   -- plr : Player to count for
   -- tbl : Table in form of {"building_name1", "building_name2", ...}
   local amount = 0
   for idx, bldng in ipairs(tbl) do
      amount = amount + #plr:get_buildings(bldng)
   end
   return amount
end

function count_buildings(plr, buildings)
   -- return overall amount of buildings:
   -- plr : Player to count for
   -- tbl : Table consisting of building description objects
   local amount = 0
   for idx, building in pairs(p1.tribe.buildings) do
      amount = amount + #plr:get_buildings(building.name)
   end
   return amount
end

function check_trainingsites()
   local training = p1:get_buildings {
      "empire_trainingcamp",
      "empire_barracks",
      "empire_arena",
      "empire_colosseum"
   }
   return
      #training.empire_trainingcamp > 0 and
      #training.empire_barracks > 0 and
      (#training.empire_arena + #training.empire_colosseum > 0)
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
