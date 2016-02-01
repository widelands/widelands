dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle.png"),
      hotspot = { 36, 86 }
   },
}

world:new_immovable_type{
   name = "winterland_rocks2",
   descname = _ "Rocks 2",
   editor_category = "rocks",
   size = "big",
   attributes = { "rocks" },
   programs = {
      shrink = {
         "transform=winterland_rocks1"
      }
   },
   animations = animations
}
