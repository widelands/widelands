-- RST
-- .. _lua_tribes_buildings_productionsites:
--
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
-- .. function:: new_productionsite_type{table}
--
--    This function adds the definition of a production site building to the engine.
--
--    :arg table: This table contains all the data that the game engine will add to this building.
--                In addition to the :ref:`lua_tribes_buildings_common`, it contains the following entries:
--
--    **size**
--        *Mandatory*. In addition to the common size values ``"small"``,
--        ``"medium"``, or ``"big"``, production sites can also have size ``"mine"``
--        for defining a mine building to be built on mountains.
--
--    **working_positions**
--        *Mandatory*. The name and amount for each worker type working at this
--        building, e.g.::
--
--            working_positions = { atlanteans_armorsmith = 1 },
--
--    **inputs**
--        *Optional*. The name and amount for each ware input queue, e.g.::
--
--            inputs = {
--                { name = "coal", amount = 10 },
--                { name = "iron", amount = 8 },
--                { name = "gold", amount = 8 }
--            },
--
--    **outputs**
--        *DEPRECATED*. The wares/workers produced by this building, e.g.::
--
--            outputs = { "shield_advanced", "shield_steel" },
--
--    **programs**.
--        *Mandatory*. The production site programs that define what preconditions
--        a building needs to fulfil in order to produce its wares and how it's
--        done, including any animations and sounds played.
--        See :ref:`productionsite_programs`.
--
--    **indicate_workarea_overlaps**
--        *DEPRECATED*. The names of other productionsites whose workareas should be highlighted
--        if theirs overlap with this building’s workarea while the player is placing a
--        building of this type. The overlaps can be shown either as desired (`true`), if the
--        proximity of these buildings is favourable, or as negative (`false`), if they influence
--        each other negatively. Example for a fishbreeder’s house::
--
--            indicate_workarea_overlaps = {
--               atlanteans_fishers_house = true,
--               atlanteans_fishbreeders_house = false
--            },
--
--    **out_of_resource_notification**.
--        *Optional*. This table defines the message sent by the productionsite
--        to the player if it has run out of a resource to collect. There are 4
--        entries:
--
--        **title**
--            *Mandatory*. A concise message title for the list in the inbox.
--
--        **heading**
--            *Mandatory*. A longer version of the title, shown in the message body.
--
--        **message**
--            *Mandatory*. The actual message. Translation needs to be fetched with pgettext.
--
--        **productivity_threshold**
--            *Optional. Default:* ``100``. An int value in percent to trigger the message.
--
--        Example::
--
--            out_of_resource_notification = {
--                -- Translators: Short for "Out of ..." for a resource
--                title = _"No Fields",
--                heading = _"Out of Fields",
--                message = pgettext("atlanteans_building", "The farmer working at this farm has no cleared soil to plant his seeds."),
--                productivity_threshold = 30
--            },
--
--    **resource_not_needed_message**.
--        *Optional*. This string defines the tooltip shown by the productionsite
--        to the player if all resources have been replenished by its worker in full.
--        Look at the Atlantean Fish Breeder's House for an example.
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
      prohibited_till = 1040
   },

   working_positions = {
      atlanteans_armorsmith = 1
   },

   inputs = {
      { name = "coal", amount = 8 },
      { name = "iron", amount = 8 },
      { name = "gold", amount = 8 }
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_shield_steel",
            "call=produce_shield_advanced",
         }
      },
      produce_shield_steel = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a steel shield because ...
         descname = _"forging a steel shield",
         actions = {
            -- time total: 67 + 3.6
            "return=skipped unless economy needs shield_steel",
            "consume=iron:2 coal:2",
            "sleep=32000",
            "animate=working duration:35s",
            "produce=shield_steel"
         }
      },
      produce_shield_advanced = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging an advanced shield because ...
         descname = _"forging an advanced shield",
         actions = {
            -- time total: 77 + 3.6
            "return=skipped unless economy needs shield_advanced",
            "consume=iron:2 coal:2 gold",
            "sleep=32000",
            "animate=working duration:45s",
            "produce=shield_advanced"
         }
      },
   },
}
