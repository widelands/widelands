dirname = path.dirname(__file__)

wl.World():new_immovable_type{
   name = "pebble2",
   descname = _ "Pebble",
   size = "none",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 8, 9 },
      },
   }
}
