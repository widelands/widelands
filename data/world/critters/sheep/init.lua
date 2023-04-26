push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_critter_type{
   name = "sheep",
   descname = _("Sheep"),
   icon = dirname .. "menu.png",
   animation_directory = dirname,
   programs = {
      remove = { "remove" },
   },
   size = 3,
   reproduction_rate = 40,
   appetite = 90,
   herbivore = {"field"},

   spritesheets = {
      idle = {
         sound_effect = {
            path = "sound/farm/sheep",
            priority = "0.01%"
         },
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 8, 16 }
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 8, 16 }
      },
      walk = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 11, 21 }
      },
   },
}

pop_textdomain()
