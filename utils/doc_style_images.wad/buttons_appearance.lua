local width = 350
wl.ui.MapView():create_child({
   widget   = "window",
   name     = "fonts_styles_window",
   title    = _("Buttons (default theme)"),
   content  = {
      widget      = "box",
      name        = "main_box",
      orientation = "vertical",
      children    = {
         { widget= "space", value=5},
         {
            widget  = "multilinetextarea",
            name    = "button-header",
            scroll_mode = "none",
            w       = width,
            font    = "wui_info_panel_heading",
            text    = "Button styles (wui)"
         },
         { widget = "space", value = 5},
         {
            widget      = "box",
            name        = "button_box_styles",
            orientation = "horizontal",
            children    = {
               { widget= "space", value=10},
               {
                  widget = "button",
                  name   =  "primary",
                  style   = "primary",
                  title   = "primary"
               },
               { widget= "space", value=10},
               {
                  widget = "button",
                  name   =  "secondary",
                  style   = "secondary",
                  title   = "secondary (default)"
               },
               { widget= "space", value=10},
               {
                  widget = "button",
                  name   =  "menu",
                  style   = "menu",
                  title   = "menu"
               },
               { widget= "space", value=10},
            },
         },
         { widget = "space", value = 20},
         {
            widget  = "multilinetextarea",
            name    = "button-header-visual",
            scroll_mode = "none",
            w       = width,
            font    = "wui_info_panel_heading",
            text    = "Button appearance"
         },
         { widget = "space", value = 5},
         {
            widget      = "box",
            name        = "button_box_global",
            orientation = "horizontal",
            children    = {
               { widget= "space", value=10},
               {
                  widget = "button",
                  name   =  "primary",
                  style   = "primary",
                  title   = "raised (default)",
                  visual  = "raised",
               },
               { widget= "space", value=10},
               {
                  widget = "button",
                  name   =  "secondary",
                  style   = "secondary",
                  title   = "permpressed",
                  visual  = "permpressed",
               },
               { widget= "space", value=10},
               {
                  widget = "button",
                  name   =  "menu",
                  style   = "menu",
                  title   = "flat",
                  visual  = "flat",
               },
               { widget= "space", value=10},
            },
         },
         { widget = "space", value = 5},
      }
   }
})
