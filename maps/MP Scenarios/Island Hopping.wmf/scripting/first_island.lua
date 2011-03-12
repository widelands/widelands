-- ==========================
-- Code for the first island 
-- ==========================
_nplayers_finished_island = { 0, 0 }
_start_fields = {
   { -- Island 1
      map.player_slots[1].starting_field,
      map.player_slots[2].starting_field,
      map.player_slots[3].starting_field,
      map.player_slots[4].starting_field
   },
   { -- Island 2
      map:get_field(143, 148),
      map:get_field(142,  45),
      map:get_field( 51,  44),
      map:get_field( 49, 147)
   },
   { -- Island 3
      map:get_field(180, 182),
      map:get_field(180,  10),
      map:get_field( 13,   9),
      map:get_field( 13, 182)
   }
}

_finish_areas = {
   { -- Island 1
      map:get_field(136,125):region(3), -- player 1
      map:get_field(136, 70):region(3), -- player 2
      map:get_field( 57, 68):region(3), -- player 3
      map:get_field( 56,122):region(3)  -- player 4
   },
   { -- Island 2
      map:get_field(167,164):region(3), -- player 1
      map:get_field(167, 28):region(3), -- player 2
      map:get_field( 27, 27):region(3), -- player 3
      map:get_field( 26,161):region(3)  -- player 4
   }
}

-- TODO: come up with proper ones
_finish_rewards = {
   { -- Island 1
      { trunk = 10, planks = 20, stone = 20 },  -- 1st to finish 
      { trunk = 15, planks = 25, stone = 25 },  -- 2nd to finish 
      { trunk = 20, planks = 30, stone = 30 },  -- 3rd to finish 
      { trunk = 25, planks = 35, stone = 35 }   -- 4th to finish 
   },
   { -- Island 2
      { trunk = 10, planks = 20, stone = 20 },  -- 1st to finish 
      { trunk = 15, planks = 25, stone = 25 },  -- 2nd to finish 
      { trunk = 20, planks = 30, stone = 30 },  -- 3rd to finish 
      { trunk = 25, planks = 35, stone = 35 }   -- 4th to finish 
   }
}

-- TODO: remove this
function cheat(plr, island_idx)
   game.players[plr]:place_building("castle", 
      _finish_areas[island_idx][plr][19] , 0, 1
   )
end

-- TODO: rename this file to something more true
-- TODO: carriers and worker bobs are not transferred. Check road workers and bobs
function _hop_to_next_island(plr, island_idx)
   local old_hq_field = _start_fields[island_idx][plr.number]
   local new_hq_field = _start_fields[island_idx + 1][plr.number]

   -- Remove all other workers and immovable of this player
   local workers = {}
   local wares = {}
   local soldiers = {}
   for x=0, map.width-1 do
      for y=0, map.height-1 do
         local f = map:get_field(x, y)
         local imm = f.immovable
         if imm and imm.owner == plr then
            -- salvage soldiers
            if imm.get_soldiers then
               for descr, count in pairs(imm:get_soldiers("all")) do
                  local sdescr = ("%i:%i:%i:%i"):format(unpack(descr))
                  if not soldiers[sdescr] then soldiers[sdescr] = 0 end
                  soldiers[sdescr] = soldiers[sdescr] + count
               end
               imm:set_soldiers({0,0,0,0}, 0)
            end

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

   -- Place the new HQ
   local new_hq = plr:place_building("headquarters", new_hq_field, false, true)

   -- Fill it with all the stuff from the last island
   add_soldiers(new_hq, soldiers)
   add_workers(new_hq, workers)
   add_wares(new_hq, wares)

   return new_hq
end

function _wait_for_castle_on_finish_area(plr, island_idx)
   local finish_area = _finish_areas[island_idx][plr.number]
   while not check_for_buildings(plr, {castle=1}, finish_area) do
      sleep(1237)
   end
end

function run_island(plr, island_idx)
   if island_idx == 3 then 
      -- TODO: special case island 3
      return 
   end
   sleep(200)
   print(("Running Island %i for player %i!"):format(island_idx, plr.number))
   
   -- TODO: inform at the beginning about the rewards for finishing this island
   _wait_for_castle_on_finish_area(plr, island_idx)

   local rank = _nplayers_finished_island[island_idx] + 1
   _nplayers_finished_island[island_idx] = rank

   -- TODO: next lines into their own function
   local rewards = _finish_rewards[island_idx][rank]
   send_to_all(rt(
      p(msgs_finished_island[rank]:format(plr.number, island_idx)) .. 
      p(finished_island_continues:format(format_rewards(rewards)))
   ))


   local new_hq = _hop_to_next_island(plr, island_idx)
   add_wares(new_hq, rewards)

   run_island(plr, island_idx + 1)
end

