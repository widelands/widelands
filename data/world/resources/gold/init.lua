push_textdomain("world")

local pics_dir = path.dirname(__file__) .. "../pics/"

wl.Descriptions():new_resource_type{
   name = "resource_gold",
   descname = _("Gold"),
   max_amount = 20,
   detectable = true,
   timeout_ms = 300000,
   timeout_radius = 8,
   representative_image = pics_dir .. "gold4.png",
   editor_pictures = {
      [5] = pics_dir .. "gold1.png",
      [10] = pics_dir .. "gold2.png",
      [15] = pics_dir .. "gold3.png",
      [1000] = pics_dir .. "gold4.png",
   }
}

pop_textdomain()
