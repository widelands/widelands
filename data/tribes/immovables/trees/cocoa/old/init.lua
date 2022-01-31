push_textdomain("tribes")

dirname = path.dirname (__file__)

descriptions:new_immovable_type {
   name = "cocoa_tree_old",
   -- TRANSLATORS:  This is an amazon rare tree.
   descname = _("Cocoa Tree (old)"),
   size = "small",
   icon = dirname .. "menu_old.png",
   programs = {
      main = {
         "animate=idle duration:8m20s",
         "remove=",
      },
      harvest = {
         "remove=",
      }
   },
   animation_directory = dirname,
   spritesheets = { idle = {
      basename = "cocoa_old",
      hotspot = {27, 68},
      fps = 8,
      frames = 4,
      columns = 2,
      rows = 2
   }}
}

pop_textdomain()
