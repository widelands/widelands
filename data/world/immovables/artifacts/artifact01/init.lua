push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "artifact01",
   descname = _("Artifact"),
   size = "small",
   programs = {},
   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 14, 20 },
      },
   }
}

pop_textdomain()
