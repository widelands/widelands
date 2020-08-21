dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "wheatfield_small",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext("immovable", "Wheat Field (small)"),
   icon = dirname .. "menu.png",
   size = "small",
   programs = {
      main = {
         "animate=idle duration:45s",
         "transform=wheatfield_medium",
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 31, 24 },
      },
   }
}
