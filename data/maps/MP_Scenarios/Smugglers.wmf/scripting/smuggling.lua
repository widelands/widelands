-- =======================================================================
--                              Smuggling logic
-- =======================================================================

include "scripting/messages.lua"

-- Init routes
route_descrs = {
   {
      value = 2,
      send = map:get_field(35, 52):region(2),
      recv = map:get_field(96, 77):region(2),
      sending_warehouse = nil,
      receiving_warehouse = nil
   },
   {
      value = 2,
      send = map:get_field(98, 55):region(2),
      recv = map:get_field(34, 76):region(2),
      sending_warehouse = nil,
      receiving_warehouse = nil
   },

   {
      value = 3,
      send = map:get_field(64, 57):region(2),
      recv = map:get_field(78, 73):region(2),
      sending_warehouse = nil,
      receiving_warehouse = nil
   },
   {
      value = 3,
      send = map:get_field(77, 58):region(2),
      recv = map:get_field(65, 72):region(2),
      sending_warehouse = nil,
      receiving_warehouse = nil
   },

   {
      value = 2,
      send = map:get_field(62, 93):region(2),
      recv = map:get_field(78, 34):region(2),
      sending_warehouse = nil,
      receiving_warehouse = nil
   },
   {
      value = 2,
      send = map:get_field(80, 95):region(2),
      recv = map:get_field(63, 29):region(2),
      sending_warehouse = nil,
      receiving_warehouse = nil
   },
   {
      value = 2,
      send = map:get_field(18, 66):region(2),
      recv = map:get_field(121, 61):region(2),
      sending_warehouse = nil,
      receiving_warehouse = nil
   },
   {
      value = 2,
      send = map:get_field(124, 72):region(2),
      recv = map:get_field(19, 57):region(2),
      sending_warehouse = nil,
      receiving_warehouse = nil
   }
}

-- =================
-- Helper functions
-- =================
-- Find a warehouse for in the given area
function find_warehouse(region)
   for idx,f in ipairs(region) do
      if f.immovable and f.immovable.descr.type_name == "warehouse" then
         return f.immovable
      end
   end
end

function do_game_over()
   if game_over_done then return end

   local w1, w2, l1, l2, lc
   if points[1] >= points_to_win then
      w1, w2 = game.players[1], game.players[2]
      l1, l2 = game.players[3], game.players[4]
      lc = points[2]
   else
      w1, w2 = game.players[3], game.players[4]
      l1, l2 = game.players[1], game.players[2]
      lc = points[1]
   end
   send_to_all(game_over:bformat(
      w1.name, w2.name, l1.name, l2.name, (ngettext("%i point", "%i points", lc)):format(lc)
   ))
   game_over_done = true
end

function do_smuggling(route_descr)
   if points[1] >= points_to_win or
      points[2] >= points_to_win
   then
      do_game_over()
      return
   end

   if (route_descr.sending_warehouse == nil or route_descr.receiving_warehouse == nil) then
      -- Route hasn't been established yet - do nothing
      print("NOCOM nothing to smuggle")
      return
   end

   print("NOCOM do smuggling")

   local send_plr = route_descr.sending_warehouse.owner
   local recv_plr = route_descr.receiving_warehouse.owner
   local send_whf = route_descr.sending_warehouse.fields[1]
   local recv_whf = route_descr.receiving_warehouse.fields[1]

   -- Ensure that the warehouses are still there and owned by the same player
   if not send_whf.immovable or
      send_whf.immovable.descr.type_name ~= "warehouse" or
      send_whf.immovable.owner ~= send_plr or
      not recv_whf.immovable or
      recv_whf.immovable.descr.type_name ~= "warehouse" or
      recv_whf.immovable.owner ~= recv_plr
   then
      send_to_all(smuggling_route_broken:bformat(
         (ngettext("%i point", "%i points", route_descr.value)):format(route_descr.value), recv_plr.name, send_plr.name)
      )
      run(wait_for_established_route, route_descr)
      return
   end

   -- Collect ware types that both sending and receiving player can use
   local wares = {}
   for idx,ware in pairs(send_plr.tribe.wares) do
      if recv_plr.tribe:has_ware(ware.name) then
         table.insert(wares, ware.name)
      end
   end
   -- If the tribes don't have any wares in common, nothing can be smuggled
   -- This should not happen, but let's have a safeguard anyway.
   if #wares < 1 then
      do_game_over()
      return
   end
   -- We start counting at 0 so that we can use the modulo (%) operator
   -- for going round robin
   local last_ware_index = 0;

   -- Warp the next available ware, going round robin
   local empty_warehouse_guard = #wares
   local warp_index = last_ware_index
   local ware_to_warp = nil
   while empty_warehouse_guard > 0 do
      -- Index shift, because Lua tables start counting at 1
      local candidate = wares[warp_index + 1]
      if send_whf.immovable:get_wares(candidate) > 0 then
         ware_to_warp = candidate
         break
      end
      warp_index = (warp_index + 1) % #wares;
      empty_warehouse_guard = empty_warehouse_guard - 1
   end
   if ware_to_warp ~= nil then
   print("NOCOM " .. " " .. send_whf.x .. ", " .. send_whf.y .. " warping ware " .. ware_to_warp ..  ": " .. send_plr.name .. " -> " .. recv_plr.name)
      send_whf.immovable:set_wares(ware_to_warp, send_whf.immovable:get_wares(ware_to_warp) - 1)
      recv_whf.immovable:set_wares(
         ware_to_warp, recv_whf.immovable:get_wares(ware_to_warp) + 1
      )
      points[recv_plr.team] = points[recv_plr.team] + route_descr.value
   end
   -- Next round robin index
   last_ware_index = (last_ware_index + 1) % #wares;
end

function wait_for_established_route(route_descr)
   local receiving_wh, sending_wh
   route_descr.sending_warehouse = nil
   route_descr.receiving_warehouse = nil

   while 1 do
      receiving_wh = find_warehouse(route_descr.recv)
      sending_wh = find_warehouse(route_descr.send)
      if receiving_wh and sending_wh and receiving_wh.owner.team == sending_wh.owner.team then
         route_descr.sending_warehouse = sending_wh
         route_descr.receiving_warehouse = receiving_wh
         break
      end
      sleep(7138)
   end

   -- Send message to all players, send fields too for players with warehouse.
   local points = (ngettext("%i point", "%i points", route_descr.value)):format(route_descr.value)
   local non_team_message = smuggling_route_established_other_team:bformat(
      receiving_wh.owner.name, sending_wh.owner.name, points
   )
   for idx,plr in ipairs(game.players) do
      if plr.number ~= receiving_wh.owner.number and plr.number ~= sending_wh.owner.number then
         send_message(plr, _"Status", non_team_message, {popup=true})
      end
   end
   send_message(receiving_wh.owner, _"Status",
      smuggling_route_established_receiver:format(points), {popup=true, field=receiving_wh.fields[1]}
   )
   send_message(sending_wh.owner, _"Status",
      smuggling_route_established_sender:format(points), {popup=true, field=sending_wh.fields[1]}
   )
end
