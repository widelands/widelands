include "scripting/richtext.lua"
include "txts/help/common_helptexts.lua"

local toggle_minimap_hotkey = help_toggle_minimap_hotkey()
local toggle_building_spaces_hotkey = help_toggle_building_spaces_hotkey()
local toggle_fullscreen_hotkey = help_toggle_fullscreen_hotkey()

push_textdomain("widelands_editor")

local r = {
   title = _("Controls"),
   text =
      h2(_("Keyboard Shortcuts")) ..
      p(
         -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
         dl(help_format_hotkey(wl.ui.get_shortcut("encyclopedia")), _("Help")) ..
         -- TRANSLATORS: This is an access key combination.
         dl(help_format_hotkey(wl.ui.get_shortcut("editor_menu")), _("Toggle main menu")) ..
         -- TRANSLATORS: This is an access key combination. The hotkey is 't'
         dl(help_format_hotkey(wl.ui.get_shortcut("editor_tools")), _("Toggle tools menu")) ..
         toggle_minimap_hotkey ..
         toggle_building_spaces_hotkey ..
         -- TRANSLATORS: This is an access key combination. The hotkey is 'p'
         dl(help_format_hotkey(wl.ui.get_shortcut("editor_players")), _("Toggle player menu")) ..
         -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
         dl(help_format_hotkey(wl.ui.get_shortcut("editor_undo")), _("Undo")) ..
         -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
         dl(help_format_hotkey(wl.ui.get_shortcut("editor_redo")), _("Redo")) ..
         -- TRANSLATORS: This is an access key combination. The hotkey is 'i'
         dl(help_format_hotkey(wl.ui.get_shortcut("editor_info")), _("Activate information tool")) ..
         -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
         dl(help_format_hotkey(wl.ui.get_shortcut("editor_load")), _("Load map")) ..
         -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
         dl(help_format_hotkey(wl.ui.get_shortcut("editor_save")), _("Save map")) ..
            toggle_fullscreen_hotkey
      ) ..

      h2(_("Tools")) ..
      p(
         -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
         dl(help_format_hotkey(help_editor_toolsize_tips()), _("Change tool size")) ..
         -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
         dl(help_format_hotkey(pgettext("hotkey", "Click")), _("Place new elements on the map, or increase map elements by the value selected by ‘Increase/Decrease value’")) ..
         -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
         dl(help_format_hotkey(pgettext("hotkey", "Shift + Click")), _("Remove elements from the map, or decrease map elements by the value selected by ‘Increase/Decrease value’")) ..
         -- TRANSLATORS: This is an access key combination. Localize, but do not change the key.
         dl(help_format_hotkey(pgettext("hotkey", "Ctrl + Click")), _("Set map elements to the value selected by ‘Set Value’"))
      )
}
pop_textdomain()
return r
