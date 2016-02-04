dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle.png"),
      hotspot = { 30, 75 }
   },
}

world:new_immovable_type{
   name = "blackland_rocks1",
   descname = _ "Rocks 1",
   editor_category = "rocks",
   size = "big",
   attributes = { "rocks" },
   programs = {
      shrink = {
         "remove="
      }
   },
   animations = animations
}
