-- =======================================================================
--          Test script for debugging.
-- =======================================================================

include "scripting/infrastructure.lua"
set_textdomain("tribes")

function observer_mode(plr_number)
    local game = wl.Game()
    local player = game.players[plr_number]
    player.see_all = true
end

function play_mode(plr_number)
    local game = wl.Game()
    local player = game.players[plr_number]
    player.see_all = false
end

function peace_mode(plr_number1, plr_number2)
    local game = wl.Game()
    local player = game.players[plr_number1]
    player:set_attack_forbidden(plr_number2, true)
end

function war_mode(plr_number1, plr_number2)
    local game = wl.Game()
    local player = game.players[plr_number1]
    player:set_attack_forbidden(plr_number2, false)
end

function allow_building(plr_number, bld_name)
    local game = wl.Game()
    local player = game.players[plr_number]
    player:allow_buildings(bld_name)
end

function forbid_building(plr_number, bld_name)
    local game = wl.Game()
    local player = game.players[plr_number]
    player:forbid_buildings(bld_name)
end

function force_expedition(plr_number)
    local game = wl.Game()
    local player = game.players[plr_number]
    local tribe = player.tribe
    local ports = player:get_buildings(tribe.name .. "_port")
    
    for i, port in ipairs(ports) do
        port:start_expedition()
    end
end

function remove_object(startx, starty)
   local game = wl.Game()
   local map = game.map

   map:get_field(startx, starty).immovable:remove()
end

function destroy_object(startx, starty)
   local game = wl.Game()
   local map = game.map

   map:get_field(startx, starty).immovable:destroy()
end

function place_object_tribe(startx, starty, objectname)
   local game = wl.Game()
   local map = game.map

   map:place_immovable(objectname, map:get_field(startx, starty), "tribe")
end

function place_object_world(startx, starty, objectname)
   local game = wl.Game()
   local map = game.map

   map:place_immovable(objectname, map:get_field(startx, starty), "world")
end

function force_flag(plr_number, startx, starty)
   local game = wl.Game()
   local map = game.map
   local player = game.players[plr_number]

   player:place_flag(map:get_field(startx, starty))
end

function remove_flag(startx, starty)
   local game = wl.Game()
   local map = game.map

   if (map:get_field(startx, starty).immovable.descr.type_name == "flag") then
      remove_object(startx, starty)
   end
end

