-- RST
-- buildings/militarysites/<name>/init.lua
-- ---------------------------------------
--
-- Militarysites are a type of building that can house soldiers and that expand the players' terrain.
-- They can also be attacked. This file contains the definition for a militarysite building.
-- The building will also need its helptexts, which are defined in ``helptexts.lua``

dirname = path.dirname(__file__)

-- RST
-- .. function:: new_militarysite_type(table)
--
--    This function adds the definition of a military site building to the engine.
--
--    :arg table: This table contains all the data that the game engine will add to this building.
--                It contains the following entries:
--
--    **msgctxt**: The context that Gettext will use to disambiguate the translations for strings in this table.
--
--    **name**: A string containing the internal name of this building.
--
--    **descname**: The translatable display name. use ``pgettext`` with the ``msgctxt`` above to fetch the string.
--
--    **helptext_script**: The full path to the ``helptexts.lua`` script for this building.
--
--    **icon**: The full path to the menu icon for this building.
--
--    **size**: The size of this building: ``"small"``, ``"medium"``, or ``"big"``.
--
--    **buildcost**: A table with the wares used to build this building, containing warename - amount pairs, e.g. ``buildcost = { log = 4, granite = 2 }``.
--
--    **return_on_dismantle**: The wares that this building will give back to the player when it is dismantled, e.g. ``buildcost = { log = 2, granite = 1 }``.
--
--    **enhancement_cost**: The wares needed to upgrade this building, e.g. ``enhancement_cost = { log = 1, granite = 1 }``.
--
--    **return_on_dismantle_on_enhanced**: The wares that this enhanced building will give back to the player when it is dismantled, e.g. ``return_on_dismantle_on_enhanced = { granite = 1 }``.
--
--    **animations**: A table containing all animations for this building.
--
--    **aihints**: A list of name - value pairs with hints for the AI.
--
--    **max_soldiers**: The maximum number of solders that can be garrisoned at this building.
--
--    **heal_per_second**: The number of health points that a garrisoned solder will heal each second.
--
--    **conquers**: The conquer radius for this building.
--
--    **prefer_heroes**: Whether this building prefers heroes or rookies per default. Set to ``true`` or ``false``.
--
--    **messages**: Messages that this building will send to the user. The following messages need to ne defined:
--         * *occupied*: Shown when your soldiers occupy a new building
--         * *aggressor*: Shown when this building sees enemy soldiers
--         * *attack*: Shown when this building is being attacked by an enemy
--         * *defeated_enemy*: Shown when you lose this building to an enemy
--         * *defeated_you*: Shown when you take this building from an enemy
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
