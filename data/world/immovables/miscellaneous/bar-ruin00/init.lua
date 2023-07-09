push_textdomain("world")

local dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "bar-ruin00",
   descname = _("Ruin"),
   size = "small",
   programs = {},
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   animations = {
      idle = {
         hotspot = { 34, 32 },
      },
   }
}

pop_textdomain()
