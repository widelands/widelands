push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_critter_type{
   name = "bunny",
   descname = _ "Bunny",
   icon = dirname .. "menu.png",
   animation_directory = dirname,
   programs = {
      remove = { "remove" },
   },
   size = 1,
   reproduction_rate = 100,
   appetite = 100,
   herbivore = {"field"},

   animations = {
      idle = {
         hotspot = { 4, 9 },
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         hotspot = { 4, 9 },
      },
   },
   spritesheets = {
      walk = {
         frames = 2,
         rows = 2,
         columns = 1,
         directional = true,
         hotspot = { 5, 9 }
      },
   },
}

pop_textdomain()
