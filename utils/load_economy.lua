-- This file can be used inside a debug build of widelands or a scenario. It will load  the
-- previously dumped economy of one player (utils/dump_economy.lua) which can be used in scenarios. The
-- state will be as complete as possible given the limitations of the current
-- state of the Lua engine.
--
-- Usage:
-- Press CTRL + SHIFT + Space to open the debug console (need a debug build for that).
-- dofile("utils/load_economy.lua")
-- load_eco(plrno, tribename, folder_of_the_file)
--


include "scripting/set.lua"

local game = wl.Game()
map = game.map


function place_flags(eco, plr)

   for i = 1, #eco.flags do
      local flag = plr:place_flag(map:get_field(eco.flags[i].x, eco.flags[i].y),true)
      for ware,count in pairs(eco.flags[i].wares) do
         flag:set_wares(ware,count)
      end
   end
end

function place_roads(eco, plr)
   for i = 1, #eco.roads do
      local command = string.format("road = plr:place_road(map:get_field(%i,%i).immovable, %s ,true)", eco.roads[i].x, eco.roads[i].y, eco.roads[i].dirs)
      local f = assert (load (command))
      f()
      -- Second carrier can't be placed currently, so we just fake a carrier 
      worker = (plr.tribe_name.."_carrier")
      road:set_workers(worker,1)
   end
end

function place_buildings(eco, plr)

   for i = 1, #eco.buildings do
      building = plr:place_building(eco.buildings[i].name, map:get_field(eco.buildings[i].x, eco.buildings[i].y), false, true)

      if eco.buildings[i].inputs then
         for ware,count in pairs(eco.buildings[i].inputs) do
            building:set_inputs(ware,count)
         end
      end

      if eco.buildings[i].workers then
         local up = {}
         for worker,count in pairs(eco.buildings[i].workers) do
            if count == 0 then
               up[#up+1] = worker
            elseif count > 1 and #up > 0 and not building.descr.type_name == "warehouse" then
               for i = 1, count - 1 do 
                  building:set_workers(up[i],1)
               end
               building:set_workers(worker,1)
            else
               building:set_workers(worker,1)
            end
         end
      end

      if eco.buildings[i].wares then
         for ware,count in pairs(eco.buildings[i].wares) do
            building:set_wares(ware,count)
         end
      end

      if eco.buildings[i].soldiers then
         for soldier,count in pairs(eco.buildings[i].soldiers) do
            building:set_soldiers({soldier:sub(1,1),soldier:sub(2,2),soldier:sub(3,3),soldier:sub(4,4)}, count)
         end
      end
   end

end

function load_eco(plrno, tribe, folder)
   plr = game.players[plrno]
   local file = ("ecodump_player"..plrno.."-"..tribe)
   local eco = game:read_campaign_data(folder, file)
   -- hq = (tribe.."_headquarters")
   -- plr:place_building(hq, map.player_slots[plrno].starting_field, false, true)

   place_flags(eco, plr)
   place_roads(eco, plr)
   place_buildings(eco, plr)

end
