-- =======================================================================
--                              Smuggling logic
-- =======================================================================

-- =================
-- Helper functions
-- =================
-- Find a warehouse for in the given area
function find_warehouse(region)
   for idx,f in ipairs(region) do
      if f.immovable and f.immovable.name == "warehouse" then
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

function do_smuggling(route_descr, recv_plr, send_plr, recv_whf, send_whf)
   while 1 do
      sleep(10000) -- Sleep 10s

      if points[1] >= points_to_win or
         points[2] >= points_to_win
      then
         do_game_over()
         break
      end

      if not send_whf.immovable or send_whf.immovable.name ~= "warehouse" or
         send_whf.immovable.owner ~= send_plr or
         not recv_whf.immovable or recv_whf.immovable.name ~= "warehouse" or
         recv_whf.immovable.owner ~= recv_plr
      then
         send_to_all(smuggling_route_broken:bformat(
            (ngettext("%i point", "%i points", route_descr.value)):format(route_descr.value), recv_plr.name, send_plr.name)
         )
         run(wait_for_established_route, route_descr)
         break
      end

      -- Warp one ware
      local wares = send_whf.immovable:get_wares("all")
      local wn = {}
      for name,count in pairs(wares) do
         if count > 0 then
            wn[#wn + 1] = name
         end
      end
      if #wn > 0 then
         local ware_to_warp = wn[math.random(#wn)]
         send_whf.immovable:set_wares(ware_to_warp, wares[ware_to_warp] - 1)
         recv_whf.immovable:set_wares(
            ware_to_warp, recv_whf.immovable:get_wares(ware_to_warp) + 1
         )
         points[recv_plr.team] = points[recv_plr.team] + route_descr.value
      end
   end
end

function wait_for_established_route(route_descr)
   local receiving_wh, sending_wh
   while 1 do
      receiving_wh = find_warehouse(route_descr.recv)
      sending_wh = find_warehouse(route_descr.send)
      if receiving_wh and sending_wh and receiving_wh.owner.team == sending_wh.owner.team then break end
      sleep(7138)
   end

   -- Send message to all players, send fields too for players with warehouse.
   local points = (ngettext("%i point", "%i points", route_descr.value)):format(route_descr.value)
   local non_team_message = smuggling_route_established_other_team:bformat(
      receiving_wh.owner.name, sending_wh.owner.name, points
   )
   for idx,plr in ipairs(game.players) do
      if plr.number ~= receiving_wh.owner.number and plr.number ~= sending_wh.owner.number then
         plr:send_message(_ "Game Status", non_team_message, {popup=true})
      end
   end
   receiving_wh.owner:send_message(_ "Game Status",
      smuggling_route_established_receiver:format(points), {popup=true, field=receiving_wh.fields[1]}
   )
   sending_wh.owner:send_message(_ "Game Status",
      smuggling_route_established_sender:format(points), {popup=true, field=sending_wh.fields[1]}
   )

   run(do_smuggling, route_descr, receiving_wh.owner, sending_wh.owner, receiving_wh.fields[1], sending_wh.fields[1])
end
