-- RST
-- .. _lua_tribes_tribes_start_conditions:
--
-- Starting Conditions
-- ===================
--
-- The starting conditions a tribe can use are listed in the tribe’s :ref:`init.lua <lua_tribes_tribes_init>` file.
--
-- They are defined in their definition files named as follows and located in
-- ``data/tribes/initialization/<tribe_name>/starting_conditions/<starting_condition_name>.lua``.
--
-- .. function:: init
--
--    This function initializes the starting condition.
--
--    **descname**: The localized name of the starting condition.
--
--    **tooltip**: A short description of the starting condition. Should contain essential hints if necessary.
--
--    **map_tags**: *Optional* A table with map tags that are required for this starting condition,
--                  e.g. ``{ "seafaring" }`` if the starting conditions needs a seafaring map.
--                  If present, the starting condition can only be selected by the user if the map contains *all* listed tags.
--
--    **incompatible_wc**: *Optional* A table with strings containing the paths of win conditions that are incompatible with this
--                         starting condition, e.g. ``{ "scripting/win_conditions/hq_hunter.lua" }`` if the starting condition
--                         is incompatible with the win condition "HQ Hunter".
--
--    **uses_map_starting_position**: *Optional* Whether this starting condition considers the player's starting position as
--                                    defined by the map; that is, this value should be ``true`` for starting conditions which
--                                    give the player some initial buildings, and ``false`` for starting conditions such as
--                                    "Discovery" and "New World". If all players use a starting condition for which this
--                                    value is ``false``, the checkbox for Custom Starting Positions mode is disabled. If at
--                                    least one player uses a starting condition for which this value is ``true``, the
--                                    checkbox for Custom Starting Positions mode is enabled, but the setting has no effect for
--                                    players whose starting condition defines this value as ``false``. Defaults to ``true``.
--
--    **func**: A standardized function to determine whether to share this starting condition between players (Shared Kingdom).
--              It needs to be declared like this: ``func = function(player, shared_in_start)``.
--              Later in the code it is necessary to use the result (variable ``shared_in_start``) to share the starting field
--              together with the starting condition.
--              Example::
--
--                 if shared_in_start then
--                    sf = shared_in_start
--                 else
--                    player:allow_workers("all")
--                 end
--
-- After these declarations the Lua function calls to create the initial map objects for the player follow.
--
-- To make the UI texts translatable, we also need to push/pop the correct textdomain.
--
-- Example:
--
-- .. code-block:: lua
--
--    include "scripting/starting_conditions.lua"
--
--    push_textdomain("tribes")
--
--    init = {
--       -- TRANSLATORS: This is the name of a starting condition
--       descname = _("Discovery"),
--       -- TRANSLATORS: This is the tooltip for the "Discovery" starting condition
--       tooltip = _("Start the game with three ships on the ocean and only a handful of supplies"),
--       map_tags = {"seafaring"},
--       incompatible_wc = {"scripting/win_conditions/hq_hunter.lua"},
--       uses_map_starting_position = false,
--
--       func = function(player, shared_in_start)
--
--       local map = wl.Game().map
--       local sf = map.player_slots[player.number].starting_field
--       if shared_in_start then
--          sf = shared_in_start
--       else
--          player:allow_workers("all")
--       end
--
--       launch_expeditions(player, {
--          {
--             log = 4,
--             granite = 1,
--             planks = 2,
--             spidercloth = 3,
--             atlanteans_woodcutter = 3,
--             atlanteans_forester = 2,
--             atlanteans_soldier = 1,
--          },
--          {
--             log = 3,
--             granite = 4,
--             planks = 2,
--             spidercloth = 3,
--             atlanteans_stonecutter = 2,
--             atlanteans_soldier = 1,
--          },
--          {
--             iron = 2,
--             atlanteans_soldier = 1,
--             atlanteans_geologist = 1,
--             atlanteans_miner = 3,
--             atlanteans_smelter = 2,
--             atlanteans_toolsmith = 1,
--             atlanteans_baker = 2,
--             atlanteans_smoker = 2,
--             atlanteans_fisher = 1,
--          },
--       })
--    end
--    }
--
--    pop_textdomain()
--    return init


-- =======================================================================
--                 Discovery starting conditions for Atlanteans
-- =======================================================================

include "scripting/starting_conditions.lua"

push_textdomain("tribes")

init = {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _("Discovery"),
   -- TRANSLATORS: This is the tooltip for the "Discovery" starting condition
   tooltip = _("Start the game with three ships on the ocean and only a handful of supplies"),
   map_tags = {"seafaring"},
   incompatible_wc = {"scripting/win_conditions/hq_hunter.lua"},
   uses_map_starting_position = false,

   func = function(player, shared_in_start)

   local map = wl.Game().map
   local sf = map.player_slots[player.number].starting_field
   if shared_in_start then
      sf = shared_in_start
   else
      player:allow_workers("all")
   end

   launch_expeditions(player, {
      {
         log = 4,
         granite = 1,
         planks = 2,
         spidercloth = 1,
         iron = 2,
         atlanteans_woodcutter = 3,
         atlanteans_forester = 2,
         atlanteans_soldier = 1,
      },
      {
         log = 3,
         granite = 4,
         planks = 2,
         spidercloth = 3,
         atlanteans_stonecutter = 2,
         atlanteans_soldier = 1,
      },
      {
         iron = 2,
         atlanteans_soldier = 1,
         atlanteans_geologist = 1,
         atlanteans_miner = 3,
         atlanteans_smelter = 2,
         atlanteans_toolsmith = 1,
         atlanteans_baker = 2,
         atlanteans_smoker = 2,
         atlanteans_fisher = 1,
      },
   })
end
}

pop_textdomain()
return init
