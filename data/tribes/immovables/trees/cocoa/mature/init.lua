push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_immovable_type {
   name = "cocoa_tree_mature",
   -- TRANSLATORS:  This is an Amazon rare tree.
   descname = _("Cocoa Tree (mature)"),
   size = "small",
   icon = dirname .. "menu_mature.png",
   programs = {
      main = {
         "animate=idle duration:60s",
         "remove=chance:3.91%",
         "transform=cocoa_tree_old",
      },
   },
   animation_directory = dirname,
   spritesheets = { idle = {
      basename = "cocoa_mature",
      hotspot = {27, 68},
      fps = 8,
      frames = 4,
      columns = 2,
      rows = 2
   }}
}

pop_textdomain()
