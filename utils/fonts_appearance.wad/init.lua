-- Show all available font styles.
-- -------------------------------

-- If the fonts style changes of the default theme, use this as
-- editor addon to make a screenshot and save it as
-- doc/sphinx/source/images/fonts_styles.png
-- See https://www.widelands.org/documentation/themes/#fonts

if not wl.Editor then return end  -- Only in the editor

wl.ui.MapView():add_toolbar_plugin(
   [[ include("addons/fonts_appearance.wad/windows.lua") ]],
   "images/wui/fieldaction/menu_debug.png",
   "Fonts and styles",
   ""
)
