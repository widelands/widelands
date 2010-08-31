-- =======================================================================
--                   Territorial Lord Win condition
-- =======================================================================

use("aux", "coroutine") -- for sleep
use("aux", "table")
use("aux", "win_condition_functions")

set_textdomain("win_conditions")

return {
	name = _ "Territorial Lord",
	description = _ (
"Each player or team tries to obtain more than half of the maps' area. " .. 
"The winner will be the player or the team that is able to keep that area " ..
"for at least 20 minutes."
),
	func = function()

		-- Get the size of the map
		-- TODO instead of using all map fields, only count the none water fields
		-- TODO here and in user land ownership calculation
		local mapsize = wl.map.get_width() * wl.map.get_height()

		-- these variables will be used once a player or team owns more than half
		-- of the map's area
		local currentcandidate = "" -- Name of Team or Player
		local candidateisteam = false
		local remaining_time = 10 -- time in seconds, once == 0, the game ends

		-- Find all valid players
		local plrs = {}
		valid_players(plrs)

		function _calc_points()
			local stats = wl.game.get_general_statistics()
			local teams = {} -- points of teams
			local maxplayerpoints = 0 -- the highest points of a player without team
			local maxpointsplayer = 0 -- the player
			for idx, p in ipairs(plrs) do
				local team = p.team
				if team == 0 then
					if maxplayerpoints < stats[p:player_number()].land_size[#stats[p:player_number()].land_size] then
						maxplayerpoints = stats[p:player_number()].land_size[#stats[p:player_number()].land_size]
						maxpointsplayer = p
					end
				else
					teams[team] = teams[team] + stats[p:player_number()].land_size[#stats[p:player_number()].land_size]
				end
			end
			if maxplayerspoints > ( mapsize / 2 ) then
				-- player owns more than half of the map's area
				if candidateisteam == false and currentcandidate == maxpointsplayer.get_name() then
					remaining_time = remaining_time - 10
				else
					currentcandidate = maxpointsplayer.get_name()
					candidateisteam = false
					remaining_time = 20 * 60 -- 20 minutes
				end
			else
				for idx, teampoints in ipairs(teams) do
					if teampoints > ( mapsize / 2 ) then
						-- team owns more than half of the map's area
						if candidateisteam == true and currentcandidate == _("Team %i"):format(idx) then
							remaining_time = remaining_time - 10
						end
					else
						currentcandidate = _("Team %i"):format(idx)
						candidateisteam = true
						remaining_time = 20 * 60 -- 20 minutes
					end
				end
			end
		end

		function _send_state()
			local msg1 = _("%s owns more than half of the maps area."):format(currentcandidate)
			msg1 = msg1 .. "\n"
			msg1 = msg1 .. _("You still got %i minutes to prevent a victory."):format(remaining_time / 60)

			local msg2 = _("You own more than half of the maps area.")
			msg2 = msg2 .. "\n"
			msg2 = msg2 .. _("Keep it for %i more minutes win the game."):format(remaining_time / 60)

			for idx, p in ipairs(plrs) do
				if candidateisteam and currentcandidate == _("Team %i"):format(p.team)
					or not candidateisteam and currentcandidate == p.get_name() then
					p:send_message(_ "Status", msg2)
				else
					p:send_message(_ "Status", msg1)
				end
			end
		end



		sleep(1000)


		-- here is the main loop!!!
		while true do
			sleep(10 * 1000) -- Sleep 10 seconds

			_calc_points()

			if remaining_time % 600 == 0 then
				_send_state()
			end

			-- Game ended?
			if remaining_time == 0 then
				-- TODO: send victory and loser messages and show full map to all
				break
			end
		end
	end
}
