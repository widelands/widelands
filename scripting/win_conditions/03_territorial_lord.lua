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
		"Each player or team tries to obtain more than half of the maps' " ..
		"area. The winner will be the player or the team that is able to keep " ..
		"that area for at least 20 minutes."
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
		local remaining_time = 10 -- (dummy) -- time in secs, if == 0 -> victory

		-- Find all valid players
		local plrs = {}
		valid_players(plrs)

		-- Find all valid teams
		local teamnumbers = {} -- array with team numbers
		for idx,p in ipairs(plrs) do
			local team = p.team
			if team > 0 then
				local found = false
				for idy,t in ipairs(teamnumbers) do
					if t == team then
						found = true
						break
					end
				end
				if not found then
					teamnumbers[#teamnumbers+1] = team
				end
			end
		end

		function _calc_points()
			local teampoints = {}     -- points of teams
			local maxplayerpoints = 0 -- the highest points of a player without team
			local maxpointsplayer = 0 -- the player
			local foundcandidate = false
			for idx, p in ipairs(plrs) do
				local team = p.team
				local landsize = p.get_current_land_size(p)
				if team == 0 then
					if maxplayerpoints < landsize then
						maxplayerpoints = landsize
						maxpointsplayer = p
					end
				else
					if not teampoints[team] then -- init the value
						teampoints[team] = 0
					end
					teampoints[team] = teampoints[team] + landsize
				end
			end
			if maxplayerpoints > ( mapsize / 2 ) then
				-- player owns more than half of the map's area
				foundcandidate = true
				if candidateisteam == false and currentcandidate == maxpointsplayer.name then
					remaining_time = remaining_time - 30
				else
					currentcandidate = maxpointsplayer.name
					candidateisteam = false
					remaining_time = 20 * 60 -- 20 minutes
				end
			else
				for idx, t in ipairs(teamnumbers) do
					if teampoints[t] > ( mapsize / 2 ) then
						-- this team owns more than half of the map's area
						foundcandidate = true
						if candidateisteam == true and currentcandidate == _("Team %i"):format(t) then
							remaining_time = remaining_time - 30
						else
							currentcandidate = _("Team %i"):format(t)
							candidateisteam = true
							remaining_time = 20 * 60 -- 20 minutes
						end
					end
				end
			end
			if not foundcandidate then
				remaining_time = 10
			end
		end

		function _send_state()
			local msg1 = _("%s owns more than half of the maps area."):format(currentcandidate)
			msg1 = msg1 .. "\n"
			msg1 = msg1 .. _("You still got %i minutes to prevent a victory."):format(remaining_time / 60)

			local msg2 = _("You own more than half of the maps area.")
			msg2 = msg2 .. "\n"
			msg2 = msg2 .. _("Keep it for %i more minutes to win the game."):format(remaining_time / 60)

			for idx, p in ipairs(plrs) do
				if candidateisteam and currentcandidate == _("Team %i"):format(p.team)
					or not candidateisteam and currentcandidate == p.name then
					p:send_message(_ "Status", msg2)
				else
					p:send_message(_ "Status", msg1)
				end
			end
		end



		-- here is the main loop!!!
		while true do
			-- Sleep 30 seconds == STATISTICS_SAMPLE_TIME
			sleep(30000)

			-- Check if a player or team is a candidate and update variables
			_calc_points()

			-- Check whether one of the players got defeated and handle defeats
			check_player_defeated(plrs, _ "You are defeated!",
				_ "You have nothing to command left. If you want, you may " ..
				  "continue as spectator.")

			-- Do this stuff, if the game is over
			if remaining_time == 0 then
				for idx, p in ipairs(plrs) do
					p.see_all = 1
					if candidateisteam and currentcandidate == _("Team %i"):format(p.team)
						or not candidateisteam and currentcandidate == p.name then
						p:send_message(_"You won!", _"You are the winner!")
					else
						p:send_message(_"You lost", _"You've lost this game!")
					end
				end
				break
			end

			-- If there is a candidate, check whether we have to send an update
			if remaining_time % 300 == 0 then -- every 5 minutes (5 * 60 )
				_send_state()
			end
		end
	end
}
