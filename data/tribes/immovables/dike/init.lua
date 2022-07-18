push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_immovable_type {
   name = "dike",
   -- TRANSLATORS: This is an immovable name used in lists of immovables
   descname = pgettext ("immovable", "Dike"),
   icon = dirname .. "menu.png",
   size = "none",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove=",
      },
      maybe_remove = {
         "remove=chance:33%",
         "animate=idle duration:5m",
         "remove=",
      }
   },
   animation_directory = dirname,
   animations = { idle = { hotspot = {18, 19}}}
}

pop_textdomain()
