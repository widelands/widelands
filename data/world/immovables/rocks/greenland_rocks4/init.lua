dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle.png"),
      hotspot = { 42, 84 }
   },
}

world:new_immovable_type{
   name = "greenland_rocks4",
   descname = _ "Rocks 4",
   icon = dirname .. "menu.png",
   editor_category = "rocks",
   size = "big",
   attributes = { "rocks" },
   programs = {
      shrink = {
         "transform=greenland_rocks3"
      }
   },
   animations = animations
}
