dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "pebble1",
   descname = _ "Pebble",
   editor_category = "miscellaneous",
   size = "none",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 3, 4 },
      },
   }
}
