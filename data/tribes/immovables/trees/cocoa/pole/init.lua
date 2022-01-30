push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_immovable_type {
   name = "cocoa_tree_pole",
   -- TRANSLATORS:  This is an amazon rare tree.
   descname = _("Cocoa Tree (pole)"),
   size = "small",
   icon = dirname .. "menu_pole.png",
   programs = {
      main = {
         "animate=idle duration:60s",
         "remove=chance:5.86%",
         "transform=cocoa_tree_mature",
      },
   },
   animation_directory = dirname,
   spritesheets = { idle = {
      basename = "cocoa_pole",
      hotspot = {16, 44},
      fps = 8,
      frames = 4,
      columns = 2,
      rows = 2
   }}
}

pop_textdomain()
