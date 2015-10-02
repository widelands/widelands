-- =======================================================================
--                         Collectors Win condition
-- =======================================================================

include "scripting/coroutine.lua" -- for sleep
include "scripting/messages.lua"
include "scripting/formatting.lua"
include "scripting/format_scenario.lua"
include "scripting/table.lua"
include "scripting/win_condition_functions.lua"

set_textdomain("win_conditions")

include "scripting/win_condition_texts.lua"

local wc_name = "Collectors"
-- This needs to be exactly like wc_name, but localized, because wc_name
-- will be used as the key to fetch the translation in C++
local wc_descname = _("Collectors")
local wc_version = 2
local wc_desc = _ (
	"You get points for precious wares in your warehouses. The player with " ..
	"the highest number of wares at the end of 4 hours wins the game."
)
local wc_points = _"Points"
return {
	name = wc_name,
	description = wc_desc,
	func = function()

   -- set the objective with the game type for all players
   broadcast_objective("win_condition", wc_descname, wc_desc)

   -- Simple flowing text. One Paragraph
   local function p(s)
      return "<p line-spacing=3 font-size=12>" .. s .. "<br></p>" ..
         "<p font-size=8> <br></p>"
   end


   -- The list of wares that give points
   local point_table = {
      barbarians = {
         gold = 3,
         ax = 2,
         sharpax = 3,
         broadax = 4,
         bronzeax = 4,
         battleax = 6,
         warriorsax = 10,
         helm = 2,
         mask = 3,
         warhelm = 6,
      },
      barbarians_order = {
         "gold", "ax", "sharpax", "broadax", "bronzeax", "battleax",
         "warriorsax", "helm", "mask", "warhelm",
      },

      empire = {
         gold = 3,
         wood_lance = 1,
         lance = 3,
         advanced_lance = 4,
         heavy_lance = 7,
         war_lance = 8,
         helm = 2,
         armor = 3,
         chain_armor = 4,
         plate_armor = 8,
      },
      empire_order = {
         "gold", "wood_lance", "lance", "advanced_lance", "heavy_lance",
         "war_lance", "helm", "armor", "chain_armor", "plate_armor"
      },

      atlanteans = {
         gold = 3,
         light_trident = 2,
         long_trident = 3,
         steel_trident = 4,
         double_trident = 7,
         heavy_double_trident = 8,
         steel_shield = 4,
         advanced_shield = 7,
         tabard = 1,
         golden_tabard = 5,
      },
      atlanteans_order = {
         "gold", "light_trident", "long_trident", "steel_trident",
         "double_trident", "heavy_double_trident", "steel_shield",
         "advanced_shield", "tabard", "golden_tabard"
       },
   }

   -- Calculate the momentary points for one player
   local function _calc_points(plr)
      set_textdomain("win_conditions")
      local bs = array_combine(
         plr:get_buildings("headquarters"), plr:get_buildings("warehouse"), plr:get_buildings("port")
      )

      local points = 0
      local descr = { "</p>" .. h2((_"Status for %s"):format(plr.name)) .. "<p line-spacing=3 font-size=12>"}
      for idx, ware in ipairs(point_table[plr.tribe_name .. "_order"]) do
         local value = point_table[plr.tribe_name][ware]
         local count = 0
         for idx,b in ipairs(bs) do
            count = count + b:get_wares(ware)
         end
         local lpoints = count * value
         points = points + lpoints
         local warename = wl.Game():get_ware_description(plr.tribe_name, ware).descname
         -- TRANSLATORS: For example: 'gold (3 P) x 4 = 12 P", P meaning "Points'
         descr[#descr+1] = listitem_bullet(_"%1$s (%2$i P) x %3$i = %4$i P"):bformat(
            warename, value, count, lpoints
         )
      end
      descr[#descr+1] =  "</p>" .. h3(ngettext("Total: %i point", "Total: %i points", points)):format(points)
              .. "<p line-spacing=3 font-size=12>"
      return points, p(table.concat(descr, "\n"))
   end

   local plrs = wl.Game().players
   -- Send all players the momentary game state
   local function _send_state(remaining_time, plrs)
      set_textdomain("win_conditions")
      local h = math.floor(remaining_time / 60)
      local m = remaining_time % 60
      -- TRANSLATORS: Context: 'The game will end in (2 hours and) 30 minutes.'
      local time = ""
      if m > 0 then
			time = (ngettext("%i minute", "%i minutes", m)):format(m)
		end
      if h > 0 then
			if m > 0 then
				-- TRANSLATORS: Context: 'The game will end in 2 hours and 30 minutes.'
				time = (ngettext("%1% hour and %2%", "%1% hours and %2%", h)):bformat(h, time)
			else
				-- TRANSLATORS: Context: 'The game will end in 2 hours.'
				time = (ngettext("%1% hour", "%1% hours", h)):bformat(h)
			end
      end
      -- TRANSLATORS: Context: 'The game will end in 2 hours and 30 minutes.'
      local msg = p(_"The game will end in %s."):format(time)
      msg = msg .. "\n\n"

      for idx, plr in ipairs(plrs) do
         local points, pstat = _calc_points(plr)

         msg = msg .. pstat
      end

      for idx, plr in ipairs(plrs) do
			send_message(plr, game_status.title,  "<rt>" .. msg .. "</rt>", {popup = true})
      end
   end

   local function _game_over(plrs)
      local points = {}
      for idx,plr in ipairs(plrs) do
         points[#points + 1] = { plr, _calc_points(plr) }
      end
      table.sort(points, function(a,b) return a[2] < b[2] end)
      for i=1,#points-1 do
         points[i][1]:send_message(lost_game_over.title, lost_game_over.body)
         wl.game.report_result(points[i][1], 0, make_extra_data(points[i][1], wc_descname, wc_version, {score=points[i][2]}))
      end
      points[#points][1]:send_message(won_game_over.title, won_game_over.body)
      wl.game.report_result(points[#points][1], 1, make_extra_data(points[#points][1], wc_descname, wc_version, {score=points[#points][2]}))
   end

   -- Instantiate the hook to calculate points
   hooks.custom_statistic = {
      name = wc_points,
      pic = "pics/genstats_points.png",
      calculator = function(p)
         local pts = _calc_points(p)
         return pts
      end,
   }

   sleep(1000)

   local remaining_time = 60 * 4

   -- Endless loop
   while true do
      _send_state(remaining_time, plrs)

      -- Game ended?
      if remaining_time <= 0 then
         _game_over(plrs)
         break
      end

      local runs = 0
      repeat
         sleep(5000)
         check_player_defeated(plrs, lost_game.title, lost_game.body, wc_descname, wc_version)
         runs = runs + 1
      until runs >= 120 -- 120 * 5000ms = 600000 ms = 10 minutes
      remaining_time = remaining_time - 10
   end
end
}
