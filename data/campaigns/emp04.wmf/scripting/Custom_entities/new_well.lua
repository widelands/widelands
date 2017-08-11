dirname = "tribes/buildings/productionsites/empire/well/"

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_well1",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Well"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",
   enhancement = "empire_well2",

   buildcost = {
      log = 2,
      granite = 1,
      marble = 1
   },
   return_on_dismantle = {
      marble = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 43, 43 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), -- TODO(GunChleoc): No animation yet.
         hotspot = { 43, 43 },
      },
   },

   aihints = {
      mines_water = true,
      prohibited_till = 800,
      forced_after = 800
   },

   working_positions = {
      empire_carrier = 1
   },

   outputs = {
      "water"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "sleep=30000",
            "animate=working 30000",
            "mine=water 5 100 10 2",
            "produce=water"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Water",
      heading = _"Out of Water",
      message = pgettext("empire_building", "The carrier working at this well can’t find any water in his well."),
      productivity_threshold = 15
   },
}
