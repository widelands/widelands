dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "pebble3",
   descname = _ "Pebble",
   size = "none",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 3, 9 },
      },
   }
}
