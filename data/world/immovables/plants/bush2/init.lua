dirname = path.dirname(__file__)

wl.World():new_immovable_type{
   name = "bush2",
   descname = _ "Bush",
   size = "none",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 12, 9 },
      },
   }
}
