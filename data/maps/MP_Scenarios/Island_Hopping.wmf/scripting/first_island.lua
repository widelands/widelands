-- ==========================
-- Code for the first island
-- ==========================
function _wait_for_castle_on_finish_area(plr, island_idx)
   local finish_area = _finish_areas[island_idx][plr.number]
   while not check_for_buildings(plr, {atlanteans_castle=1}, finish_area) do
      sleep(1237)
   end
end

function run_island(plr, island_idx)
   if island_idx == 3 then
      return -- Nothing to do for 3rd island. Only watch who will win.
   end
   sleep(200)
   print(("Running Island %i for player %i!"):format(island_idx, plr.number))

   _wait_for_castle_on_finish_area(plr, island_idx)

   local rank = _nplayers_finished_island[island_idx] + 1
   _nplayers_finished_island[island_idx] = rank

   local rewards = _finish_rewards[island_idx][rank]
   send_to_all_inboxes(
      msgs_finished_island[rank]:bformat(plr.name, island_idx + 1) ..
      finished_island_continues:format(format_rewards(rewards))
   )

   local new_hq = hop_to_next_island(plr, island_idx)
   add_wares(new_hq, rewards)

   run_island(plr, island_idx + 1)
end

function watch_hill()
   sleep(200)

   while true do
      local castle_field = nil
      local started_claiming = nil
      local claiming_player = nil
      local send_msg = 0

      -- Wait for castle to appear
      while castle_field == nil do
         sleep(7477)
         for idx, f in pairs(hill) do
            if f.immovable and f.immovable.descr.name == "atlanteans_castle"
            then
               castle_field = f
               started_claiming = game.time
               claiming_player = castle_field.immovable.owner
               send_to_all_inboxes(player_claims_hill:format(claiming_player.name))
               break
            end
         end
      end

      -- Keep an eye on this castle
      while true do
         sleep(1049)
         local imm = castle_field.immovable
         if not imm or imm.descr.name ~= "atlanteans_castle" or imm.owner ~= claiming_player then
            send_to_all_inboxes(lost_control:format(claiming_player.name))
            break
         end

         local claimed_for = (game.time - started_claiming) / 1000
         if 5*60 < claimed_for and send_msg == 0 then
            send_to_all_inboxes(had_control_for:bformat(claiming_player.name, _("5 minutes")))
            send_msg = send_msg + 1
         end
         if 10*60 < claimed_for and send_msg == 1 then
            send_to_all_inboxes(had_control_for:bformat(claiming_player.name, _("10 minutes")))
            send_msg = send_msg + 1
         end
         if 15*60 < claimed_for and send_msg == 2 then
            send_to_all_inboxes(had_control_for:bformat(claiming_player.name, _("15 minutes")))
            send_msg = send_msg + 1
         end
         if 20*60 < claimed_for and send_msg == 3 then
            send_to_all_inboxes(player_won:format(claiming_player.name))
            return
         end
      end
   end
end
