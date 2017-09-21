-- =======================================================================
--                 Minimum Starting conditions for Barbarians
-- =======================================================================

include "scripting/infrastructure.lua"

set_textdomain("tribes")

return {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _ "Minimum",
   -- TRANSLATORS: This is the tooltip for the "Minimum" starting condition
   tooltip = _"Start the game with just a warehouse and the bare minimum for bootstrapping an economy. Warning: the AI can't successfully start from this",
   func = function(player, shared_in_start)
      local sf = wl.Game().map.player_slots[player.number].starting_field
      if shared_in_start then
         sf = shared_in_start
      else
         player:allow_workers("all")
      end
      
      prefilled_buildings(player, { "barbarians_warehouse", sf.x, sf.y,
         wares = {
            log = 3,
            coal = 1,
            hammer = 2,
            felling_ax = 1,
            pick = 1,
            --Forester, gardener:
            shovel = 2,
            --Smelter:
            iron = 1,
            --Geologist, fisher/hunter, innkeeper, miner:
            iron_ore = 4
         },
         soldiers = {
            [{0,0,0,0}] = 1,
         }
      })
      
      player:reveal_fields(sf:region(9))
      player:conquer(sf, 9)
   end
}
