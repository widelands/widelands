dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "bar-ruin03",
   descname = _ "Ruin",
   size = "big",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 44, 34 },
      },
   }
}
