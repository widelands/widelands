push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_critter_type{
   name = "moose",
   descname = _ "Moose",
   icon = dirname .. "menu.png",
   animation_directory = dirname,
   programs = {
      remove = { "remove" },
   },
   size = 10,
   reproduction_rate = 30,
   appetite = 50,
   herbivore = {"field"},

   animations = {
      idle = {
         hotspot = { 15, 27 },
         fps = 20,
         sound_effect = {
            path = "sound/animals/moose",
            priority = 5
         },
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         hotspot = { 15, 27 },
         fps = 20,
      },
      walk = {
         hotspot = { 21, 34 },
         fps = 20,
         directional = true
      }
   }
}

pop_textdomain()
