-- =======================================================================
--       Minimum (Poor Hamlet) starting conditions for Barbarians
-- =======================================================================

include "scripting/infrastructure.lua"

push_textdomain("tribes")

init = {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _("Poor Hamlet"),
   -- TRANSLATORS: This is the tooltip for the "Poor Hamlet" (minimum) starting condition
   tooltip = _("Start the game with just a warehouse and the bare minimum for bootstrapping an economy. Warning: the AI can’t successfully start from this."),
   incompatible_wc = {"scripting/win_conditions/hq_hunter.lua"},

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
            -- Forester, gardener:
            shovel = 2,
            -- Smelter:
            iron = 1,
            -- Geologist, fisher/hunter, innkeeper, miner:
            iron_ore = 4
         },
         soldiers = {
            [{0,0,0,0}] = 1,
         }
      })

      player:reveal_fields(sf:region(10))
      player:conquer(sf, 9)

      push_textdomain("tribes")
      player:send_to_inbox(_("Be careful"), _("You have only one iron for each tool you will need to start your economy. Make sure no unneeded tool is created."))
      pop_textdomain()

      check_trees_rocks_poor_hamlet(player, sf, "barbarians_warehouse", {granite = 1}, {log = 4})
   end
}
pop_textdomain()
return init
