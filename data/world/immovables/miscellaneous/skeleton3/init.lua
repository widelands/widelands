push_textdomain("world")

local dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "skeleton3",
   descname = _("Skeleton"),
   size = "none",
   programs = {},
   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 47, 69 },
      },
   }
}

pop_textdomain()
