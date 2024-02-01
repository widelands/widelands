-- Show wui buttons
-- ----------------

-- If the button styles changes of the default theme, use this as
-- an editor addon to make a screenshot and save it as
-- doc/sphinx/source/images/buttons.png
-- See: https://www.widelands.org/documentation/themes/#buttons

if not wl.Editor then return end  -- Only in the editor

wl.ui.MapView():add_toolbar_plugin(
   [[ include("addons/buttons_apearance.wad/windows.lua") ]],
   "images/wui/fieldaction/menu_debug.png",
   "Buttons styles",
   ""
)
