dirname = path.dirname(__file__)

wl.World():new_immovable_type{
   name = "deadtree4",
   descname = _ "Dead Tree",
   size = "none",
   programs = {
      main = {
         "animate=idle duration:20s",
         "remove=chance:7.03%"
      }
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 23, 61 },
      },
   }
}
