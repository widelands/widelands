push_textdomain("tribes")

dirname = path.dirname(__file__)


wl.Descriptions():new_immovable_type{
   name = "deadtree7",
   -- TRANSLATORS:  This is an amazon rare tree.
   descname = _("Dead Tree"),
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:20s",
         "remove=chance:6.25%",
      },
   },
   icon = dirname .. "menu.png",
   animation_directory = dirname,
   animations = { idle = { hotspot = {2, 72}}}
}

pop_textdomain()
