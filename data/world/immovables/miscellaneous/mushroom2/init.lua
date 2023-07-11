push_textdomain("world")

local dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "mushroom2",
   descname = _("Mushroom"),
   size = "none",
   programs = {},
   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 5, 7 },
      },
   }
}

pop_textdomain()
