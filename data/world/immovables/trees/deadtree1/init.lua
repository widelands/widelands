push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "deadtree1",
   descname = _ "Dead Tree",
   size = "none",
   programs = {
      main = {
         "animate=idle duration:20s",
         "remove=chance:6.25%"
      }
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 25, 56 },
      },
   }
}

pop_textdomain()
