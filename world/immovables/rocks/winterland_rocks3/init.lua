dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle.png"),
      hotspot = { 42, 89 }
   },
}

world:new_immovable_type{
   name = "winterland_rocks3",
   descname = _ "Rocks 3",
   editor_category = "rocks",
   size = "big",
   attributes = { "rocks" },
   programs = {
      shrink = {
         "transform=winterland_rocks2"
      }
   },
   animations = animations
}
