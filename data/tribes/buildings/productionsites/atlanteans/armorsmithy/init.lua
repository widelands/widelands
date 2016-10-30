-- RST
-- Production Sites
-- ----------------
--
-- Productionsites are a type of building where wares are being consumed to
-- produce other wares, or to produce workers.
-- They also have workers working at them.
--
-- Productionsites are defined in
-- ``data/tribes/buildings/productionsites/<tribe_name>/<building_name>/init.lua``.
-- The building will also need its help texts, which are defined in
-- ``data/tribes/buildings/productionsites/<tribe_name>/<building_name>/helptexts.lua``


dirname = path.dirname(__file__)

-- RST
-- .. function:: new_productionsite_type(table)
--
--    This function adds the definition of a production site building to the engine.
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
--    **size**: The size of this building: ``"small"``, ``"medium"``, ``"big"`` or ``"mine"``.
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
--    **working_positions**: The name and amount for each worker type working at this building.
--
--    **inputs**: The name and amount for each ware input queue.
--
--    **outputs**: The wares/workers produced by this building.
--
--    **programs**: The production site programs that define what preconditions
--    a building needs to fulfil in order to produce its wares and how it's done,
--    including any animations and sounds played. See also :doc:`productionsite_program`.
--
--    **out_of_resource_notification**: This optional table defines the message
--    sent by the productionsite to the player if it has run out of a resource to
--    collect. Table keys are:
--
--        *title*: A concise message title for the list in the inbox.
--
--        *heading*: A longer version of the title, shown in the message body.
--
--        *message*: The actual message. Translation needs to be fetched with pgettext.
--
--        *productivity_threshold*: An optional int value in percent to trigger the message.
--
tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_armorsmithy",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Armor Smithy"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 2,
      granite = 2,
      planks = 2,
      quartz = 1
   },
   return_on_dismantle = {
      granite = 1,
      planks = 1,
      quartz = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 53, 60 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 53, 60 },
      }
   },

   aihints = {
      prohibited_till = 900
   },

   working_positions = {
      atlanteans_armorsmith = 1
   },

   inputs = {
      { name = "coal", amount = 8 },
      { name = "iron", amount = 8 },
      { name = "gold", amount = 8 }
   },
   outputs = {
      "shield_advanced",
      "shield_steel"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_shield_steel",
            "call=produce_shield_advanced",
            "return=skipped"
         }
      },
      produce_shield_steel = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a steel shield because ...
         descname = _"forging a steel shield",
         actions = {
            -- time total: 90
            "return=skipped unless economy needs shield_steel",
            "sleep=45000",  -- +13 enlarge
            "consume=iron:2 coal:2",
            "animate=working 45000",  -- +10 enlarge
            "produce=shield_steel"
         }
      },
      produce_shield_advanced = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging an advanced shield because ...
         descname = _"forging an advanced shield",
         actions = {
            -- time total: 110
            "return=skipped unless economy needs shield_advanced",
            "consume=iron:2 coal:2 gold",
            "sleep=50000",  -- +18 enlarge
            "animate=working 60000",  -- +15 enlarge
            "produce=shield_advanced"
         }
      },
   },
}
