dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "pebble4",
   descname = _ "Pebble",
   size = "none",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 5, 5 }
      },
   }
}
