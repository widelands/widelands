-- =======================================================================
--                   Territorial Lord Win condition
-- =======================================================================

use("aux", "coroutine") -- for sleep
use("aux", "table")
use("aux", "win_condition_functions")

set_textdomain("win_conditions")

local wc_name = _ "Territorial Lord"
local wc_desc = _ (
	"Each player or team tries to obtain more than half of the maps' " ..
	"area. The winner will be the player or the team that is able to keep " ..
	"that area for at least 20 minutes."
)
return {
	name = wc_name,
	description = wc_desc,
	func = function()

		-- Get all valueable fields of the map
		local fields = {}
      local map = wl.Game().map
		for x=0,map.width-1 do
			for y=0,map.height-1 do
				local f = map:get_field(x,y)
				if f then
					-- add this field to the list as long as it has not movecaps swim
					if not f.has_movecaps_swim(f) then
						fields[#fields+1] = f
					end
				end
			end
		end

		-- these variables will be used once a player or team owns more than half
		-- of the map's area
		local currentcandidate = "" -- Name of Team or Player
		local candidateisteam = false
		local remaining_time = 10 -- (dummy) -- time in secs, if == 0 -> victory

		-- Find all valid players
      local plrs = wl.Game().players

		-- send a message with the game type to all players
		broadcast(plrs, wc_name, wc_desc)

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

		local _landsizes = {}
		local function _calc_current_landsizes()
			-- init the landsizes for each player
			for idx,plr in ipairs(plrs) do
				_landsizes[plr.number] = 0
			end

			for idx,f in ipairs(fields) do
				-- check if field is owned by a player
				local o = f.owner
				if o then
					local n = o.number
					_landsizes[n] = _landsizes[n] + 1
				end
			end
		end

		function _calc_points()
			local teampoints = {}     -- points of teams
			local maxplayerpoints = 0 -- the highest points of a player without team
			local maxpointsplayer = 0 -- the player
			local foundcandidate = false

			_calc_current_landsizes()

			for idx, p in ipairs(plrs) do
				local team = p.team
				if team == 0 then
					if maxplayerpoints < _landsizes[p.number] then
						maxplayerpoints = _landsizes[p.number]
						maxpointsplayer = p
					end
				else
					if not teampoints[team] then -- init the value
						teampoints[team] = 0
					end
					teampoints[team] = teampoints[team] + _landsizes[p.number]
				end
			end

			if maxplayerpoints > ( #fields / 2 ) then
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
					if teampoints[t] > ( #fields / 2 ) then
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
					p:send_message(_ "Status", msg2, {popup = true})
				else
					p:send_message(_ "Status", msg1, {popup = true})
				end
			end
		end

		-- Start a new coroutine that checks for defeated players
		run(function()
			sleep(5000)
			check_player_defeated(plrs, _ "You are defeated!",
				_ ("You have nothing to command left. If you want, you may " ..
		         "continue as spectator."))
		end)

		-- here is the main loop!!!
		while true do
			-- Sleep 30 seconds == STATISTICS_SAMPLE_TIME
			sleep(30000)

			-- Check if a player or team is a candidate and update variables
			_calc_points()

			-- Do this stuff, if the game is over
			if remaining_time == 0 then
				for idx, p in ipairs(plrs) do
					p.see_all = 1
					if candidateisteam and currentcandidate == _("Team %i"):format(p.team)
						or not candidateisteam and currentcandidate == p.name then
						p:send_message(_"You won!", _"You are the winner!", {popup = true})
					else
						p:send_message(_"You lost", _"You've lost this game!", {popup = true})
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
