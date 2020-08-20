dirname = path.dirname(__file__)

wl.World():new_immovable_type{
   name = "deadtree5",
   descname = _ "Dead Tree",
   size = "none",
   programs = {
      main = {
         "animate=idle duration:20s",
         "remove=chance:4.69%"
      }
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 27, 56 },
      },
   }
}