function force_road(startx, starty, cmd)
   local game = wl.Game()
   local map = game.map
   local startflag = map:get_field(startx, starty).immovable
   local player = startflag.owner
   local roadtype = "normal"

   if cmd:sub(-1) ~= "|" then
      cmd = cmd .. "|"
   end

   moves = {}
   for m in cmd:gmatch("%a+[,|]") do
      moves[#moves+1] = m:sub(1,-2)
      if(m:sub(-1) == '|') then
         --moves[#moves+1] = true -- Force the road
         r = player:place_road(roadtype, startflag, table.unpack(moves))
         startflag = r.end_flag
         moves = {}
      end
   end
end

function remove_road(startx, starty)
   local game = wl.Game()
   local map = game.map

   if (map:get_field(startx, starty).immovable.descr.type_name == "road") then
      remove_object(startx, starty)
   end
end

function find_needed_flag_on_road(center_field, player, radius)
   for f_idx, field in ipairs(center_field:region(radius)) do
      if player == field.owner and field.buildable and field.has_roads == true then
         return field
      end
   end
   return nil
end

function connect_road(startx, starty, targetx, targety)
   local game = wl.Game()
   local map = game.map
   local startflag = map:get_field(startx, starty).immovable
   local targetflag = map:get_field(targetx, targety).immovable
   local player = startflag.owner
   local roadtype = "normal"

   local diffx = targetx - startx
   local diffy = targety - starty
   local cmd = ""

   if (diffx > 0) and (diffy > 0) then
      cmd = "br,br"
   end
   if (diffx > 0) and (diffy == 0) then
      cmd = "r,r"
   end
   if (diffx > 0) and (diffy < 0) then
      cmd = "tr,tr"
   end
   if (diffx < 0) and (diffy > 0) then
      cmd = "bl,bl"
   end
   if (diffx < 0) and (diffy == 0) then
      cmd = "l,l"
   end
   if (diffx < 0) and (diffy < 0) then
      cmd = "tl,tl"
   end
   if (diffx == 0) and (diffy > 0) then
      cmd = "br,bl"
   end
   if (diffx == 0) and (diffy < 0) then
      cmd = "tl,tr"
   end

   if cmd:sub(-1) ~= "|" then
      cmd = cmd .. "|"
   end

   moves = {}
   for m in cmd:gmatch("%a+[,|]") do
      moves[#moves+1] = m:sub(1,-2)
      if(m:sub(-1) == '|') then
         --moves[#moves+1] = true -- Force the road
         r = player:place_road(roadtype, startflag, table.unpack(moves))
         startflag = r.end_flag
         moves = {}
      end
   end
end

function place_building(plr_number, startx, starty, radius, buildingname)
   local game = wl.Game()
   local map = game.map
   local player = game.players[plr_number]

   place_building_in_region(player, buildingname, map:get_field(startx, starty):region(radius))
end

function replace_building(startx, starty, buildingname)
   local game = wl.Game()
   local map = game.map
   local player = map:get_field(startx, starty).owner

   if (map:get_field(startx, starty).immovable.descr.type_name == "flag") then
      remove_object(startx, starty-1) -- Remove existing building
      remove_object(startx-1, starty-1) -- Remove existing building
   else
      remove_object(startx, starty) -- Remove existing building
   end
   place_building_in_region(player, buildingname, map:get_field(startx, starty):region(1)) -- Place new one
end

function remove_building(startx, starty)
   local game = wl.Game()
   local map = game.map

   if (map:get_field(startx, starty).immovable.descr.type_name == "productionsite") then
      remove_object(startx, starty)
   end
   if (map:get_field(startx, starty).immovable.descr.type_name == "trainingsite") then
      remove_object(startx, starty)
   end
   if (map:get_field(startx, starty).immovable.descr.type_name == "militarysite") then
      remove_object(startx, starty)
   end
   if (map:get_field(startx, starty).immovable.descr.type_name == "warehouse") then
      remove_object(startx, starty)
   end
   if (map:get_field(startx, starty).immovable.descr.type_name == "market") then
      remove_object(startx, starty)
   end
end


function place_headquarters(plr_number, startx, starty)
    local game = wl.Game()
    local player = game.players[plr_number]
    local tribe = player.tribe
    local hqname = tribe.name .. "_headquarters"

    place_building(plr_number, startx, starty, 1, hqname)
end

function place_warehouse(plr_number, startx, starty)
    local game = wl.Game()
    local player = game.players[plr_number]
    local tribe = player.tribe
    local whname = tribe.name .. "_warehouse"

    place_building(plr_number, startx, starty, 1, whname)
end

function place_port(plr_number, startx, starty)
    local game = wl.Game()
    local player = game.players[plr_number]
    local tribe = player.tribe
    local ptname = tribe.name .. "_port"

    place_building(plr_number, startx, starty, 0, ptname)
end

function destroy_all_buildings(plr_number, buildingname)
   local game = wl.Game()
   local player = game.players[plr_number]

   for i, tplr in ipairs(game.players) do
       for i, building in ipairs(tplr.tribe.buildings) do
          if building.type_name == buildingname or building.name == buildingname then
             for i, building in ipairs(player:get_buildings(building.name)) do
                building:destroy()
             end
          end
       end
   end 
end

function restock_warehouse(startx, starty)
    local game = wl.Game()
    local map = game.map
    local player = map:get_field(startx, starty).owner
    local tribe = player.tribe
    local field = map:get_field(startx, starty).immovable
    local warehouses = player:get_buildings(tribe.name .. "_warehouse")
    local headquarters = player:get_buildings(tribe.name .. "_headquarters")
    local ports = player:get_buildings(tribe.name .. "_port")

    local MAX_UNDEFINED_CARRIERS = 10
    local MAX_UNDEFINED_BUILDERS = 4
    local MAX_UNDEFINED_WARES = 16

    for i, building in ipairs(headquarters) do
        if building == field then
            for j, ware in ipairs(tribe.wares) do
                if building:get_wares(ware.name) < MAX_UNDEFINED_WARES then
                    building:set_wares(ware.name, MAX_UNDEFINED_WARES)
                end
            end
            if (building:get_workers(tribe.name .. "_carrier") < MAX_UNDEFINED_CARRIERS) then
                building:set_workers(tribe.name .. "_carrier", MAX_UNDEFINED_CARRIERS)
            end
            if (building:get_workers(tribe.name .. "_builder") < MAX_UNDEFINED_BUILDERS) then
                building:set_workers(tribe.name .. "_builder", MAX_UNDEFINED_BUILDERS)
            end
        end
    end
    for i, building in ipairs(warehouses) do
        if building == field then
            for j, ware in ipairs(tribe.wares) do
                if building:get_wares(ware.name) < MAX_UNDEFINED_WARES then
                    building:set_wares(ware.name, MAX_UNDEFINED_WARES)
                end
            end
            if (building:get_workers(tribe.name .. "_carrier") < MAX_UNDEFINED_CARRIERS) then
                building:set_workers(tribe.name .. "_carrier", MAX_UNDEFINED_CARRIERS)
            end
            if (building:get_workers(tribe.name .. "_builder") < MAX_UNDEFINED_BUILDERS) then
                building:set_workers(tribe.name .. "_builder", MAX_UNDEFINED_BUILDERS)
            end
        end
    end
    for i, building in ipairs(ports) do
        if building == field then
            for j, ware in ipairs(tribe.wares) do
                if building:get_wares(ware.name) < MAX_UNDEFINED_WARES then
                    building:set_wares(ware.name, MAX_UNDEFINED_WARES)
                end
            end
            if (building:get_workers(tribe.name .. "_carrier") < MAX_UNDEFINED_CARRIERS) then
                building:set_workers(tribe.name .. "_carrier", MAX_UNDEFINED_CARRIERS)
            end
            if (building:get_workers(tribe.name .. "_builder") < MAX_UNDEFINED_BUILDERS) then
                building:set_workers(tribe.name .. "_builder", MAX_UNDEFINED_BUILDERS)
            end
        end
    end
end

function evacuade_warehouse(startx, starty)
    local game = wl.Game()
    local map = game.map
    local player = map:get_field(startx, starty).owner
    local tribe = player.tribe
    local field = map:get_field(startx, starty).immovable
    local warehouses = player:get_buildings(tribe.name .. "_warehouse")

    for i, building in ipairs(warehouses) do
        if building == field then
            for i, ware in ipairs(tribe.wares) do
                building:set_warehouse_policies(ware.name, "remove")
            end
            for i, worker in ipairs(tribe.workers) do
                building:set_warehouse_policies(worker.name, "remove")
            end
        end
    end
end

function warehouse_ware_policy(startx, starty, warename, policiename)
    local game = wl.Game()
    local map = game.map
    local player = map:get_field(startx, starty).owner
    local tribe = player.tribe
    local field = map:get_field(startx, starty).immovable
    local warehouses = player:get_buildings(tribe.name .. "_warehouse")

    for i, building in ipairs(warehouses) do
        if building == field then
            building:set_warehouse_policies(warename, policiename)
        end
    end
end

function warehouse_worker_policy(startx, starty, workername, policiename)
    local game = wl.Game()
    local map = game.map
    local player = map:get_field(startx, starty).owner
    local tribe = player.tribe
    local field = map:get_field(startx, starty).immovable
    local warehouses = player:get_buildings(tribe.name .. "_warehouse")

    for i, building in ipairs(warehouses) do
        if building == field then
            building:set_warehouse_policies(workername, policiename)
        end
    end
end

function set_ware(plr_number, warename, warecount)
   local player = wl.Game().players[plr_number]
   local tribe = player.tribe

   for i, building in ipairs(tribe.buildings) do
      -- restock warehouses
      if building.type_name == "warehouse" then
        for i, building in ipairs(player:get_buildings(building.name)) do
             building:set_wares(warename, warecount)
        end
      end
   end
end

-- following functions could produce memory leak - dont use it, if not necessary! --
function cheat_wares(plr_number)
   local MAX_UNDEFINED_WARES = 16

   local player = wl.Game().players[plr_number]
   local tribe = player.tribe

   for i, building in ipairs(tribe.buildings) do
      -- restock warehouses
      if building.type_name == "warehouse" then
       for i, building in ipairs(player:get_buildings(building.name)) do
         for i, ware in ipairs(tribe.wares) do
            if building:get_wares(ware.name) < MAX_UNDEFINED_WARES then
               building:set_wares(ware.name, MAX_UNDEFINED_WARES)
            end
         end
       end
      end
   end
end

function allow_worker(plr_number, wrk_name)
    local game = wl.Game()
    local player = game.players[plr_number]
    player:allow_workers(wrk_name)
end

function set_worker(plr_number, workername, workercount)
   local player = wl.Game().players[plr_number]
   local tribe = player.tribe

   for i, building in ipairs(tribe.buildings) do
      -- restaff warehouses
      if building.type_name == "warehouse" then
        for i, building in ipairs(player:get_buildings(building.name)) do
            building:set_workers(workername, workercount)
        end
      end
   end
end

function cheat_workers(plr_number)
   local MAX_UNDEFINED_WORKERS = 4

   local player = wl.Game().players[plr_number]
   local tribe = player.tribe

   for i, building in ipairs(tribe.buildings) do
      -- restaff warehouses
      if building.type_name == "warehouse" then
       for i, building in ipairs(player:get_buildings(building.name)) do
         for i, worker in ipairs(tribe.workers) do
            if (building:get_workers(worker.name) < MAX_UNDEFINED_WORKERS) and not (worker.type_name == "soldier") then
               building:set_workers(worker.name, MAX_UNDEFINED_WORKERS)
            end
         end
       end
      end
   end
end

function cheat_soldiers(plr_number)
   local MAX_UNDEFINED_SOLDIERS = 8
   local SOLDIER_STATS_NOVICE = { 0, 0, 0, 0 }

   local player = wl.Game().players[plr_number]
   local tribe = player.tribe

   -- retrieve best soldier stats
   local soldier_stats = SOLDIER_STATS_NOVICE
   for i, worker in ipairs(tribe.workers) do
      if worker.type_name == "soldier" then
       soldier_stats = { worker.max_health_level, worker.max_attack_level, worker.max_defense_level, worker.max_evade_level }
      end
   end

   for i, building in ipairs(tribe.buildings) do
      -- restaff warehouses
      if building.type_name == "warehouse" then
       for i, building in ipairs(player:get_buildings(building.name)) do
         for i, worker in ipairs(tribe.workers) do
            if worker.type_name == "soldier" then
               building:set_soldiers(soldier_stats, MAX_UNDEFINED_SOLDIERS)
            else
               building:set_workers(worker.name, MAX_UNDEFINED_SOLDIERS)
            end
         end
       end
      end
   end
end
