push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_critter_type{
   name = "brownbear",
   descname = _("Brown bear"),
   icon = dirname .. "menu.png",
   animation_directory = dirname,
   programs = {
      remove = { "remove" },
   },
   size = 10,
   reproduction_rate = 60,
   appetite = 90,
   carnivore = true,

   spritesheets = {
      idle = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 21, 16 }
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 21, 16 }
      },
      walk = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 24, 24 }
      },
   },
}

pop_textdomain()
