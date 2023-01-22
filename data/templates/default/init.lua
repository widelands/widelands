-- This script defines a GUI style for Widelands. At the moment, we only
-- support the default template.

-- Background textures and colors have the format { image = filename, color = {r, g, b } }.

-- Required parameters for font styles are:
-- * face: string
-- * color: table with r, g, b values as int
-- * size: positive int
-- Optional bools are: bold, italic, underline, shadow

-- Some elements carry custom parameters like e.g. "margin" that should be expanded upon in the future.

-- Some common elements for reuse
local fs_button = "fsmenu/button.png"
local wui_button = "wui/button.png"

local fs_blue =  {0, 31, 40}
local fs_green =  {10, 50, 0}
local fs_brown =  {45, 34, 18}

local wui_light =  {85, 63, 35}
local wui_green =  {3, 15, 0}
local wui_brown =  {32, 19, 8}

local fs_font_color = {255, 220, 0}
local fs_font_face = "sans"
local fs_font_size = 14

local wui_font_color = {255, 255, 0}
local wui_font_face = "sans"
local wui_font_size = 14

local ingame_font_face = "sans"
local about_title_color = {47, 145, 49}
local ingame_heading_color = {209, 209, 209}
local ingame_text_color = wui_font_color
local ingame_padding = 6
local ingame_text_space_before = 0
local ingame_text_space_after = ingame_padding
local ingame_heading_space_before_big = 2 * ingame_padding
local ingame_heading_space_before_small = ingame_padding
local ingame_heading_space_after = 2 * ingame_padding

local campaign_leader_color = about_title_color

local unknown_font_color = {209, 45, 45}
local unknown_font_face = "sans"
local unknown_font_size = 16

local default_wui_font = {
   color = wui_font_color,
   face = wui_font_face,
   size = wui_font_size,
   bold = true,
   shadow = true
}
local default_fs_font = {
   color = fs_font_color,
   face = fs_font_face,
   size = fs_font_size,
   bold = true,
   shadow = true
}

local default_button_fonts = {
   enabled_wui = default_wui_font,
   enabled_fs = default_fs_font,
   disabled = {
         color = {127, 127, 127},
         face = fs_font_face,
         size = fs_font_size,
         bold = true,
         shadow = true
   },
}

local productivity_colors = {
   low = {187, 0, 0},
   medium = {255, 225, 30},
   high = {0, 187, 0},
}

