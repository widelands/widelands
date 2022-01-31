push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "atlanteans_mill",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Mill"),
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

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 58, 61 },
      },
   },

   spritesheets = {
      working = {
         fps = 25,
         frames = 19,
         rows = 5,
         columns = 4,
         hotspot = { 58, 61 }
      },
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

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            "call=produce_cornmeal",
            "call=produce_blackroot_flour",
         }
      },
      produce_cornmeal = {
         -- TRANSLATORS: Completed/Skipped/Did not start grinding corn because ...
         descname = _("grinding corn"),
         actions = {
            "return=skipped when site has blackroot and economy needs blackroot_flour and not economy needs cornmeal",
            "return=skipped unless economy needs cornmeal",
            "sleep=duration:3s500ms",
            "consume=corn",
            "playsound=sound/mill/mill_turning priority:85% allow_multiple",
            "animate=working duration:15s",
            "produce=cornmeal"
         }
      },
      produce_blackroot_flour = {
         -- TRANSLATORS: Completed/Skipped/Did not start grinding blackroot because ...
         descname = _("grinding blackroot"),
         actions = {
            -- No check whether we need blackroot_flour because blackroots cannot be used for anything else.
            "return=skipped when site has corn and economy needs cornmeal and not economy needs blackroot_flour",
            "consume=blackroot",
            "sleep=duration:3s500ms",
            "playsound=sound/mill/mill_turning priority:85% allow_multiple",
            "animate=working duration:15s",
            "produce=blackroot_flour"
         }
      },
   },
}

pop_textdomain()
