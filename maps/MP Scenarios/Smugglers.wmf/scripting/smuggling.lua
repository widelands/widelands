-- =======================================================================
--                              Smuggling logic                             
-- =======================================================================

-- =================
-- Helper functions 
-- =================
function find_warehouse(plr, region)
   for idx,f in ipairs(region) do
      if f.immovable and f.immovable.owner == plr and
            f.immovable.name == "warehouse"
      then
         return f.immovable
      end
   end
end

function do_game_over() 
   if game_over_done then return end

   local w1, w2, l1, l2, lc
   if wares_smuggled[1] >= wares_to_smuggle then
      w1, w2 = game.players[1], game.players[3]
      l1, l2 = game.players[2], game.players[4]
      lc = wares_smuggled[2]
   else
      w1, w2 = game.players[2], game.players[4]
      l1, l2 = game.players[1], game.players[3]
      lc = wares_smuggled[1]
   end

   send_to_all(game_over:format(w1.name, w2.name, l1.name, l2.name, lc))
   game_over_done = true
end

function do_smuggling(top_plr, bottom_plr, fwh1, fwh2)
   while 1 do
      sleep(10000) -- Sleep 10s

      if wares_smuggled[1] >= wares_to_smuggle or
         wares_smuggled[2] >= wares_to_smuggle
      then
         do_game_over()
         break
      end

      if not fwh1.immovable or fwh1.immovable.name ~= "warehouse" or
         fwh1.immovable.owner ~= top_plr or
         not fwh2.immovable or fwh2.immovable.name ~= "warehouse" or
         fwh2.immovable.owner ~= bottom_plr
      then 
         send_to_all(smuggling_route_broken:format(
            top_plr.name, bottom_plr.name)
         )
         run(wait_for_established_route, top_plr, bottom_plr)
         break
      end

      -- Warp one ware
      local wares = fwh1.immovable:get_wares("all")
      local wn = {}
      for name,count in pairs(wares) do
         if count > 0 then
            wn[#wn + 1] = name
         end
      end
      if #wn > 0 then
         local ware_to_warp = wn[math.random(#wn)]
         fwh1.immovable:set_wares(ware_to_warp, wares[ware_to_warp] - 1)
         fwh2.immovable:set_wares(
            ware_to_warp, fwh2.immovable:get_wares(ware_to_warp) + 1
         )
         wares_smuggled[top_plr.team] = wares_smuggled[top_plr.team] + 1
      end
   end
end

function wait_for_established_route(top_plr, bottom_plr)
   local w1, w2
   while 1 do
      w1 = find_warehouse(top_plr, warehouse_regions[top_plr.number])
      w2 = find_warehouse(bottom_plr, warehouse_regions[bottom_plr.number])
      if w1 and w2 then break end
      sleep(7138)
   end

   send_to_all(smuggling_route_established:format(
      top_plr.name, bottom_plr.name
   ))

   run(do_smuggling, top_plr, bottom_plr, w1.fields[1], w2.fields[1])
end
