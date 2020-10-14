push_textdomain("world")

dirname = path.dirname(__file__)

wl.World():new_critter_type{
   name = "stag",
   descname = _ "Stag",
   icon = dirname .. "menu.png",
   animation_directory = dirname,
   programs = {
      remove = { "remove" },
   },
   size = 7,
   reproduction_rate = 30,
   appetite = 30,
   herbivore = {"field"},

   animations = {
      idle = {
         sound_effect = {
            path = "sound/animals/stag",
            priority = 2
         },
         hotspot = { 12, 26 },
         fps = 20,
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         hotspot = { 12, 26 },
         fps = 20,
      },
      walk = {
         hotspot = { 25, 30 },
         fps = 20,
         directional = true
      }
   }
}

pop_textdomain()
