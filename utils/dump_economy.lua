-- This file can be used inside a debug build of widelands or be included in a scenario. It will dump the
-- economy of one player as a campaign data file (.wcd) which can be used in scenarios. The
-- state will be as complete as possible given the limitations of the current
-- state of the Lua engine.
--
-- Usage:
-- Press CTRL + SHIFT + Space to open the debug console (need a debug build for that).
-- dofile("utils/dump_economy.lua")
-- dump_eco(plrnumber, mapname)

-- arg:mapname 
--    is a string for the folder in .widelands/campaigns/ where the file will be stored
--    it should be chosen according the map/scenario
--    the name of the file will be e.g. ecodump_player1_tribename.wcd

--
-- Note that constructionsites will be completely ignored.
--
-- The HQ flag of a player will be the base for the graph traversal used. Therefore if he has more
-- than one economy, this script will only dump one of it.

include "scripting/set.lua"

local game = wl.Game()
local map = game.map
local rv = {
   details = {},
   flags = {},
   roads = {},
   buildings = {}
}

function traverse_economy(plr, flag)

   rv.details[1] = {playerno = plr.number, tribe = plr.tribe_name}
   local roads_done = Set:new{}
   local flags_done = Set:new{}
   local buildings_done = Set:new{}
   local flags_to_check = Set:new{flag}
   
   local _find_directions = function(r)
      local lf = nil
      local f = r.start_flag.fields[1]
      local steps = {}
      local _step = function (d)
         steps[#steps+1] = ("[[%s]]"):format(d)
         f, lf = f[d..'n'], f
      end
      local flag_found = false
      while not flag_found do
         for idx,nn in ipairs{"r", "br", "bl", "l", "tl", "tr"} do
            local n = f[nn .. "n"]
            if n.immovable == r and lf ~= n then
               _step(nn)
               break
            elseif n.immovable == r.end_flag then
               _step(nn)
               flag_found = true
               break
            end
         end
      end
      return steps
   end

   local _handle_flag = function (f)
      if flags_done:contains(f) then return end
      rv.flags[#rv.flags + 1] = {x = f.fields[1].x, y = f.fields[1].y, wares = f:get_wares("all")}
      flags_to_check:add(f)
   end

   local _discover_road = function (r)
      if roads_done:contains(r) then return end
      rv.roads[#rv.roads + 1] = {x = r.start_flag.fields[1].x, y = r.start_flag.fields[1].y, dirs = table.concat(_find_directions(r), ','), workers = r:get_workers("all")}
      roads_done:add(r)
   end

   local _discover_building = function (b)
      if buildings_done:contains(b.fields[1]) then return end
      local _inputs = {}
      local _wares = {}
      local _soldiers = {}
      local _workers = {}

      if b.get_inputs then
         _inputs = b:get_inputs("all")
      end

      if b.get_wares then
         _wares = b:get_wares("all")
      end

      if b.get_workers then
         _workers = b:get_workers("all")
      end

      if b.get_soldiers then
         for descr,n in pairs(b:get_soldiers("all")) do
            _soldiers[descr[1] .. descr[2] .. descr[3].. descr[4]] = n
         end
      end

      rv.buildings[#rv.buildings + 1] = {name = b.descr.name, x = b.fields[1].x, y = b.fields[1].y, wares = _wares, inputs = _inputs, workers = _workers, soldiers = _soldiers}
      buildings_done:add(b.fields[1])
   end

   while flags_to_check.size > 0 do
      local flag = flags_to_check:pop_at(1)
      local f = flag.fields[1]

      flags_done:add(flag)

      for idx,n in ipairs{f.rn, f.brn, f.bln, f.ln, f.tln, f.trn} do
         if n.immovable and n.immovable.owner == plr then
            if n.immovable.descr.type_name == "road" then
               local r = n.immovable
               if r.start_flag == flag then
                  _handle_flag(r.end_flag)
                  _discover_road(r)
               elseif r.end_flag == flag then
                  _handle_flag(r.start_flag)
                  _discover_road(r)
               end
            elseif n.immovable.descr.type_name == "constructionsite" or n.immovable.descr.type_name == "dismantlesite" then
               print(("IGNORING constructionsite/dismantlesite at %s"):format(tostring(n)))
            elseif n.immovable.descr.type_name:sub(-4) == "site" or n.immovable.descr.type_name == "warehouse" then
               _discover_building(n.immovable)
            else
               print("UNKNOWN immovable type: ", n.immovable.type)
               assert(nil)
            end
         end
      end
   end

end



function dump_eco(plrno, mapname)

   local plr = game.players[plrno]
   local start_flag = map.player_slots[plrno].starting_field.brn.immovable
   assert(start_flag.descr.type_name == "flag")

   traverse_economy(plr, start_flag)
   local folder = mapname
   local file = ("ecodump_player"..plrno.."-"..plr.tribe_name)
   game:save_campaign_data(folder, file, rv)

end
