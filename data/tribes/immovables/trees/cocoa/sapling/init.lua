push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_immovable_type {
   name = "cocoa_tree_sapling",
   -- TRANSLATORS:  This is an amazon rare tree.
   descname = _("Cocoa Tree (sapling)"),
   size = "small",
   icon = dirname .. "menu_sapling.png",
   programs = {
      main = {
         "animate=idle duration:80s",
         "remove=chance:7.81%",
         "transform=cocoa_tree_pole",
      },
   },
   animation_directory = dirname,
   spritesheets = { idle = {
      basename = "cocoa_sapling",
      hotspot = {11, 21},
      fps = 8,
      frames = 4,
      columns = 2,
      rows = 2
   }}
}

pop_textdomain()
