-- RST
-- Headquarters, Warehouses and Ports
-- ----------------------------------
--
-- Warehouses are a type of building where wares and workers are being stored
-- that aren't currently needed at another building.
-- Warehouses can also be one of two special types: a headquarters or a port.
-- A *headquarters* is also the tribe's main building.
-- A *port* can only be built on a designated port space and allows transferring
-- wares by sea and starting expeditions.
--
-- Warehouses are defined in
-- ``data/tribes/buildings/warehouses/<tribe_name>/<building_name>/init.lua``.
-- The building will also need its help texts, which are defined in
-- ``data/tribes/buildings/warehouses/<tribe_name>/<building_name>/helptexts.lua``

dirname = path.dirname(__file__)

-- RST
-- .. function:: new_warehouse_type(table)
--
--    This function adds the definition of a warehouse building to the engine.
--
--    :arg table: This table contains all the data that the game engine will add to this building.
--                It contains the following entries:
--
--    **msgctxt**: The context that Gettext will use to disambiguate the translations for strings in this table.
--
--    **name**: A string containing the internal name of this building.
--
--    **descname**: The translatable display name. Use ``pgettext`` with the ``msgctxt`` above to fetch the string.
--
--    **helptext_script**: The full path to the ``helptexts.lua`` script for this building.
--
--    **icon**: The full path to the menu icon for this building.
--
--    **size**: The size of this building: ``"small"``, ``"medium"``, ``"big"`` or ``"port"``.
--
--    **destructible**: Set this to ``false`` for headquarters, leave blank otherwise.
--
--    **buildcost**: A table with the wares used to build this building, containing warename - amount pairs, e.g. ``buildcost = { log = 4, granite = 2 }``. If this table isn't defined, the building will be a headquarters.
--
--    **return_on_dismantle**: The wares that this building will give back to the player when it is dismantled, e.g. ``buildcost = { log = 2, granite = 1 }``. If this table isn't defined, the building will be a headquarters.
--
--    **enhancement_cost**: The wares needed to upgrade this building, e.g. ``enhancement_cost = { log = 1, granite = 1 }``. If this table isn't defined, the building will be a headquarters.
--
--    **return_on_dismantle_on_enhanced**: The wares that this enhanced building will give back to the player when it is dismantled, e.g. ``return_on_dismantle_on_enhanced = { granite = 1 }``. If this table isn't defined, the building will be a headquarters.
--
--    **animations**: A table containing all animations for this building.
--
--    **aihints**: A list of name - value pairs with hints for the AI.
--
--    **heal_per_second**: The number of health points that a garrisoned soldier will heal each second.
--
--    **conquers**: The conquer radius for this building. This is used for headquarters and ports.
--
tribes:new_warehouse_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_headquarters",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Headquarters"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",
   destructible = false,

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 81, 110 },
      },
   },

   aihints = {},

   heal_per_second = 220,
   conquers = 9,
}
