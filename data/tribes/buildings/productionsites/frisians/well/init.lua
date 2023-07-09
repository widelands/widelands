push_textdomain("tribes")

local dirname = path.dirname (__file__)

wl.Descriptions():new_productionsite_type {
   name = "frisians_well",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Well"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 1,
      granite = 2,
      brick = 1
   },
   return_on_dismantle = {
      granite = 1,
      brick = 1
   },

   spritesheets = {
      working = {
         hotspot = {19, 33},
         frames = 10,
         columns = 5,
         rows = 2,
         fps = 10
      }
   },
   animations = {
      idle = {
         hotspot = {19, 33}
      }
   },

   aihints = {
      basic_amount = 1
   },

   working_positions = {
      frisians_carrier = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            "sleep=duration:20s",
            "animate=working duration:20s",
            "mine=resource_water radius:1 yield:100% when_empty:65%",
            "produce=water",
         }
      },
   },

   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Water"),
      heading = _("Out of Water"),
      message = pgettext ("frisians_building", "The carrier working at this well can’t find any water in his well."),
      productivity_threshold = 33
   },
}

pop_textdomain()
