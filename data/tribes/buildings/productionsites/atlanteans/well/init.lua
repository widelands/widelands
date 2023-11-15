push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
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

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 36, 42 },
      },
      working = {
         basename = "idle", -- TODO(GunChleoc): No animation yet.
         hotspot = { 36, 42 },
      },
   },

   aihints = {
      basic_amount = 1,
   },

   working_positions = {
      atlanteans_carrier = 1
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            -- time total: 20 + 20 + 3.6 = 43.6 sec
            -- average time if water resource is depleted:
            --   (65 * (20 + 20 + 3.6) + 35 * (20 + 20 + 10)) / 65 = 70.523 sec
            "sleep=duration:20s",
            "animate=working duration:20s",
            "mine=resource_water radius:1 yield:100% when_empty:65%",
            "produce=water"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _("No Water"),
      heading = _("Out of Water"),
      message = pgettext("atlanteans_building", "The carrier working at this well can’t find any water in his well."),
      productivity_threshold = 33
   },
}

pop_textdomain()
