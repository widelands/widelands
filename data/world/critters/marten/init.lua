push_textdomain("world")

local dirname = path.dirname(__file__)

wl.Descriptions():new_critter_type{
   name = "marten",
   descname = _("Marten"),
   icon = dirname .. "menu.png",
   animation_directory = dirname,
   programs = {
      remove = { "remove" },
   },
   size = 2,
   reproduction_rate = 50,
   appetite = 20,
   carnivore = true,

   spritesheets = {
      idle = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 11, 11 }
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 11, 11 }
      },
      walk = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 15, 14 }
      },
   },
}

pop_textdomain()
