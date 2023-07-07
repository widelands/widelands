push_textdomain("world")

local dirname = path.dirname(__file__)

wl.Descriptions():new_critter_type{
   name = "wisent",
   descname = _("Wisent"),
   icon = dirname .. "menu.png",
   animation_directory = dirname,
   programs = {
      remove = { "remove" },
   },
   size = 10,
   reproduction_rate = 20,
   appetite = 50,
   herbivore = {"field"},

   spritesheets = {
      idle = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 14, 27 }
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 14, 27 }
      },
      walk = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 24, 32 }
      },
   },
}

pop_textdomain()
