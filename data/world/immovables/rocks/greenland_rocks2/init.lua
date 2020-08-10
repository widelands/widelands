dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle.png"),
      hotspot = { 36, 86 }
   },
}

world:new_immovable_type{
   name = "greenland_rocks2",
   descname = _ "Rocks 2",
   icon = dirname .. "menu.png",
   editor_category = "rocks",
   size = "big",
   attributes = { "rocks" },
   programs = {
      shrink = {
         "transform=greenland_rocks1"
      }
   },
   animations = animations
}
