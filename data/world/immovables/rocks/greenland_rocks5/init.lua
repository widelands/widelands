dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle.png"),
      hotspot = { 40, 89 }
   },
}

world:new_immovable_type{
   name = "greenland_rocks5",
   descname = _ "Rocks 5",
   icon = dirname .. "menu.png",
   editor_category = "rocks",
   size = "big",
   attributes = { "rocks" },
   programs = {
      shrink = {
         "transform=greenland_rocks4"
      }
   },
   animations = animations
}
