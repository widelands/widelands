push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_critter_type{
   name = "wisent",
   descname = _ "Wisent",
   icon = dirname .. "menu.png",
   animation_directory = dirname,
   programs = {
      remove = { "remove" },
   },
   size = 10,
   reproduction_rate = 20,
   appetite = 50,
   herbivore = {"field"},

   animations = {
      idle = {
         hotspot = { 14, 27 },
         fps = 20,
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         hotspot = { 14, 27 },
         fps = 20,
      },
      walk = {
         hotspot = { 24, 32 },
         fps = 20,
         directional = true
      }
   }
}

pop_textdomain()
