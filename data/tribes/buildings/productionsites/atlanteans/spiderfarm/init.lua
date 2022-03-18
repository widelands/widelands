push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "atlanteans_spiderfarm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Spider Farm"),
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 2,
      granite = 2,
      planks = 2
   },
   return_on_dismantle = {
      granite = 1,
      planks = 1
   },

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 87, 75 },
      },
      working = {
         basename = "idle", -- TODO(GunChleoc): No animation yet.
         hotspot = { 87, 75 },
      }
   },

   aihints = {
      basic_amount = 1,
      prohibited_till = 350
   },

   working_positions = {
      atlanteans_spiderbreeder = 1
   },

   inputs = {
      { name = "water", amount = 7 },
      { name = "corn", amount = 7 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            "return=skipped unless economy needs spider_silk",
            "consume=corn water",
            "sleep=duration:25s",
            "animate=working duration:30s",
            "produce=spider_silk"
         }
      },
   },
}

pop_textdomain()
