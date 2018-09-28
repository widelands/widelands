-- NOCOM for Territorial Lord and Time. Document.

function get_buildable_fields()
   local fields = {}
   local map = wl.Game().map
   for x=0, map.width-1 do
      for y=0, map.height-1 do
         local f = map:get_field(x,y)
         if f.is_buildable then
            table.insert(fields, f)
         end
      end
   end
   print("NOCOM Found " .. #fields .. " buildable fields")
   return fields
end

function count_owned_fields_for_all_players(fields, plrs)
   local owned_fields = {}
   -- init the landsizes for each player
   for idx,plr in ipairs(plrs) do
      owned_fields[plr.number] = 0
   end

   for idx,f in ipairs(fields) do
      -- check if field is owned by a player
      local owner = f.owner
      if owner then
         local owner_number = owner.number
         if owned_fields[owner_number] == nil then
            -- In case player was defeated and lost all their warehouses, make sure they don't count
            owned_fields[owner_number] = -1
         elseif owned_fields[owner_number] >= 0 then
            owned_fields[owner_number] = owned_fields[owner_number] + 1
         end
      end
   end
   return owned_fields
end


-- Used by calculate_territory_points keep track of when the winner changes
local winning_players = {}
local winning_teams = {}

-- NOCOM document
territory_points = {
   -- These variables will be used once a player or team owns more than half
   -- of the map's area, to display the name of winning team/player.
   -- TODO(GunChleoc): We want to be able to list multiple winners in case of a draw.
   last_winning_team = -1,
   last_winning_player = -1,
   -- Remaining time for victory by > 50% territory. Default value is also used to calculate whether to send a report to players.
   remaining_time = 10, -- (dummy) -- time in secs, if == 0 -> victory
   -- Points by player
   all_player_points = {},
   -- Points by rank, as also used by the backend for the custom statistics
   points = {}
}

-- NOCOM document
function calculate_territory_points(fields, plrs)
   local points = {} -- tracking points of teams and players without teams
   local territory_was_kept = false

   territory_points.all_player_points = count_owned_fields_for_all_players(fields, plrs)
   local ranked_players = rank_players(territory_points.all_player_points, plrs)

   -- Check if we have a winner. The table was sorted, so we can simply grab the first entry.
   local winning_points = -1
   if ranked_players[1].points > ( #fields / 200 ) then
      winning_points = ranked_players[1].points
   end

   -- Calculate which team or player is the current winner, and whether the winner has changed
   for tidx, teaminfo in ipairs(ranked_players) do
      local is_winner = teaminfo.points == winning_points
      if teaminfo.team ~= 0 then
         points[#points + 1] = { team_str:format(teaminfo.team), teaminfo.points }
         if is_winner then
            print("NOCOM Winner is team " .. teaminfo.team .. " with " .. teaminfo.points .. " points")
            territory_was_kept = winning_teams[teaminfo.team] ~= nil
            winning_teams[teaminfo.team] = true
            territory_points.last_winning_team = teaminfo.team
            territory_points.last_winning_player = -1
         else
            winning_teams[teaminfo.team] = nil
         end
      end

      for pidx, playerinfo in ipairs(teaminfo.players) do
         if teaminfo.points ~= playerinfo.points then
            winning_players[playerinfo.number] = nil
         elseif is_winner and teaminfo.team == 0 then
            print("NOCOM Winner is player " .. playerinfo.number .. " with " .. playerinfo.points .. " points")
            territory_was_kept = winning_players[playerinfo.number] ~= nil
            winning_players[playerinfo.number] = true
            territory_points.last_winning_player = playerinfo.number
            territory_points.last_winning_team = -1
         else
            winning_players[playerinfo.number] = nil
         end
         if teaminfo.team == 0 and plrs[playerinfo.number] ~= nil then
            points[#points + 1] = { plrs[playerinfo.number].name, playerinfo.points }
         end
      end
   end

   -- Set the remaining time according to whether the winner is still the same
   if territory_was_kept then
      -- Still the same winner
      territory_points.remaining_time = territory_points.remaining_time - 30
      print("NOCOM Territory was kept by " .. territory_points.last_winning_team .. " - " .. territory_points.last_winning_player .. ". Remaining time: " .. territory_points.remaining_time)
   elseif winning_points == -1 then
      -- No winner. This value is used to calculate whether to send a report to players.
      territory_points.remaining_time = 10
   else
      -- Winner changed
      territory_points.remaining_time = 20 * 60 -- 20 minutes
      print("NOCOM NEW aqcuisition by " .. territory_points.last_winning_team .. " - " .. territory_points.last_winning_player .. ". Remaining time: " .. territory_points.remaining_time)
   end
   territory_points.points = points
end
