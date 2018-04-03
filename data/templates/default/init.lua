-- This script defines a GUI style for Widelands. At the moment, we only
-- support the default template.
-- NOCOM update documentation
-- So far, only background textures and colors can be defined, and they all have
-- the format { image = filename, color = {r, g, b } }.

dirname = path.dirname(__file__)

-- Some common elements for reuse
local fs_button = dirname .. "fsmenu/button.png"
local wui_button = dirname .. "wui/button.png"

local fs_blue =  {0, 31, 40}
local fs_green =  {10, 50, 0}
local fs_brown =  {45, 34, 18}

local wui_light =  {85, 63, 35}
local wui_green =  {3, 15, 0}
local wui_brown =  {32, 19, 8}

local fs_font_color = {255, 255, 0}
local fs_font_face = "sans"
local fs_font_size = 14

local wui_font_color = {255, 255, 0}
local wui_font_face = "sans"
local wui_font_size = 14

local default_ui_font = {
   color = fs_font_color,
   face = fs_font_face,
   size = fs_font_size,
   bold = true,
   shadow = true
}

local default_button_fonts = {
   enabled = default_ui_font,
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

   -- Button backgrounds
   buttons = {
      -- Buttons used in Fullscreen menus
      fsmenu = {
         -- Main menu ("Single Player", "Watch Replay", ...)
         menu = {
            image = fs_button,
            color = fs_blue,
            fonts = default_button_fonts
         },
         -- Primary user selection ("OK", ...)
         primary = {
            image = fs_button,
            color = fs_green,
            fonts = default_button_fonts
         },
         -- Secondary user selection ("Cancel", "Delete", selection buttons, ...)
         secondary = {
            image = fs_button,
            color = fs_brown,
            fonts = default_button_fonts
         },
      },
      -- Buttons used in-game and in the editor
      wui = {
         -- Main menu ("Exit Game"), Building Windows, selection buttons, ...
         menu = {
            image = wui_button,
            color = wui_light,
            fonts = default_button_fonts
         },
         -- Primary user selection ("OK", attack, ...)
         primary = {
            image = wui_button,
            color = wui_green,
            fonts = default_button_fonts
         },
         -- Secondary user selection ("Cancel", "Delete", ...)
         secondary = {
            image = wui_button,
            color = wui_brown,
            fonts = default_button_fonts
         },
         -- Building buttons on fieldaction and building statistics need to be
         -- transparent in order to match the background of the tab panel.
         building_stats = {
            image = "",
            color = {0, 0, 0},
            fonts = default_button_fonts
         },
      }
   },
   -- Slider cursors (Sound control, attack, statistics, ...)
   sliders = {
      fsmenu = {
         menu = {
            image = fs_button,
            color = fs_blue,
            fonts = {
               labels = {
                  color = fs_font_color,
                  face = "condensed",
                  size = 11,
                  bold = true,
                  shadow = true
               }
            }
         }
      },
      wui = {
         -- Sound Options, Statistics
         light = {
            image = wui_button,
            color = wui_brown,
            fonts = {
               labels = {
                  color = fs_font_color,
                  face = "condensed",
                  size = 11,
                  bold = true,
                  shadow = true
               }
            }
         },
         -- Fieldaction (attack)
         dark = {
            image = wui_button,
            color = wui_green,
            fonts = {
               labels = {
                  color = fs_font_color,
                  face = "condensed",
                  size = 11,
                  bold = true,
                  shadow = true
               }
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
         menu = {
            image = fs_button,
            color = fs_green,
            fonts = {
               default = {
                  color = fs_font_color,
                  face = fs_font_face,
                  size = fs_font_size,
                  shadow = true
               }
            }
         },
      },
      wui = {
         menu = {
            image = wui_button,
            color = wui_brown,
            fonts = {
               default = {
                  color = wui_font_color,
                  face = wui_font_face,
                  size = wui_font_size,
                  shadow = true
               }
            }
         },
      }
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

   map_object = {
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
      building_statistics_font = {
         color = wui_font_color, -- Default color
         face = "condensed",
         size = 12, -- Do not make this bigger - the UI element size is still hard-coded.
         bold = true,
         shadow = true
      },
      colors = {
         construction = { 163, 144, 19 },
         neutral = { 255, 250, 170 },
         low = productivity_colors["low"],
         medium = productivity_colors["medium"],
         high = productivity_colors["high"],
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
         }
      },
      wui = {
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
   -- * size; int
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
         underline = true,
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
      -- Displayed in the loading screens
      fsmenu_gametip = {
         color = { 0, 0, 0 },
         face = "serif",
         size = 16,
      },
      -- Game and Map info panels
      fsmenu_info_panel_heading = {
         color = { 255, 255, 0 },
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
         color = { 0, 255, 0 },
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
      -- List IRC clients in the internet lobby
      fsmenu_game_setup_irc_client = {
         color = { 0, 255, 0 },
         face = fs_font_face,
         size = fs_font_size,
         bold = true,
         shadow = true
      },
      -- Page titles. Also used in game summary TODO(GunChleoc): Refactor game summary
      fsmenu_title = {
         color = fs_font_color,
         face = fs_font_face,
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

      -- Textarea default style, also used for sliders, checkboxes, both in fsmenu and wui ...
      label = default_ui_font,
      tooltip = {
         color = fs_font_color,
         face = fs_font_face,
         size = fs_font_size,
         bold = true,
      },
      warning = {
         color = {255, 22, 22},
         face = fs_font_face,
         size = fs_font_size,
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
         color = fs_font_color,
         face = wui_font_face,
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
   },
}
