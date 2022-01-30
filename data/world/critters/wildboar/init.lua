push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_critter_type{
   name = "wildboar",
   descname = _("Wild boar"),
   icon = dirname .. "menu.png",
   animation_directory = dirname,
   programs = {
      remove = { "remove" },
   },
   size = 8,
   reproduction_rate = 10,
   appetite = 20,
   herbivore = {"field"},

   spritesheets = {
      idle = {
         sound_effect = {
            path = "sound/animals/boar",
            priority = "0.01%"
         },
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 10, 18 }
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 10, 18 }
      },
      walk = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 20, 22 }
      },
   }
}

pop_textdomain()
