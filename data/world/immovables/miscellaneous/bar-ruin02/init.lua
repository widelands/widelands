push_textdomain("world")

local dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "bar-ruin02",
   descname = _("Ruin"),
   size = "medium",
   programs = {},
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   animations = {
      idle = {
         hotspot = { 43, 43 },
      },
   }
}

pop_textdomain()
