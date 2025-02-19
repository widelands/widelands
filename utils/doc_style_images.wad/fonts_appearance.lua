local width = 280
wl.ui.MapView():create_child({
   widget   = "window",
   name     = "fonts_styles_window",
   title    = _("Font styles (default theme)"),
   content  = {
      widget      = "box",
      name        = "main_box",
      orientation = "vertical",
      spacing     = 5,
      resizing    = "expandboth",
      children    = {
         { widget= "space", value=5},
         {
            widget      = "box",
            name        = "all_fonts_headers",
            orientation = "horizontal",
            children    = {
               {
                  widget  = "multilinetextarea",
                  name    = "wui-header",
                  scroll_mode = "none",
                  w       = width,
                  font    = "wui_info_panel_heading",
                  text    = "Fonts used In-Game (wui)"
               },
               {
                  widget  = "multilinetextarea",
                  name    = "fs-header",
                  scroll_mode = "none",
                  w       = width,
                  font    = "wui_info_panel_heading",
                  text    = "Fonts used for main menus (fs)"
               },
            },
         },
         {
            widget      = "box",
            name        = "fonts_box_wui",
            orientation = "horizontal",
            children    = {
               {
                  widget = "box",
                  name   = "wui",
                  spacing     = 5,
                  orientation = "vertical",
                  align       = "top",
                  children = {
                     { widget = "space", value = 5},
                     {
                        widget = "textarea",
                        name   =  "wui_window_title",
                        font   = "wui_window_title",
                        text   = "wui_window_title"
                     },
                     {
                        widget = "textarea",
                        name   =  "wui_message_heading",
                        font   = "wui_message_heading",
                        text   = "wui_message_heading"
                     },
                     {
                        widget = "textarea",
                        name   =  "wui_message_paragraph",
                        font   = "wui_message_paragraph",
                        text   = "wui_message_paragraph"
                     },               {
                        widget = "textarea",
                        name   =  "wui_info_panel_heading",
                        font   = "wui_info_panel_heading",
                        text   = "wui_info_panel_heading"
                     },
                     {
                        widget = "textarea",
                        name   =  "wui_info_panel_paragraph",
                        font   = "wui_info_panel_paragraph",
                        text   = "wui_info_panel_paragraph"
                     },
                     {
                        widget = "textarea",
                        name   =  "label_wui",
                        font   = "label_wui",
                        text   = "label_wui"
                     },
                     {
                        widget = "textarea",
                        name   =  "tooltip_header_wui",
                        font   = "tooltip_header_wui",
                        text   = "tooltip_header_wui"
                     },
                     {
                        widget = "textarea",
                        name   =  "tooltip_wui",
                        font   = "tooltip_wui",
                        text   = "tooltip_wui"
                     },
                     {
                        widget = "textarea",
                        name   =  "tooltip_hotkey_wui",
                        font   = "tooltip_hotkey_wui",
                        text   = "tooltip_hotkey_wui"
                     },
                     {
                        widget = "textarea",
                        name   =  "game_summary_title",
                        font   = "game_summary_title",
                        text   = "game_summary_title"
                     },
                     {
                        widget = "textarea",
                        name   =  "wui_attack_box_slider_label",
                        font   = "wui_attack_box_slider_label",
                        text   = "wui_attack_box_slider_label"
                     },
                     {
                        widget = "textarea",
                        name   =  "wui_game_speed_and_coordinates",
                        font   = "wui_game_speed_and_coordinates",
                        text   = "wui_game_speed_and_coordinates"
                     },
                     {
                        widget = "textarea",
                        name   =  "chat_message",
                        font   = "chat_message",
                        text   = "chat_message"
                     },
                     {
                        widget = "textarea",
                        name   =  "chat_timestamp",
                        font   = "chat_timestamp",
                        text   = "chat_timestamp"
                     },
                     {
                        widget = "textarea",
                        name   =  "chat_whisper",
                        font   = "chat_whisper",
                        text   = "chat_whisper"
                     },
                     {
                        widget = "textarea",
                        name   =  "chat_playername",
                        font   = "chat_playername",
                        text   = "chat_playername"
                     },
                     {
                        widget = "textarea",
                        name   =  "chat_server",
                        font   = "chat_server",
                        text   = "chat_server"
                     },
                  },
               },
               { widget= "space", value=20},
               {
                  widget = "box",
                  name   = "fonts_box_fs",
                  orientation = "vertical",
                  align       = "top",
                  spacing     = 5,
                  children = {
                     { widget = "space", value = 5},
                     {
                        widget = "textarea",
                        name   = "fs_window_title",
                        font   = "fs_window_title",
                        text   = "fs_window_title"
                     },
                     {
                        widget = "textarea",
                        name   = "fsmenu_gametip",
                        font   = "fsmenu_gametip",
                        text   = "fsmenu_gametip"
                     },
                     {
                        widget = "textarea",
                        name   = "fsmenu_info_panel_heading",
                        font   = "fsmenu_info_panel_heading",
                        text   = "fsmenu_info_panel_heading"
                     },
                     {
                        widget = "textarea",
                        name   = "fsmenu_info_panel_paragraph",
                        font   = "fsmenu_info_panel_paragraph",
                        text   = "fsmenu_info_panel_paragraph"
                     },
                     {
                        widget = "textarea",
                        name   = "fsmenu_game_setup_headings",
                        font   = "fsmenu_game_setup_headings",
                        text   = "fsmenu_game_setup_headings"
                     },
                     {
                        widget = "textarea",
                        name   = "fsmenu_game_setup_mapname",
                        font   = "fsmenu_game_setup_mapname",
                        text   = "fsmenu_game_setup_mapname"
                     },
                     {
                        widget = "textarea",
                        name   = "fsmenu_game_setup_superuser",
                        font   = "fsmenu_game_setup_superuser",
                        text   = "fsmenu_game_setup_superuser"
                     },
                     {
                        widget = "textarea",
                        name   = "fsmenu_game_setup_irc_client",
                        font   = "fsmenu_game_setup_irc_client",
                        text   = "fsmenu_game_setup_irc_client"
                     },
                     {
                        widget = "textarea",
                        name   = "fsmenu_translation_info",
                        font   = "fsmenu_translation_info",
                        text   = "fsmenu_translation_info"
                     },
                     {
                        widget = "textarea",
                        name   = "label_fs",
                        font   = "label_fs",
                        text   = "label_fs"
                     },
                     {
                        widget = "textarea",
                        name   = "tooltip_header_fs",
                        font   = "tooltip_header_fs",
                        text   = "tooltip_header_fs"
                     },
                     {
                        widget = "textarea",
                        name   = "tooltip_fs",
                        font   = "tooltip_fs",
                        text   = "tooltip_fs"
                     },
                     {
                        widget = "textarea",
                        name   = "tooltip_hotkey_fs",
                        font   = "tooltip_hotkey_fs",
                        text   = "tooltip_hotkey_fs"
                     },
                     {
                        widget = "textarea",
                        name   = "fsmenu_intro",
                        font   = "fsmenu_intro",
                        text   = "fsmenu_intro"
                     },
                     {
                        widget = "textarea",
                        name   = "italic",
                        font   = "italic",
                        text   = "italic"
                     },
                  },
               },
            },
         },
         { widget= "space", value=20},
         {
            widget  = "multilinetextarea",
            name    = "global_fonts_header",
            scroll_mode = "none",
            w       = width,
            font    = "wui_info_panel_heading",
            text    = "Global fonts"
         },
         { widget = "space", value = 5},
         {
            widget      = "box",
            name        = "fonts_box_global",
            orientation = "horizontal",
            resizing    = "fullsize",
            children    = {         
               { widget = "inf_space"},
               {
                  widget = "textarea",
                  name   =  "warning",
                  font   = "warning",
                  text   = "warning"
               },
               { widget = "inf_space"},
               {
                  widget = "textarea",
                  name   =  "disabled",
                  font   = "disabled",
                  text   = "disabled"
               },
               { widget = "inf_space"},
               {
                  widget = "textarea",
                  name   =  "unknown",
                  font   = "unknown",
                  text   = "unknown"
               },
               { widget = "inf_space"},
            },
         },
         { widget= "space", value=20},
      }
   }
})
