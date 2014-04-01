-- =======================================================================
--                   Territorial Time Win condition
-- =======================================================================

include "scripting/coroutine.lua" -- for sleep
include "scripting/table.lua"
include "scripting/win_condition_functions.lua"

set_textdomain("win_conditions")

include "scripting/win_condition_texts.lua"

local wc_name = _ "Territorial Time"
local wc_version = 1
local wc_desc = _ (
	"Each player or team tries to obtain more than half of the map’s " ..
	"area. The winner will be the player or the team that is able to keep " ..
	"that area for at least 20 minutes, or the one with the most territory " ..
	"after 4 hours, whichever comes first."
)
local wc_has_territory = _"%1$s has %2$3.0f%% of the land (%3$i of %4$i)."
local wc_had_territory = _"%1$s had %2$3.0f%% of the land (%3$i of %4$i)."
local team_str = _"Team %i"

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
					if not f:has_caps("swimmable") then
						if f:has_caps("walkable") then
							fields[#fields+1] = f
						else
							-- editor disallows placement of immovables on dead and acid fields
							if f.immovable then
								fields[#fields+1] = f
							end
						end
					end
				end
			end
		end

		-- variables to track the maximum 4 hours of gametime
		local remaining_max_time = 4 * 60 * 60 -- 4 hours

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
			local points = {} -- tracking points of teams and players without teams
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
					points[#points + 1] = { p.name, _landsizes[p.number] }
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
			end
			for idx, t in ipairs(teamnumbers) do
				if teampoints[t] > ( #fields / 2 ) then
					-- this team owns more than half of the map's area
					foundcandidate = true
					if candidateisteam == true and currentcandidate == team_str:format(t) then
						remaining_time = remaining_time - 30
					else
						currentcandidate = team_str:format(t)
						candidateisteam = true
						remaining_time = 20 * 60 -- 20 minutes
					end
				end
				points[#points + 1] = { team_str:format(t), teampoints[t] }
			end
			if not foundcandidate then
				remaining_time = 10
			end
			return points
		end

		function _percent(part, whole)
			return (part * 100) / whole
		end

		-- Helper function to get the points that the leader has
		function _maxpoints(points)
			local max = 0
			for i=1,#points do
				if points[i][2] > max then max = points[i][2] end
			end
			return max
		end

		-- Helper function that returns a string containing the current
		-- land percentages of players/teams.
		function _status(points, has_had)
			local msg = ""
			for i=1,#points do
				msg = msg .. "\n"
				if (has_had == "has") then
					msg = msg ..
						(wc_has_territory):bformat(
							points[i][1],
							_percent(points[i][2], #fields),
							points[i][2],
							#fields)
				else
					msg = msg ..
						(wc_had_territory):bformat(
							points[i][1],
							_percent(points[i][2], #fields),
							points[i][2],
							#fields)
				end

			end
			return msg
		end

		function _send_state(points)
			set_textdomain("win_conditions")
			local msg1 = (_"%s owns more than half of the map’s area."):format(currentcandidate)
			msg1 = msg1 .. "\n"
			msg1 = msg1 .. (ngettext("You’ve still got %i minute to prevent a victory.",
						 "You’ve still got %i minutes to prevent a victory.",
						 remaining_time / 60))
					:format(remaining_time / 60)

			local msg2 = _"You own more than half of the map’s area."
			msg2 = msg2 .. "\n"
			msg2 = msg2 .. (ngettext("Keep it for %i more minute to win the game.",
						 "Keep it for %i more minutes to win the game.",
						 remaining_time / 60))
					:format(remaining_time / 60)

			for idx, p in ipairs(plrs) do
				local msg = ""
				if remaining_time < remaining_max_time and _maxpoints(points) > ( #fields / 2 ) then
					if candidateisteam and currentcandidate == team_str:format(p.team)
						or not candidateisteam and currentcandidate == p.name then
						msg = msg .. msg2 .. "\n\n"
					else
						msg = msg .. msg1 .. "\n\n"
					end
					msg = msg .. (ngettext("Otherwise the game will end in %i minute.",
							       "Otherwise the game will end in %i minutes.",
							       remaining_max_time / 60))
						:format(remaining_max_time / 60)
				else
					msg = msg .. (ngettext("The game will end in %i minute.",
							       "The game will end in %i minutes.",
							       remaining_max_time / 60))
						:format(remaining_max_time / 60)

				end
				msg = msg .. "\n\n"
				msg = msg .. game_status.body
				msg = msg .. _status(points, "has")
				p:send_message(game_status.title, msg, {popup = true})
			end
		end

		-- Start a new coroutine that checks for defeated players
		run(function()
			sleep(5000)
			check_player_defeated(plrs, lost_game.title,
				lost_game.body, wc_name, wc_version)
		end)

		-- here is the main loop!!!
		while true do
			-- Sleep 30 seconds == STATISTICS_SAMPLE_TIME
			sleep(30000)

			remaining_max_time = remaining_max_time - 30

			-- Check if a player or team is a candidate and update variables
			-- Returns the names and points for the teams and players without a team
			local points = _calc_points()


			-- Game is over, do stuff after loop
			if remaining_time <= 0 or remaining_max_time <= 0 then break end

			-- at the beginning send remaining max time message only each 30 minutes
			-- if only 30 minutes or less are left, send each 5 minutes
			-- also check if there is a candidate and we need to send an update
			if ((remaining_max_time < (30 * 60) and remaining_max_time % (5 * 60) == 0)
					or remaining_max_time % (30 * 60) == 0)
					or remaining_time % 300 == 0 then
				_send_state(points)
			end
		end

		local points = _calc_points()
		table.sort(points, function(a,b) return a[2] > b[2] end)

		-- Game has ended
		for idx, p in ipairs(plrs) do
			p.see_all = 1

			maxpoints = points[1][2]
			local wonmsg = won_game_over.body
			wonmsg = wonmsg .. "\n\n" .. game_status.body
			local lostmsg = lost_game_over.body
			lostmsg = lostmsg .. "\n\n" .. game_status.body
			for i=1,#points do
				if points[i][1] == team_str:format(p.team) or points[i][1] == p.name then
					if points[i][2] >= maxpoints then
						p:send_message(won_game_over.title, wonmsg .. _status(points, "had"))
						wl.game.report_result(p, 1, make_extra_data(p, wc_name, wc_version, {score=_landsizes[p.number]}))
					else
						p:send_message(lost_game_over.title, lostmsg .. _status(points, "had"))
						wl.game.report_result(p, 0, make_extra_data(p, wc_name, wc_version, {score=_landsizes[p.number]}))
					end
				end
			end
		end
	end
}
