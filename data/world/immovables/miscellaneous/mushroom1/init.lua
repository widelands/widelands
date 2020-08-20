dirname = path.dirname(__file__)

wl.World():new_immovable_type{
   name = "mushroom1",
   descname = _ "Mushroom",
   size = "none",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 6, 9 },
      },
   }
}
