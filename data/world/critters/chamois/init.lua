push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_critter_type{
   name = "chamois",
   descname = _ "Chamois",
   icon = dirname .. "menu.png",
   animation_directory = dirname,
   programs = {
      remove = { "remove" },
   },
   size = 5,
   reproduction_rate = 60,
   appetite = 20,
   herbivore = {"field"},

   spritesheets = {
      idle = {
         fps = 20,
         frames = 19,
         rows = 5,
         columns = 4,
         hotspot = { 11, 13 }
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         fps = 20,
         frames = 19,
         rows = 5,
         columns = 4,
         hotspot = { 11, 13 }
      },
      walk = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 11, 20 }
      },
   },

}

pop_textdomain()
