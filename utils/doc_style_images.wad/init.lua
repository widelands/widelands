if not wl.Editor then return end  -- Only in the editor

-- Display styles
-- --------------

-- If a style of the default theme changes, use this as an
-- editor addon to make screenshots.

-- Buttons
-- Save screenshot as doc/sphinx/source/images/buttons.png
-- See: https://www.widelands.org/documentation/themes/#buttons
wl.ui.MapView():add_toolbar_plugin(
   [[ include("addons/doc_style_images.wad/buttons_appereance.lua") ]],
   "images/wui/fieldaction/menu_debug.png",
   "Button styles",
   ""
)

-- Fonts
-- Save screenshot as doc/sphinx/source/images/fonts_styles.png
-- See https://www.widelands.org/documentation/themes/#fonts
wl.ui.MapView():add_toolbar_plugin(
   [[ include("addons/doc_style_images.wad/fonts_appereance.lua") ]],
   "images/wui/fieldaction/menu_debug.png",
   "Font styles",
   ""
)
