push_textdomain("world")

wl.World():new_critter_type{
   name = "fox",
   descname = _ "Fox",
   animation_directory = path.dirname(__file__),
   programs = {
      remove = { "remove" },
   },
   size = 4,
   reproduction_rate = 80,
   appetite = 70,
   carnivore = true,

   animations = {
      idle = {
         sound_effect = {
            path = "sound/animals/coyote",
            priority = 0.01
         },
         hotspot = { 10, 13 },
         fps = 10,
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         hotspot = { 10, 13 },
         fps = 10,
      },
      walk = {
         hotspot = { 11, 14 },
         fps = 20,
         directional = true
      }
   }
}

pop_textdomain()
