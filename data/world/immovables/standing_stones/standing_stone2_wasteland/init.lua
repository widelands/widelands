dirname = path.dirname(__file__)

wl.World():new_immovable_type{
   name = "standing_stone2_wasteland",
   descname = _ "Standing Stone",
   size = "big",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 20, 38 },
      },
   }
}
