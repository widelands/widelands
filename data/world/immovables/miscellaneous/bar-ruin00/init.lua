dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "bar-ruin00",
   descname = _ "Ruin",
   size = "small",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 34, 32 },
      },
   }
}
