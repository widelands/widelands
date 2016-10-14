-- RST
-- Military Sites
-- --------------
--
-- Militarysites are a type of building that can house soldiers and that expand the players' terrain.
-- They can also be attacked.
--
-- Militarysites are defined in
-- ``data/tribes/buildings/militarysites/<tribe_name>/<building_name>/init.lua``.
-- The building will also need its help texts, which are defined in
-- ``data/tribes/buildings/militarysites/<tribe_name>/<building_name>/helptexts.lua``

dirname = path.dirname(__file__)

-- RST
-- .. function:: new_militarysite_type{table}
--
--    This function adds the definition of a military site building to the engine.
--
--    :arg table: This table contains all the data that the game engine will add to this building.
--                It contains the following entries:
--
--    **msgctxt**
--        The context that Gettext will use to disambiguate the translations for
--        strings in this table, e.g.::
--
--            msgctxt = "atlanteans_building",
--
--    **name**
--        A string containing the internal name of this building, e.g.::
--
--            name = "atlanteans_fortress",
--
--    **descname**
--        The translatable display name. Use ``pgettext`` with the ``msgctxt``
--        above to fetch the string, e.g.::
--
--            descname = pgettext("atlanteans_building", "Fortress"),
--
--    **helptext_script**
--        The full path to the ``helptexts.lua`` script for this building.
--
--    **icon**
--        The full path to the menu icon for this building.
--
--    **size**
--        The size of this building: ``"small"``, ``"medium"``, or ``"big"``.
--
--    **buildcost**
--        A table with the wares used to build this building, containing
--        warename - amount pairs, e.g.::
--
--            buildcost = { log = 4, granite = 2 }
--
--    **return_on_dismantle**
--        The wares that this building will give back to the player when it is
--        dismantled, e.g.::
--
--            buildcost = { log = 2, granite = 1 }
--
--    **enhancement_cost**
--        The wares needed to upgrade this building, e.g.::
--
--            enhancement_cost = { log = 1, granite = 1 }
--
--    **return_on_dismantle_on_enhanced**
--        The wares that this enhanced building will give back to the player
--        when it is dismantled, e.g.::
--
--            return_on_dismantle_on_enhanced = { granite = 1 }
--
--    **animations**
--        A table containing all animations for this building. Every building
--        needs to have an ``idle`` animation. Other animations that a building
--        can have are ``build``, ``working``, ``unoccupied``. Mines also have
--        an ``unoccupied`` animation.
--        See :doc:`animations` for a detailed description of the animation format.
--
--    **aihints**
--        A list of name - value pairs with hints for the AI.
--
--    **max_soldiers**
--        The maximum number of solders that can be garrisoned at this building.
--
--    **heal_per_second**
--        The number of health points that a garrisoned soldier will heal each second.
--
--    **conquers**
--        The conquer radius for this building.
--
--    **prefer_heroes**
--        Whether this building prefers heroes or rookies per default.
--        Set to ``true`` or ``false``.
--
--    **messages**
--        Messages that this building will send to the user. There are 5 messages,
--        defined like this::
--
--            messages = {
--                occupied = _"Your soldiers have occupied your fortress.",
--                aggressor = _"Your fortress discovered an aggressor.",
--                attack = _"Your fortress is under attack.",
--                defeated_enemy = _"The enemy defeated your soldiers at the fortress.",
--                defeated_you = _"Your soldiers defeated the enemy at the fortress."
--            }
tribes:new_militarysite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_castle",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Castle"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      planks = 4,
      granite = 7,
      log = 5,
      spidercloth = 2,
      diamond = 1,
      quartz = 1
   },
   return_on_dismantle = {
      planks = 2,
      granite = 5,
      log = 2
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 91, 91 },
      }
   },

   aihints = {
      expansion = true,
      fighting = true,
      mountain_conqueror = true,
      prohibited_till = 1500
   },

   max_soldiers = 12,
   heal_per_second = 200,
   conquers = 12,
   prefer_heroes = true,

   messages = {
      occupied = _"Your soldiers have occupied your castle.",
      aggressor = _"Your castle discovered an aggressor.",
      attack = _"Your castle is under attack.",
      defeated_enemy = _"The enemy defeated your soldiers at the castle.",
      defeated_you = _"Your soldiers defeated the enemy at the castle."
   },
}
