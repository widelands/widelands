push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "debris02",
   descname = _("Debris"),
   size = "small",
   programs = {},
   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 35, 35 },
      },
   }
}

pop_textdomain()
