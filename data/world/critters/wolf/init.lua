push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_critter_type{
   name = "wolf",
   descname = _("Wolf"),
   icon = dirname .. "menu.png",
   animation_directory = dirname,
   programs = {
      remove = { "remove" },
   },
   size = 5,
   reproduction_rate = 80,
   appetite = 100,
   carnivore = true,

   spritesheets = {
      idle = {
         sound_effect = {
            -- Sound files with numbers starting from 10 are generating silence.
            path = "sound/animals/wolf",
            priority = "3%"
         },
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 8, 15 }
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 8, 15 }
      },
      walk = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 19, 19 }
      },
   },
}

pop_textdomain()
