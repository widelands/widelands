push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_productionsite_type {
   name = "europeans_well_advanced",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Advanced Well"),
   icon = dirname .. "menu.png",
   size = "small",

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
            "sleep=duration:10s",
            "animate=working duration:20s",
            "mine=resource_water radius:4 yield:100% when_empty:99%",
            "produce=water:4"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Water",
      heading = _"Out of Water",
      message = pgettext("europeans_building", "The carrier working at this well can’t find any water in his well."),
      productivity_threshold = 1
   },
}

pop_textdomain()
