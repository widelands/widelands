push_textdomain("world")

local dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "mushroom1",
   descname = _("Mushroom"),
   size = "none",
   programs = {},
   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 6, 9 },
      },
   }
}

pop_textdomain()
