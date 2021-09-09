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

   spritesheets = {
      idle = {
         sound_effect = {
            path = "sound/animals/moose",
            priority = "5%"
         },
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 15, 27 }
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 15, 27 }
      },
      walk = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 21, 34 }
      },
   },
}

pop_textdomain()
