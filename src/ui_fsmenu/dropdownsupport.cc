#include "dropdownsupport.h"
template <typename T>
DropDownSupport<T>::DropDownSupport(UI::Panel* parent,
                                    const std::string& name,
                                    int32_t x,
                                    int32_t y,
                                    uint32_t w,
                                    uint32_t max_list_items,
                                    int button_dimension,
                                    const std::string& label,
                                    const UI::DropdownType type,
                                    UI::PanelStyle style,
                                    UI::ButtonStyle button_style) {
}

TribeDropdownSupport::TribeDropdownSupport(UI::Panel* parent,
                                           const std::string& name,
                                           int32_t x,
                                           int32_t y,
                                           uint32_t w,
                                           uint32_t max_list_items,
                                           int button_dimension,
                                           const std::string& label)
   : DropDownSupport<std::string>(parent,
                                  name,
                                  x,
                                  y,
                                  w,
                                  max_list_items,
                                  button_dimension,
                                  label,
                                  UI::DropdownType::kPictorial,
                                  UI::PanelStyle::kFsMenu,
                                  UI::ButtonStyle::kFsMenuSecondary) {
}
