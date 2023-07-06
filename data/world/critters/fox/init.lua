push_textdomain("world")

local dirname = path.dirname(__file__)

wl.Descriptions():new_critter_type{
   name = "fox",
   descname = _("Fox"),
   icon = dirname .. "menu.png",
   animation_directory = dirname,
   programs = {
      remove = { "remove" },
   },
   size = 4,
   reproduction_rate = 80,
   appetite = 70,
   carnivore = true,

   spritesheets = {
      idle = {
         sound_effect = {
            path = "sound/animals/coyote",
            priority = "0.01%"
         },
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 10, 13 }
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 10, 13 }
      },
      walk = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 11, 14 }
      },
   },
}

pop_textdomain()
