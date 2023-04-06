push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_productionsite_type {
   name = "empire_sawmill",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Sawmill"),
   icon = dirname .. "menu.png",
   size = "medium",

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
   spritesheets = {
      idle = {
         frames = 1,
         columns = 1,
         rows = 1,
         hotspot = { 48, 62 },
      },
      working = {
         basename = "idle", -- TODO(GunChleoc): No animation yet.
         frames = 1,
         columns = 1,
         rows = 1,
         hotspot = { 48, 62 },
      },
   },

   aihints = {
      basic_amount = 2,
      very_weak_ai_limit = 2,
      weak_ai_limit = 2
   },

   working_positions = {
      empire_carpenter = 1
   },

   inputs = {
      { name = "log", amount = 8 }
   },

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start sawing logs because ...
         descname = _("sawing logs"),
         actions = {
            "return=skipped unless economy needs planks",
            "consume=log:2",
            "sleep=duration:16s500ms", -- Much faster than barbarians' wood hardener
            "playsound=sound/sawmill/sawmill priority:40% allow_multiple",
            "animate=working duration:20s", -- Much faster than barbarians' wood hardener
            "produce=planks"
         }
      },
   },
}

pop_textdomain()
