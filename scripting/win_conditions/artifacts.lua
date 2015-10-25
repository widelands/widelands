-- =======================================================================
--                         Artifact condition
-- =======================================================================

include "scripting/coroutine.lua" -- for sleep
include "scripting/win_condition_functions.lua"
include "scripting/formatting.lua"

set_textdomain("win_conditions")

include "scripting/win_condition_texts.lua"

local wc_name = "Artifacts"
-- This needs to be exactly like wc_name, but localized, because wc_name
-- will be used as the key to fetch the translation in C++
local wc_descname = _("Artifacts")
local wc_version = 1
local wc_desc = _ "Search for the ancient artifacts. Once all of them are found, the team who owns most of them will win the game."
return {
	name = wc_name,
	description = wc_desc,
	map_tags = { "artifacts" }, -- Map tags needed so that this win condition will be available
	func = function()
		-- set the objective with the game type for all players
		broadcast_objective("win_condition", wc_descname, wc_desc)

		local function _getkey(p)
			if p.team == 0 then
				-- Players without a team have team number 0. To distinguish between them, we use a special key.
				return "player" .. p.number
			else
				return p.team
			end
		end

		local artifact_fields = {}
		local map = wl.Game().map

		local i = 1
		-- find all artifacts
		for x=0, map.width-1 do
			for y=0, map.height-1 do
				local field = map:get_field(x,y)
					if field.immovable and field.immovable:has_attribute("artifact") then
						-- this assumes that the immovable has size small or medium, i.e. only occupies one field
						artifact_fields[i] = map:get_field(x,y)
						i = i + 1
					end
			end
		end

		local plrs = wl.Game().players
		if #artifact_fields == 0 then
			for idx, plr in ipairs(plrs) do
				send_message(plr, _"No Artifacts", rt(p(_"There are no artifacts on this map. Please make sure you choose a suitable map.")), {popup = true})
			end
			return
		end

		-- Iterate all players, if one is defeated, remove him
		-- from the list, send him a defeated message and give him full vision
		-- Check if all artifacts have been found (i.e. controlled by a player)
		repeat
			sleep(5000)
			check_player_defeated(plrs, lost_game.title, lost_game.body, wc_descname, wc_version)
			local all_artifacts_found = true
			for idx, f in ipairs(artifact_fields) do
				if not f.owner then
					all_artifacts_found = false
					break
				end
			end
		until all_artifacts_found

		-- All artifacts are found, the game is over.
		local artifacts_per_team = {}
		for idx, p in ipairs(plrs) do
			artifacts_per_team[_getkey(p)] = 0
		end

		for idx, f in ipairs(artifact_fields) do
			local key = _getkey(f.owner)
			artifacts_per_team[key] = artifacts_per_team[key] + 1
		end

		local function _max(a)
			local max = -1
			for k, v in pairs(a) do
				if v > max then max = v end
			end
			return max
		end

		local max_artifacts = _max(artifacts_per_team)

		local function _get_member_names(t)
			local s = ""
			for idx, p in ipairs(t) do
				if s == "" then
					s = p.name
				else
					-- TRANSLATORS: This is used to seperate players’ names in a list, e.g. "Steve, Robert, David"
					s = s .. _", " .. p.name
				end
			end
			return s
		end

		local teams = {}
		local msg = _"Overview:" .. "\n"
		for idx, p in ipairs(plrs) do
			if p.team == 0 then
				local artifacts = (ngettext("%i artifact", "%i artifacts", artifacts_per_team[_getkey(p)])):format(artifacts_per_team[_getkey(p)])
				msg = msg .. "\n" .. (_"%1$s owns %2$s."):bformat(p.name, artifacts)
			else
				if teams[p.team] then
					teams[p.team][#teams[p.team]+1] = p
				else
					teams[p.team] = {p}
				end
			end
		end
		for idx, t in ipairs(teams) do
			local members = _get_member_names(t)
			local artifacts = (ngettext("%i artifact", "%i artifacts", artifacts_per_team[_getkey(t[1])])):format(artifacts_per_team[_getkey(t[1])])
			-- TRANSLATORS: %1$i is the team's number; %2$s is a list of team members (e.g. "Steve, Robert, David")
			-- TRANSLATORS: %3$s is something like "x artifact(s)"
			msg = msg .. "\n" .. (_"Team %1$i (%2$s) owns %3$s."):bformat(t[1].team, members, artifacts)
		end


		for idx, p in ipairs(plrs) do
			local key = _getkey(p)
			-- If two or more teams have the same amount of artifacts, they are all considered winners.
			if artifacts_per_team[key] == max_artifacts then
				p:send_message(won_game_over.title, won_game_over.body .. "\n\n" .. msg)
				wl.game.report_result(p, 1, make_extra_data(p, wc_descname, wc_version, {score=artifacts_per_team[key]}))
			else
				p:send_message(lost_game_over.title, lost_game_over.body .. "\n\n" .. msg)
				wl.game.report_result(p, 0, make_extra_data(p, wc_descname, wc_version, {score=artifacts_per_team[key]}))
			end
		end
	end,
}

