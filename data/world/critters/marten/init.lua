push_textdomain("world")

dirname = path.dirname(__file__)

wl.World():new_critter_type{
   name = "marten",
   descname = _ "Marten",
   icon = dirname .. "menu.png",
   animation_directory = dirname,
   programs = {
      remove = { "remove" },
   },
   size = 2,
   reproduction_rate = 50,
   appetite = 20,
   carnivore = true,

   animations = {
      idle = {
         hotspot = { 11, 11 },
         fps = 20,
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         hotspot = { 11, 11 },
         fps = 20,
      },
      walk = {
         hotspot = { 15, 14 },
         fps = 20,
         directional = true
      }
   }
}

pop_textdomain()
