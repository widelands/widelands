-- ==========================
-- Code for the first island 
-- ==========================
_nplayers_finished_first_island = 0
_island1_finishs = {
   map:get_field(136,125):region(3), -- player 1
   map:get_field(136, 70):region(3), -- player 2
   map:get_field( 57, 68):region(3), -- player 3
   map:get_field( 56,122):region(3)  -- player 4
}
_island1_finish_rewards = {
   { trunk = 10, planks = 20, stone = 20 },  -- 1st to finish 
   { trunk = 15, planks = 25, stone = 25 },  -- 2nd to finish 
   { trunk = 20, planks = 30, stone = 30 },  -- 3rd to finish 
   { trunk = 25, planks = 35, stone = 35 }   -- 4th to finish 
}

-- TODO: move into its own file
_island2_starts = {
   map:get_field(143, 148),
   map:get_field(142,  45),
   map:get_field( 51,  44),
   map:get_field( 49, 147)
}

-- TODO: remove this
function cheat()
   game.players[1]:place_building("castle", map:get_field(136, 125), 0, 1)
end

function run_island1(plr)
   sleep(200)
   
   while not check_for_buildings(plr, {castle=1}, _island1_finishs[plr.number]) do
      sleep(1237)
   end
   _nplayers_finished_first_island = _nplayers_finished_first_island + 1

   local sf = map.player_slots[plr.number].starting_field
   finished_island(plr, sf, _island2_starts[plr.number],
      1, _nplayers_finished_first_island,
      _island1_finish_rewards[_nplayers_finished_first_island]
   )

   -- TODO: inform at the beginning about the rewards for finishing this island
   -- TODO: message when reaching island 2
   -- TODO: write island 2
   -- run_island2(plr)
end

-- TODO: transfer of soldiers?
function finished_island(plr, old_hq_field, new_hq_field, island_no, rank, rewards)
   -- TODO: next lines into their own function
   send_to_all(rt(
      p(msgs_finished_island[rank]:format(plr.number, island_no)) .. 
      p(finished_island_continues:format(format_rewards(rewards)))
   ))

   -- Remove all other workers and immovable of this player
   local workers = {}
   local wares = {}
   for x=0, map.width-1 do
      for y=0, map.height-1 do
         local f = map:get_field(x, y)
         local imm = f.immovable
         if imm and imm.owner == plr then
            -- salvage workers
            if imm.get_workers then
               for name, count in pairs(imm:get_workers("all")) do
                  if not workers[name] then workers[name] = 0 end
                  workers[name] = workers[name] + count
                  imm:set_workers(name, 0)
               end
            end

            -- salvage wares
            if imm.get_wares then
               for name, count in pairs(imm:get_wares("all")) do
                  if not wares[name] then wares[name] = 0 end
                  wares[name] = wares[name] + count
                  imm:set_wares(name, 0)
               end
            end

            -- Remove this object
            imm:remove()
         end
      end
   end

   -- Place the new HQ, completely empty
   local new_hq = plr:place_building("headquarters", new_hq_field, false, true)
   add_workers(new_hq, workers)
   add_wares(new_hq, wares)
   add_wares(new_hq, rewards)
end
