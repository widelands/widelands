dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle.png"),
      hotspot = { 30, 75 }
   },
}

world:new_immovable_type{
   name = "blackland_rocks4",
   descname = _ "Rocks 4",
   icon = dirname .. "menu.png",
   editor_category = "rocks",
   size = "big",
   attributes = { "rocks" },
   programs = {
      shrink = {
         "transform=blackland_rocks3"
      }
   },
   animations = animations
}
