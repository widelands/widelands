-- =======================================================================
--              Minimum Starting Conditions for Atlanteans
-- =======================================================================

include "scripting/infrastructure.lua"

push_textdomain("tribes")

local r = {
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

      prefilled_buildings(player, { "atlanteans_warehouse", sf.x, sf.y,
         wares = {
            log = 2,
            planks = 2,
            corn = 3,
            hammer = 1,
            --Woodcutter, sawyer, toolsmith:
            saw = 3,
            pick = 1,
            shovel =1 ,
            milking_tongs = 1,
            -- Smelter:
            iron = 1,
            -- Baker, blackroot farmer, farmer, geologist, 3x miner, smoker:
            iron_ore = 8
         },
         soldiers = {
            [{0,0,0,0}] = 1,
         }
      })

      player:reveal_fields(sf:region(10))
      player:conquer(sf, 9)

      push_textdomain("tribes")
      player:send_to_inbox(_"Be careful", _"You have only one iron for each tool you will need to start your economy. Make sure no unneeded tool is created.")
      pop_textdomain()

      check_trees_rocks_poor_hamlet(player, sf, "atlanteans_warehouse", {granite = 4, planks = -1}, {log = 1, planks = 1, granite = 1})
   end
}
pop_textdomain()
return r
