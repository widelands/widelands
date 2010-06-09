-- =======================================================================
--                         Collectors Win condition
-- =======================================================================

use("aux", "coroutine") -- for sleep
use("aux", "table")

set_textdomain("win_conditions")

return {
   name = _ "Collectors",
   description = _ (
"You get points for wares in your HQ, the one with the highest number " .. 
"of wares at the end of 4 hours wins the game."
),
   func = function() 

-- Simple flowing text. One Paragraph
local function p(s)
   return "<p line-spacing=3 font-size=12>" .. s .. "<br></p>" ..
      "<p font-size=8> <br></p>"
end


-- The list of wares that give points 
local point_table = {
   barbarians = {
      gold = 3,
      axe = 2,
      sharpaxe = 3,
      broadaxe = 4,
      bronzeaxe = 4,
      battleaxe = 6,
      warriorsaxe = 10,
      helm = 2,
      mask = 3,
      warhelmet = 6,
   },
   barbarians_order = {
      "gold", "axe", "sharpaxe", "broadaxe", "bronzeaxe", "battleaxe",
      "warriorsaxe", "helm", "mask", "warhelmet",
   },

   empire = {
      gold = 3,
      wood_lance = 1,
      lance = 3,
      advanced_lance = 4,
      heavy_lance = 7,
      war_lance = 8,
      helm = 2,
      armour = 3,
      chain_armour = 4,
      plate_armour = 8,
   },
   empire_order = {
      "gold", "wood_lance", "lance", "advanced_lance", "heavy_lance",
      "war_lance", "helm", "armour", "chain_armour", "plate_armour"
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
   local bs = array_combine(
      plr:get_buildings("headquarters"), plr:get_buildings("warehouse")
   )

   local points = 0
   local descr = { _("Status for Player %i<br>"):format(plr.number) }
   for idx, ware in ipairs(point_table[plr.tribe .. "_order"]) do
      local value = point_table[plr.tribe][ware]
      local count = 0
      for idx,b in ipairs(bs) do
         count = count + b:get_wares(ware)
      end
      local lpoints = count * value
      points = points + lpoints
      descr[#descr+1] = ("  %s (%i) x %i = %i<br>"):format(
         ware, value, count, lpoints
      )
   end
   descr[#descr+1] = _("Total: %i"):format(points)

   return points, p(table.concat(descr, "\n"))
end

-- Send all players the momentary game state
local function _send_state(remaining_time, plrs)
   local msg = p(_"The game will end in %i minutes."):format(remaining_time)
   msg = msg .. "\n\n"

   for idx, plr in ipairs(plrs) do
      local points, pstat = _calc_points(plr)

      msg = msg .. pstat
   end

   for idx, plr in ipairs(plrs) do
      plr:send_message(_ "Status", "<rt>" .. msg .. "</rt>")
   end
end

local function _game_over(plrs)
   local points = {}
   for idx,plr in ipairs(plrs) do
      points[#points + 1] = { plr, _calc_points(plr) }
   end
   table.sort(points, function(a,b) return a[2] < b[2] end)
   for i=1,#points-1 do
      points[i][1]:send_message(_"You lost", _"You've lost this game!")
   end
   points[#points][1]:send_message(_"You won!", _"You are the winner!")
end

sleep(1000)


-- Find all valid players
local remaining_time = 60 * 4
local plrs = {}

for i=1,10 do
   if pcall(wl.game.Player, i) then
      plrs[#plrs+1] = wl.game.Player(i)
   end
end

-- Endless loop
while true do
   _send_state(remaining_time, plrs)
   
   -- Game ended?
   if remaining_time <= 0 then
      _game_over(plrs)
      break
   end

   sleep(10 * 60 * 1000) -- Sleep 10 minutes
   remaining_time = remaining_time - 10
end

end
}
