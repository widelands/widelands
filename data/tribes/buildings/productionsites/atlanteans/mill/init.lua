dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_mill",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Mill"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      log = 3,
      granite = 3,
      planks = 2
   },
   return_on_dismantle = {
      log = 1,
      granite = 2,
      planks = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 58, 61 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 58, 61 },
         fps = 25
      }
   },

   aihints = {
      basic_amount = 1,
      prohibited_till = 570
   },

   working_positions = {
      atlanteans_miller = 1
   },

   inputs = {
      { name = "corn", amount = 6 },
      { name = "blackroot", amount = 6 }
   },
   outputs = {
      "cornmeal",
      "blackroot_flour"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_cornmeal",
            "call=produce_blackroot_flour",
            "return=no_stats"
         }
      },
      produce_cornmeal = {
         -- TRANSLATORS: Completed/Skipped/Did not start grinding corn because ...
         descname = _"grinding corn",
         actions = {
            "return=skipped when site has blackroot and economy needs blackroot_flour and not economy needs cornmeal",
            "return=skipped unless economy needs cornmeal",
            "sleep=3500",
            "consume=corn",
            "playsound=sound/mill/mill_turning 240",
            "animate=working 15000",
            "produce=cornmeal"
         }
      },
      produce_blackroot_flour = {
         -- TRANSLATORS: Completed/Skipped/Did not start grinding blackroot because ...
         descname = _"grinding blackroot",
         actions = {
            -- No check whether we need blackroot_flour because blackroots cannot be used for anything else.
            "return=skipped when site has corn and economy needs cornmeal and not economy needs blackroot_flour",
            "consume=blackroot",
            "sleep=3500",
            "playsound=sound/mill/mill_turning 240",
            "animate=working 15000",
            "produce=blackroot_flour"
         }
      },
   },
}
