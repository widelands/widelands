-- =======================================================================
--                Minimum Starting conditions for Empire
-- =======================================================================

include "scripting/infrastructure.lua"

set_textdomain("tribes")

return {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _ "Poor Hamlet",
   -- TRANSLATORS: This is the tooltip for the "Poor Hamlet" (minimum) starting condition
   tooltip = _"Start the game with just a warehouse and the bare minimum for bootstrapping an economy. Warning: the AI can't successfully start from this",
   func =  function(player, shared_in_start)
      local sf = wl.Game().map.player_slots[player.number].starting_field
      if shared_in_start then
         sf = shared_in_start
      else
         player:allow_workers("all")
      end

      prefilled_buildings(player, { "empire_warehouse", sf.x, sf.y,
         wares = {
            granite = 1,
            log = 2,
            planks = 2,
            hammer = 2,
            felling_ax = 1,
            saw = 1,
            pick = 1,
            shovel = 1,
            --Smelter:
            iron = 1,
            --Farmer, fisher/hunter, geologist, innkeeper, miner, vinefarmer (x2):
            iron_ore = 7
         },
         soldiers = {
            [{0,0,0,0}] = 1
         }
      })

      player:reveal_fields(sf:region(10))
      player:conquer(sf, 9)
      player:send_message(_"Be careful.", _"You have only one iron for each tool you will need to start your economy. Make sure no unneeded tool is created.")

      local function add_wares(waretable)
         local hq = player:get_buildings("empire_warehouse")[1]
         for ware,warecount in pairs(waretable) do
            local oldwarecount = hq:get_wares(ware) or 0
            hq:set_wares(ware, oldwarecount+warecount)
         end
      end

      --NOTE: pessimistically, this could be a single rock
      local has_rocks = false
      local has_trees = false
      for k,f in pairs(sf:region(10)) do
         if f.immovable then 
            if f.immovable:has_attribute('rocks') then
               has_rocks = true
            elseif f.immovable:has_attribute('tree') then
               has_trees = true
            end
            if has_trees and has_rocks then
               break
            end
         end
      end
      if not has_rocks then
         add_wares({granite = 1})
         player:send_message(_"No rocks nearby", _"There are no rocks near to your starting position.  Therefore, you receive extra resources for bootstrapping your economy.")
      end
      -- adding exactly one forester
      if not has_trees then
         add_wares({log = 1, planks = 1, granite = 1})
         player:send_message(_"No trees nearby", _"There are no trees near to your starting position.  Therefore, you receive extra resources for bootstrapping your economy.")
      end
   end
}