-- These are the style definitions to be returned.
-- Note: you have to keep all the keys intact, or Widelands will not be happy.
return {
   -- Automatic resizing of fonts to make them fit onto buttons etc.
   -- won't go below this size
   minimum_font_size = 10,
   minimap_icon_frame = fs_font_color,

   -- red, green, blue, alpha
   background_focused = {240, 240, 240, 200},
   background_semi_focused = {180, 180, 180, 200},
   focus_border_thickness = 2,

   -- Windows
   windows = {
      fsmenu = {
         -- red, green, blue, alpha
         window_border_focused = {220, 220, 250, 40},
         window_border_unfocused = {50, 0, 0, 40},
         background        = "fsmenu/windows/background.png",
         border_top        = "fsmenu/windows/top.png",
         border_bottom     = "fsmenu/windows/bottom.png",
         border_right      = "fsmenu/windows/right.png",
         border_left       = "fsmenu/windows/left.png",
         button_close      = "fsmenu/windows/close.png",
         button_pin        = "fsmenu/windows/pin.png",
         button_unpin      = "fsmenu/windows/unpin.png",
         button_minimize   = "fsmenu/windows/minimize.png",
         button_unminimize = "fsmenu/windows/maximize.png",
      },
      wui = {
         window_border_focused = {220, 220, 250, 40},
         window_border_unfocused = {50, 0, 0, 40},
         background        = "wui/windows/background.png",
         border_top        = "wui/windows/top.png",
         border_bottom     = "wui/windows/bottom.png",
         border_right      = "wui/windows/right.png",
         border_left       = "wui/windows/left.png",
         button_close      = "wui/windows/close.png",
         button_pin        = "wui/windows/pin.png",
         button_unpin      = "wui/windows/unpin.png",
         button_minimize   = "wui/windows/minimize.png",
         button_unminimize = "wui/windows/maximize.png",
      },
   },

   -- Buttons
   buttons = {
      -- Buttons used in Fullscreen menus
      fsmenu = {
         -- Main menu ("Single Player", "Watch Replay", ...)
         menu = {
            enabled = {
               font = default_button_fonts.enabled_fs,
               background = {
                  image = fs_button,
                  color = fs_blue,
               }
            },
            disabled = {
               font = default_button_fonts.disabled,
               background = {
                  image = fs_button,
                  color = fs_blue,
               }
            }
         },
         -- Primary user selection ("OK", ...)
         primary = {
            enabled = {
               font = default_button_fonts.enabled_fs,
               background = {
                  image = fs_button,
                  color = fs_green,
               }
            },
            disabled = {
               font = default_button_fonts.disabled,
               background = {
                  image = fs_button,
                  color = fs_green,
               }
            }
         },
         -- Secondary user selection ("Cancel", "Delete", selection buttons, ...)
         secondary = {
            enabled = {
               font = default_button_fonts.enabled_fs,
               background = {
                  image = fs_button,
                  color = fs_brown,
               }
            },
            disabled = {
               font = default_button_fonts.disabled,
               background = {
                  image = fs_button,
                  color = fs_brown,
               }
            }
         },
      },
      -- Buttons used in-game and in the editor
      wui = {
         -- Main menu ("Exit Game"), Building Windows, selection buttons, ...
         menu = {
            enabled = {
               font = default_button_fonts.enabled_wui,
               background = {
                  image = wui_button,
                  color = wui_light,
               }
            },
            disabled = {
               font = default_button_fonts.disabled,
               background = {
                  image = wui_button,
                  color = wui_light,
               }
            }
         },
         -- Primary user selection ("OK", attack, ...)
         primary = {
            enabled = {
               font = default_button_fonts.enabled_wui,
               background = {
                  image = wui_button,
                  color = wui_green,
               }
            },
            disabled = {
               font = default_button_fonts.disabled,
               background = {
                  image = wui_button,
                  color = wui_green,
               }
            }
         },
         -- Secondary user selection ("Cancel", "Delete", ...)
         secondary = {
            enabled = {
               font = default_button_fonts.enabled_wui,
               background = {
                  image = wui_button,
                  color = wui_brown,
               }
            },
            disabled = {
               font = default_button_fonts.disabled,
               background = {
                  image = wui_button,
                  color = wui_brown,
               }
            }
         },
         -- Building buttons on fieldaction and building statistics need to be
         -- transparent in order to match the background of the tab panel.
         building_stats = {
            enabled = {
               font = default_button_fonts.enabled_wui,
               background = {
                  image = "",
                  color = {0, 0, 0},
               }
            },
            disabled = {
               font = default_button_fonts.disabled,
               background = {
                  image = "",
                  color = {0, 0, 0},
               }
            }
         },
      }
   },
   -- Slider cursors (Sound control, attack, statistics, ...)
   sliders = {
      fsmenu = {
         menu = {
            background = {
               image = fs_button,
               color = fs_blue,
            },
            font = {
               color = fs_font_color,
               face = "condensed",
               size = 11,
               bold = true,
               shadow = true
            }
         }
      },
      wui = {
         -- Sound Options, Statistics
         light = {
            background = {
               image = wui_button,
               color = wui_brown,
            },
            font = {
               color = wui_font_color,
               face = "condensed",
               size = 11,
               bold = true,
               shadow = true
            }
         },
         -- Fieldaction (attack)
         dark = {
            background = {
               image = wui_button,
               color = wui_green,
            },
            font = {
               color = wui_font_color,
               face = "condensed",
               size = 11,
               bold = true,
               shadow = true
            }
         },
      }
   },
   -- Background for tab panels
   tabpanels = {
      fsmenu = {
         -- Options, About, ... this comes with a hard-coded border too
         menu = { image = "", color = {5, 5, 5} },
      },
      wui = {
         -- Most in-game and in-editor tabs. Building windows, Editor tools,
         -- Encyclopedia, ...
         light = { image = "", color = {0, 0, 0} },
         -- Building buttons in Fieldaction and Building Statistics need a dark
         -- background, otherwise the icons will be hard to see.
         dark = { image = wui_button, color = wui_brown },
      }
   },
   -- Used both for one-line and multiline edit boxes
   editboxes = {
      fsmenu = {
         background = {
            image = fs_button,
            color = fs_green,
            margin = 4
         },
         font = {
            color = fs_font_color,
            face = fs_font_face,
            size = fs_font_size,
            shadow = true
         },
      },
      wui = {
         background = {
            image = wui_button,
            color = wui_brown,
            margin = 2,
         },
         font = {
            color = wui_font_color,
            face = wui_font_face,
            size = wui_font_size,
            shadow = true
         },
      },
   },
   -- Background for dropdown menus
   dropdowns = {
      fsmenu = {
         menu = { image = fs_button, color = fs_brown },
      },
      wui = {
         menu = { image = wui_button, color = wui_brown },
      }
   },
   -- Scrollbar buttons, table headers etc.
   scrollbars = {
      fsmenu = {
         menu = { image = fs_button, color = fs_blue },
      },
      wui = {
         menu = { image = wui_button, color = wui_brown },
      }
   },

   -- In-game statistics plots
   statistics_plot = {
      fonts = {
         x_tick = {
            color = { 255, 0, 0 },
            face = "condensed",
            size = 13,
         },
         y_min_value = {
            color = { 125, 0, 0 },
            face = "condensed",
            size = 13,
         },
         y_max_value = {
            color = { 60, 125, 0 },
            face = "condensed",
            size = 13,
         },
      },
      colors = {
         axis_line = { 0, 0, 0 },
         zero_line = { 255, 255, 255 },
      }
   },

   -- Map census and statistics, and building statistics window
   building_statistics = {
      census_font = {
         color = wui_font_color, -- Default color
         face = "condensed",
         size = wui_font_size,
         bold = true,
         shadow = true
      },
      statistics_font = {
         color = wui_font_color, -- Default color
         face = "condensed",
         size = wui_font_size,
         bold = true,
         shadow = true
      },
      -- Building statistics window
      statistics_window = {
         fonts = {
            button_font = {
               color = wui_font_color, -- Default color
               face = "condensed",
               size = 12, -- Do not make this bigger - the UI element size is still hard-coded.
               bold = true,
               shadow = true
            },
            details_font = {
               color = wui_font_color, -- Default color
               face = "sans",
               size = 12, -- Do not make this bigger - the UI element size is still hard-coded.
               bold = true,
               shadow = true
            },
         },
         editbox_margin = 0
      },
      colors = {
         construction = { 163, 144, 19 },
         neutral = { 255, 250, 170 },
         low = productivity_colors["low"],
         medium = productivity_colors["medium"],
         high = productivity_colors["high"],
         low_alt = {200, 100, 100},
         medium_alt = {230, 230, 10},
         high_alt = {50, 225, 50},
      }
   },

   progressbar = {
      fsmenu = {
         font = {
            color = { 128, 128, 255 },
            face = fs_font_face,
            size = fs_font_size,
            bold = true,
            shadow = true
         },
         background_colors = {
            low = { 64, 64, 0 }, -- Unused
            medium = { 64, 64, 0 },
            high = { 64, 64, 0 }, -- Unused
         }
      },
      wui = {
         font = {
            color = {255, 250, 170},
            face = wui_font_face,
            size = wui_font_size,
            bold = true,
            shadow = true
         },
         background_colors = {
            low = productivity_colors["low"],
            medium = productivity_colors["medium"],
            high = productivity_colors["high"],
         }
      }
   },

   tables = {
      fsmenu = {
         enabled = {
            color = fs_font_color,
            face = fs_font_face,
            size = fs_font_size,
            bold = true,
            shadow = true
         },
         disabled = {
            color = {127, 127, 127},
            face = fs_font_face,
            size = fs_font_size,
            bold = true,
            shadow = true
         },
         hotkey = {
            color = {180, 180, 180},
            face = fs_font_face,
            size = fs_font_size,
            bold = true,
            shadow = true
         }
      },
      wui = {
         enabled = {
            color = wui_font_color,
            face = wui_font_face,
            size = wui_font_size,
            bold = true,
            shadow = true
         },
         disabled = {
            color = {127, 127, 127},
            face = wui_font_face,
            size = wui_font_size,
            bold = true,
            shadow = true
         },
         hotkey = {
            color = {180, 180, 180},
            face = wui_font_face,
            size = wui_font_size,
            bold = true,
            shadow = true
         }
      },
   },

   wareinfo = {
      -- TODO(GunChleoc): This design is ugly.
      -- Group stuff more logically once everything has been styled.
      normal = {
         fonts = {
            header = {
               color = wui_font_color,
               face = "sans",
               size = 11,
            },
            info = {
               color = wui_font_color,
               face = "condensed",
               size = 10,
            },
         },
         colors = {
            icon_frame = { 69, 69, 69 },
            icon_background = { 69, 69, 69 },
            info_background = { 0, 0, 0 },
         },
         icon_background_image = "images/wui/ware_list_bg.png",
      },
      highlight = {
         fonts = {
            header = {
               color = wui_font_color,
               face = "sans",
               size = 11,
            },
            info = {
               color = wui_font_color,
               face = "condensed",
               size = 10,
            },
         },
         colors = {
            icon_frame = {255, 255, 0},
            icon_background = {69, 69, 69},
            info_background = {0, 0, 0},
         },
         icon_background_image = "images/wui/ware_list_bg_selected.png",
      }
   },

   -- Font styles. Required parameters are:
   -- * face: string
   -- * color: table with r, g, b values as int
   -- * size: positive int
   -- Optional bools are: bold, italic, underline, shadow
   fonts = {
      -- Basic chat message text color
      chat_message = {
         color = wui_font_color,
         face = "serif",
         size = fs_font_size,
         shadow = true,
      },
      -- Basic chat message text color
      chat_timestamp = {
         color = { 51, 255, 51 },
         face = "serif",
         size = 9,
         shadow = true,
      },
      -- Chat for private messages
      chat_whisper = {
         color = { 238, 238, 238 },
         face = "serif",
         size = fs_font_size,
         italic = true,
         shadow = true,
      },
      -- Chat playername highlight
      chat_playername = {
         color = { 255, 255, 255 },
         face = "serif",
         size = fs_font_size,
         bold = true,
         shadow = true,
      },
      -- Chat log messages color. Also doubles as spectator playercolor for chat messages.
      chat_server = {
         color = { 221, 221, 221 },
         face = "serif",
         size = fs_font_size,
         bold = true,
         shadow = true,
      },
      -- Intro screen
      fsmenu_intro = {
         color = { 192, 192, 128 },
         face = fs_font_face,
         size = 16,
         bold = true,
         shadow = true
      },
      italic = {
         color = { 238, 238, 238 },
         face = "serif",
         size = fs_font_size,
         italic = true,
         shadow = true,
      },
      -- Displayed in the loading screens
      fsmenu_gametip = {
         color = { 33, 33, 27 },
         face = "serif",
         size = 16,
      },
      -- Game and Map info panels
      fsmenu_info_panel_heading = {
         color = fs_font_color,
         face = fs_font_face,
         size = fs_font_size,
         bold = true,
         shadow = true
      },
      fsmenu_info_panel_paragraph = {
         color = { 209, 209, 209 },
         face = fs_font_face,
         size = fs_font_size,
         shadow = true
      },
      -- Internet lobby and launch game
      fsmenu_game_setup_headings = {
         color = { 127, 255, 191 },
         face = fs_font_face,
         size = fs_font_size,
         bold = true,
         shadow = true
      },
      fsmenu_game_setup_mapname = {
         color = { 255, 255, 127 },
         face = fs_font_face,
         size = fs_font_size,
         bold = true,
         shadow = true
      },
      -- List admin in the internet lobby
      fsmenu_game_setup_superuser = {
         color = { 0, 255, 0 },
         face = fs_font_face,
         size = fs_font_size,
         bold = true,
         shadow = true
      },
      -- List IRC clients in the internet lobby
      fsmenu_game_setup_irc_client = {
         color = { 221, 221, 221 },
         face = fs_font_face,
         size = fs_font_size,
         bold = true,
         shadow = true
      },
      -- Page titles. Used in game summary TODO(GunChleoc): Refactor game summary
      game_summary_title = {
         color = wui_font_color,
         face = wui_font_face,
         size = 22,
         bold = true,
         shadow = true
      },
      -- Make font a bit smaller so the link will fit at 800x600 resolution.
      fsmenu_translation_info = {
         color = fs_font_color,
         face = fs_font_face,
         size = fs_font_size - 2,
         bold = true,
         shadow = true
      },

      -- Textarea default style, also used for sliders, checkboxes
      label_wui = default_wui_font,
      label_fs = default_fs_font,
      tooltip_wui = {
         color = wui_font_color,
         face = wui_font_face,
         size = 14,
         bold = false,
      },
      tooltip_hotkey_wui = {
         color = {180, 180, 180},
         face = wui_font_face,
         size = 14,
         bold = false,
      },
      tooltip_header_wui = {
         color = wui_font_color,
         face = wui_font_face,
         size = 16,
         bold = true,
      },
      tooltip_fs = {
         color = fs_font_color,
         face = fs_font_face,
         size = 14,
         bold = false,
      },
      tooltip_hotkey_fs = {
         color = {180, 180, 180},
         face = fs_font_face,
         size = 14,
         bold = false,
      },
      tooltip_header_fs = {
         color = fs_font_color,
         face = fs_font_face,
         size = 16,
         bold = true,
      },
      warning = {
         color = {255, 22, 22},
         face = wui_font_face,
         size = wui_font_size,
         bold = true,
         shadow = true
      },
      disabled = {
         color = {127, 127, 127},
         face = wui_font_face,
         size = wui_font_size,
         bold = true,
         shadow = true
      },

      wui_attack_box_slider_label = {
         color = wui_font_color,
         face = wui_font_face,
         size = 10,
         bold = true,
         shadow = true
      },
      wui_info_panel_heading = {
         color = { 209, 209, 209 },
         face = wui_font_face,
         size = wui_font_size,
         bold = true,
      },
      wui_info_panel_paragraph = {
         color = { 255, 255, 0 },
         face = wui_font_face,
         size = wui_font_size,
      },
      -- Messages
      wui_message_heading = {
         color = { 209, 209, 209 },
         face = wui_font_face,
         size = 18,
         bold = true,
      },
      wui_message_paragraph = {
         color = { 255, 255, 0 },
         face = wui_font_face,
         size = 12,
      },
      wui_window_title = {
         color = wui_font_color,
         face = wui_font_face,
         size = 13,
         bold=true,
         shadow=true,
      },
      fs_window_title = {
         color = fs_font_color,
         face = fs_font_face,
         size = 13,
         bold=true,
         shadow=true,
      },
      wui_game_speed_and_coordinates = {
         color = wui_font_color,
         face = "condensed",
         size = wui_font_size,
         bold = true,
         shadow = true
      },

      -- Returned when lookup by name fails.
      -- Make this stand out to help debugging
      unknown = {
         color = unknown_font_color,
         face = unknown_font_face,
         size = unknown_font_size,
         bold = true,
      }
   },

   paragraphs = {
      -- Campaign and help texts
      readme_title = {
         -- this is used for the "Widelands" title
         font = {
            color = about_title_color,
            -- should be "Widelands/Widelands.ttf", but it's not possible to pass font filename here
            face = ingame_font_face,
            size = 38,
         },
         align = "center",
         valign = "center",
         space_before = 0,
         space_after = ingame_text_space_after,
      },
      about_title = {
         font = {
            color = about_title_color,
            face = ingame_font_face,
            size = 28,
         },
         align = "center",
         space_before = ingame_text_space_before,
         space_after = ingame_text_space_after,
      },
      ingame_subtitle = {
         font = {
            color = ingame_heading_color,
            face = ingame_font_face,
            size = 14,
            italic = true,
         },
         align = "center",
         space_before = ingame_text_space_before,
         space_after = ingame_text_space_after,
      },
      authors_heading_1 = {
         font = {
            color = {244, 161, 49},
            face = ingame_font_face,
            size = 24,
         },
         space_before = ingame_heading_space_before_big,
         space_after = ingame_heading_space_after,
      },
      ingame_heading_1 = {
         font = {
            color = ingame_heading_color,
            face = ingame_font_face,
            size = 18,
            bold = true,
         },
         space_before = ingame_heading_space_before_big,
         space_after = ingame_heading_space_after,
      },
      ingame_objectives_heading = {
         font = {
            color = ingame_heading_color,
            face = ingame_font_face,
            size = 18,
            bold = true,
         },
         space_before = ingame_heading_space_before_small,
         space_after = ingame_heading_space_after,
      },
      ingame_heading_2 = {
         font = {
            color = ingame_heading_color,
            face = ingame_font_face,
            size = 14,
            bold = true,
         },
         space_before = ingame_heading_space_before_big,
         space_after = ingame_heading_space_after,
      },
      ingame_heading_3 = {
         font = {
            color = ingame_heading_color,
            face = ingame_font_face,
            size = 13,
         },
         space_before = ingame_heading_space_before_small,
         space_after = ingame_heading_space_after,
      },
      ingame_heading_4 = {
         font = {
            color = ingame_heading_color,
            face = ingame_font_face,
            size = 12,
            italic = true,
         },
         space_before = ingame_text_space_before,
         space_after = ingame_text_space_after,
      },
      ingame_text = {
         font = {
            color = ingame_text_color,
            face = ingame_font_face,
            size = 12,
         },
         space_before = ingame_text_space_before,
         space_after = ingame_text_space_after,
      },
      ingame_sparse = {
         font = {
            color = ingame_text_color,
            face = ingame_font_face,
            size = 12,
         },
         space_before = ingame_text_space_before + ingame_padding,
         space_after = ingame_text_space_after,
      },
      ingame_image_line = {
         font = {
            color = ingame_text_color,
            face = ingame_font_face,
            size = 12,
         },
         space_before = ingame_text_space_before + ingame_padding,
         space_after = ingame_text_space_after + ingame_padding,
      },
      ingame_lore_author = {
         font = {
            color = ingame_text_color,
            face = ingame_font_face,
            size = 10,
            italic = true,
         },
         align = "right",
         space_before = ingame_text_space_before,
         space_after = ingame_text_space_after + 3,
      },
      about_text = {
         font = {
            color = fs_font_color,
            face = fs_font_face,
            size = 12,
         },
         space_before = ingame_text_space_before,
         space_after = ingame_text_space_after,
      },
      about_sparse = {
         font = {
            color = fs_font_color,
            face = fs_font_face,
            size = 12,
         },
         space_before = ingame_text_space_before + ingame_padding,
         space_after = ingame_text_space_after,
      },


      -- Returned when lookup by name fails.
      -- Make this stand out to help debugging
      unknown = {
         font = {
            color = unknown_font_color,
            face = unknown_font_face,
            size = unknown_font_size,
            italic = true,
         },
         space_before = unknown_font_size,
         space_after = unknown_font_size,
      },
   },

   colors = {
      campaign_bar_thron = campaign_leader_color,
      campaign_bar_boldreth = {0xFF, 0x00, 0x00},
      campaign_bar_khantrukh = {0x80, 0x80, 0xFF},

      campaign_emp_lutius = campaign_leader_color,
      campaign_emp_amalea = {0xFF, 0x00, 0x00},
      campaign_emp_saledus = {0x8F, 0x91, 0x31},
      campaign_emp_marcus = {0x11, 0x75, 0xAF},
      campaign_emp_julia = {0xD0, 0xD0, 0xD0},

      campaign_atl_jundlina = campaign_leader_color,
      campaign_atl_sidolus = {0xFF, 0x1A, 0x30},
      campaign_atl_loftomor = {0xFD, 0xD5, 0x3D},
      campaign_atl_colionder = {0x33, 0xA9, 0xFD},
      campaign_atl_opol = {0xFF, 0x3F, 0xFB},
      campaign_atl_ostur = {0x37, 0x5F, 0xFC},
      campaign_atl_kalitath = {0xFF, 0x00, 0x00},

      campaign_fri_reebaud = {0x55, 0xBB, 0x55},
      campaign_fri_hauke = {0x77, 0x99, 0xBB},
      campaign_fri_maukor = {0x99, 0x00, 0x00},
      campaign_fri_murilius = {0xBB, 0x77, 0x55},
      campaign_fri_claus = {0x1C, 0xC2, 0x00},
      campaign_fri_henneke = {0xF7, 0xFF, 0x00},
      campaign_fri_iniucundus = {0xFF, 0x00, 0x59},
      campaign_fri_angadthur = {0x00, 0x16, 0xFF},
      campaign_fri_amazon = {0x6C, 0x00, 0x97},
      campaign_fri_ketelsen = {0x44, 0x55, 0xCC},

      map_river_advisor = {0xEE, 0xBB, 0x00},

      unknown = {73, 64, 55}
   },

   dimensions = {
      text_default_gap = ingame_padding,
      text_space_before_inline_header = 8,
      ingame_message_gap = 8,
      ingame_space_before_objective = 18,
      ingame_space_before_immovable_icon = 14,
      help_terrain_tree_header_space_before = 12,
      help_terrain_tree_header_space_after = 3,
      editor_tooltip_icon_gap = 2,
      campaign_message_box_default_h = 400,
      campaign_message_box_default_w = 450,
      campaign_message_box_top_pos_y = 25,
      campaign_fri02_poem_indent = 10,
      map_plateau_message_w = 400,
      map_plateau_message_h = 300,
      map_plateau_message_pos_y = 1,
      ui_default_padding = 4,
   }
}
