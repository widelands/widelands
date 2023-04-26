-- =======================================================================
--         Minimum (Poor Hamlet) starting conditions for Amazons
-- =======================================================================

include "scripting/infrastructure.lua"

push_textdomain("tribes")

init = {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _("Poor Hamlet"),
   -- TRANSLATORS: This is the tooltip for the "Poor Hamlet" (minimum) starting condition
   tooltip = _("Start the game with just a warehouse and the bare minimum for bootstrapping an economy. Warning: the AI can’t successfully start from this."),
   incompatible_wc = {"scripting/win_conditions/hq_hunter.lua"},

   func =  function(player, shared_in_start)
      local sf = wl.Game().map.player_slots[player.number].starting_field
      if shared_in_start then
         sf = shared_in_start
      else
         player:allow_workers("all")
      end

      prefilled_buildings(player, { "amazons_warehouse", sf.x, sf.y,
         wares = {
            log = 4,
            rubber = 1,
            --Woodcutter, stonecutter, jungle preserver, liana cutter, stonecarver:
            felling_ax = 1,
            pick = 1,
            shovel =1 ,
            machete = 1,
            hammer = 2,
            chisel = 1,
         },
         soldiers = {
            [{0,0,0,0}] = 1,
         }
      })

      player:reveal_fields(sf:region(10))
      player:conquer(sf, 9)
      player:send_to_inbox(_("Be careful"), _("You only have exactly the wares for each tool you will need to start your economy. Make sure no unneeded tool is created."))

      check_trees_rocks_poor_hamlet(player, sf, "amazons_warehouse", {granite = 3}, {log = 3, rope = 1})
   end
}

pop_textdomain()
return init
