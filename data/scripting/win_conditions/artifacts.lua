-- =======================================================================
--                         Artifact condition
-- =======================================================================

include "scripting/coroutine.lua" -- for sleep
include "scripting/win_conditions/win_condition_functions.lua"

push_textdomain("win_conditions")

include "scripting/win_conditions/win_condition_texts.lua"

local wc_name = "Artifacts"
-- This needs to be exactly like wc_name, but localized, because wc_name
-- will be used as the key to fetch the translation in C++
local wc_descname = _("Artifacts")
local wc_version = 1
local wc_desc = _("Search for ancient artifacts. Once all of them are found, the team who owns most of them will win the game.")
local wc_artifacts = "Artifacts owned"
-- This needs to be exactly like wc_artifacts, but localized, because wc_artifacts
-- will be used as the key to fetch the translation in C++
local wc_artifacts_i18n = _("Artifacts owned")

-- Table of all artifacts to conquer
local artifact_fields = {}

local r = {
   name = wc_name,
   description = wc_desc,
   peaceful_mode_allowed = true,
   map_tags = { "artifacts" }, -- Map tags needed so that this win condition will be available only for suitable maps
   init = function()
      -- Find all artifacts
      local map = wl.Game().map
      for x=0, map.width-1 do
         for y=0, map.height-1 do
            local field = map:get_field(x,y)
            if field.immovable and field.immovable:has_attribute("artifact") then
               -- This assumes that the immovable has size small or medium, i.e. only occupies one field
               table.insert(artifact_fields, map:get_field(x,y))
            end
         end
      end
   end,
   func = function()
      push_textdomain("win_conditions")
      -- set the objective with the game type for all players
      broadcast_objective("win_condition", wc_descname, wc_desc)

      local function _getkey(plr)
         if plr.team == 0 then
            -- Players without a team have team number 0. To distinguish between them, we use a special key.
            return "player" .. plr.number
         else
            return plr.team
         end
      end

      local artifacts_owner = {}
      local plrs = wl.Game().players

      -- statistic variables and functions
      -- initializing artifacts owned table
      local artifacts_per_player = {}
      -- funtion to calculate actual number of owned artifacts per player
      local function _calcowned()
         for idx, plr in ipairs(wl.Game().players) do
            artifacts_per_player[plr.number] = 0
         end
         for idx, plr in pairs(artifacts_owner) do
         artifacts_per_player[plr.number] = artifacts_per_player[plr.number] + 1
         end
      end

      if #artifact_fields == 0 then
         for idx, plr in ipairs(plrs) do
            send_to_inbox(plr, _("No Artifacts"), p(_("There are no artifacts on this map. This should not happen. Please file a bug report on %s and specify your Widelands version and the map you tried to load.")):bformat("https://www.widelands.org/wiki/ReportingBugs/"), {popup = true})
         end
         return
      end

      local found_artifact = {
         -- TRANSLATORS: Keep this as short as possible. You can also translate this as "New artifact"
         title = _("Artifact Found"),
         body_team = p(_([[Your team found a new artifact.]])),
         body_single = p(_([[You found a new artifact.]]))
      }
      local lost_artifact = {
         -- TRANSLATORS: Keep this as short as possible.
         title = _("Artifact Lost"),
         body_team = p(_([[One of your team’s artifacts was stolen by an enemy.]])),
         body_single = p(_([[One of your artifacts was stolen by an enemy.]]))
      }
      local stole_artifact = {
         -- TRANSLATORS: Keep this as short as possible.
         title = _("Artifact Conquered"),
         body_team = p(_([[Your team stole an artifact from an enemy.]])),
         body_single = p(_([[You stole an artifact from an enemy.]]))
      }

      local function _broadcast_to_team(player, msg, f)
         if player.team == 0 then
            player:send_to_inbox(msg.title, msg.body_single, {msg, field = f})
         else
            for idx, plr in ipairs(plrs) do
               if plr.team == player.team then
                  plr:send_to_inbox(msg.title, msg.body_team, {msg, field = f})
               end
            end
         end
      end


      -- Install statistics hook
      hooks.custom_statistic = {
         name = wc_artifacts,
         pic = "images/wui/stats/genstats_artifacts.png",
         calculator = function(p)
            _calcowned(p)
            return artifacts_per_player[p.number] or 0
         end,
      }

      -- Iterate all players, if one is defeated, remove him
      -- from the list, send him a defeated message and give him full vision
      -- Check if all artifacts have been found (i.e. controlled by a player)

      repeat
         sleep(1000)
         check_player_defeated(plrs, lost_game.title, lost_game.body, wc_descname, wc_version)
         local all_artifacts_found = true
         for idx, f in ipairs(artifact_fields) do
            if f.owner then
               if artifacts_owner[f] then
                  if f.owner.team ~= artifacts_owner[f].team or (f.owner.team == 0 and f.owner.number ~= artifacts_owner[f].number) then
                     -- a new team has this artifact
                     _broadcast_to_team(f.owner, stole_artifact, f)
                     _broadcast_to_team(artifacts_owner[f], lost_artifact, f)
                     artifacts_owner[f] = f.owner
                  end
               else
                  _broadcast_to_team(f.owner, found_artifact, f)
                  artifacts_owner[f] = f.owner
               end
            else
               all_artifacts_found = false
            end
         end
      until all_artifacts_found

      -- All artifacts are found, the game is over.
      local artifacts_per_team = {}
      for idx, plr in ipairs(wl.Game().players) do
         artifacts_per_team[_getkey(plr)] = 0
      end

      for idx, plr in pairs(artifacts_owner) do
         local key = _getkey(plr)
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
         local membernames = {}
         for idx, plr in ipairs(t) do
            table.insert(membernames, plr.name)
         end
         return localize_list(membernames, ",")
      end

      local teams = {}
      local msg = h3(_("Overview:"))
      for idx, plr in ipairs(plrs) do
         if plr.team == 0 then
            local artifacts = (ngettext("%i artifact", "%i artifacts", artifacts_per_team[_getkey(plr)])):format(artifacts_per_team[_getkey(plr)])
            -- TRANSLATORS: e.g. Team 1 owns 5 artifacts.
            msg = msg .. p((_("%1$s owns %2$s.")):bformat(plr.name, artifacts))
         else
            if teams[plr.team] then
               teams[plr.team][#teams[plr.team]+1] = plr
            else
               teams[plr.team] = {plr}
            end
         end
      end
      for idx, t in ipairs(teams) do
         local members = _get_member_names(t)
         local artifacts = (ngettext("%i artifact", "%i artifacts", artifacts_per_team[_getkey(t[1])])):format(artifacts_per_team[_getkey(t[1])])
         -- TRANSLATORS: %1$i is the team's number; %2$s is a list of team members (e.g. "Steve, Robert, David"), %3$s is something like "x artifact(s)"
         msg = msg .. p((_("Team %1$i (%2$s) owns %3$s.")):bformat(t[1].team, members, artifacts))
      end

      for idx, plr in ipairs(plrs) do
         local key = _getkey(plr)
         -- If two or more teams have the same amount of artifacts, they are all considered winners.
         if artifacts_per_team[key] == max_artifacts then
            plr:send_to_inbox(won_game_over.title, won_game_over.body .. msg)
            wl.game.report_result(plr, 1, make_extra_data(plr, wc_descname, wc_version, {score=artifacts_per_team[key]}))
         else
            plr:send_to_inbox(lost_game_over.title, lost_game_over.body .. msg)
            wl.game.report_result(plr, 0, make_extra_data(plr, wc_descname, wc_version, {score=artifacts_per_team[key]}))
         end
      end

   pop_textdomain()
   end,
}
pop_textdomain()
return r
