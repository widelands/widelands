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
      w1, w2 = game.players[1], game.players[3]
      l1, l2 = game.players[2], game.players[4]
      lc = points[2]
   else
      w1, w2 = game.players[2], game.players[4]
      l1, l2 = game.players[1], game.players[3]
      lc = points[1]
   end

   send_to_all(game_over:format(w1.name, w2.name, l1.name, l2.name, lc))
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
         send_to_all(smuggling_route_broken:format(
            route_descr.value, recv_plr.name, send_plr.name)
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
   local w1, w2
   while 1 do
      w1 = find_warehouse(route_descr.recv)
      w2 = find_warehouse(route_descr.send)
      if w1 and w2 and w1.owner.team == w2.owner.team then break end
      sleep(7138)
   end

   send_to_all(smuggling_route_established:format(
      w1.owner.name, w2.owner.name, route_descr.value 
   ))

   run(do_smuggling, route_descr, w1.owner, w2.owner, w1.fields[1], w2.fields[1])
end
