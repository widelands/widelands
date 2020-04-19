#include "mapdetailsbox.h"

#include "base/i18n.h"

MapDetailsBox::MapDetailsBox(Panel* parent,
                             int32_t x,
                             int32_t y,
                             int32_t padding,
                             int32_t indent,
                             int32_t max_x,
                             int32_t max_y)
   : UI::Box(parent, x, y, UI::Box::Vertical, max_x, max_y),
     title_box_(this, 0, 0, UI::Box::Horizontal),
     map_name_(&title_box_,
               0,
               0,
               0,
               0,
               "Name der Map",
               UI::Align::kLeft,
               g_gr->styles().font_style(UI::FontStyle::kLabel)),
     select_map_(&title_box_,
                 "change_map_or_save2",
                 0,
                 0,
                 20,
                 20,
                 UI::ButtonStyle::kFsMenuSecondary,
                 g_gr->images().get("images/wui/menus/toggle_minimap.png"),
                 _("Change map or saved game")),
     map_description_(
        this,
        0,
        0,
        50,  // arbitrary, will be set while layouting boxes...
        50,
        UI::PanelStyle::kFsMenu,
        "sample map description which might be a\n very long text so scrollbar is needed") {
}
MapDetailsBox::~MapDetailsBox() {
}
