push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "atlanteans_well",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Well"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 2,
      granite = 1,
      planks = 1
   },
   return_on_dismantle = {
      log = 1,
      granite = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 31, 32 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 31, 32 },
      },
   },

   aihints = {
      basic_amount = 1,
      collects_ware_from_map = "water"
   },

   working_positions = {
      atlanteans_carrier = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "sleep=duration:20s",
            "animate=working duration:20s",
            "mine=resource_water radius:1 yield:100% when_empty:65%",
            "produce=water"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Water",
      heading = _"Out of Water",
      message = pgettext("atlanteans_building", "The carrier working at this well can’t find any water in his well."),
      productivity_threshold = 33
   },
}

pop_textdomain()
