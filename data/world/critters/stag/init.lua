push_textdomain("world")

local dirname = path.dirname(__file__)

wl.Descriptions():new_critter_type{
   name = "stag",
   descname = _("Stag"),
   icon = dirname .. "menu.png",
   animation_directory = dirname,
   programs = {
      remove = { "remove" },
   },
   size = 7,
   reproduction_rate = 30,
   appetite = 30,
   herbivore = {"field"},

   spritesheets = {
      idle = {
         sound_effect = {
            path = "sound/animals/stag",
            priority = "2%"
         },
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 12, 26 }
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 12, 26 }
      },
      walk = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 16, 30 }
      },
   },
}

pop_textdomain()
