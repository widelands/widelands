push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_well_normal",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Normal Well"),
   icon = dirname .. "menu.png",
   size = "small",
   
   enhancement = {
        name = "europeans_well_advanced",
        enhancement_cost = {
          marble_column = 1,
          quartz = 1,
          diamond = 1
        },
        enhancement_return_on_dismantle = {
          marble = 1,
          quartz = 1,
          diamond = 1
        },
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
      collects_ware_from_map = "water"
   },

   working_positions = {
      europeans_carrier = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "return=skipped unless economy needs water",
            "sleep=duration:20s",
            "animate=working duration:20s",
            "mine=resource_water radius:2 yield:100% when_empty:70%",
            "produce=water:2"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Water",
      heading = _"Out of Water",
      message = pgettext("europeans_building", "The carrier working at this well can’t find any water in his well."),
      productivity_threshold = 25
   },
}

pop_textdomain()
