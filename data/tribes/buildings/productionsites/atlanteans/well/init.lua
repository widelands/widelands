dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_well",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Well"),
   helptext_script = dirname .. "helptexts.lua",
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

   outputs = {
      "water"
   },

   indicate_workarea_overlaps = {
      atlanteans_well = false,
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "sleep=20000",
            "animate=working 20000",
            "mine=water 1 100 65 2",
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
