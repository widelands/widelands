push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_critter_type{
   name = "wildboar",
   descname = _ "Wild boar",
   icon = dirname .. "menu.png",
   animation_directory = dirname,
   programs = {
      remove = { "remove" },
   },
   size = 8,
   reproduction_rate = 10,
   appetite = 20,
   herbivore = {"field"},

   animations = {
      idle = {
         hotspot = { 10, 18 },
         fps = 20,
         sound_effect = {
            path = "sound/animals/boar",
            priority = "0.01%"
         },
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         hotspot = { 10, 18 },
         fps = 20,
      },
      walk = {
         hotspot = { 20, 22 },
         fps = 20,
         directional = true
      }
   }
}

pop_textdomain()
