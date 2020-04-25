#ifndef WL_UI_FSMENU_DROPDOWNSUPPORT_H
#define WL_UI_FSMENU_DROPDOWNSUPPORT_H

#include <string>

#include "ui_basic/dropdown.h"
class GameSettings;

template <typename T> class DropDownSupport {
public:
	DropDownSupport(UI::Panel* parent,
	                const std::string& name,
	                int32_t x,
	                int32_t y,
	                uint32_t w,
	                uint32_t max_list_items,
	                int button_dimension,
	                const std::string& label,
	                const UI::DropdownType type,
	                UI::PanelStyle style,
	                UI::ButtonStyle button_style);
	virtual ~DropDownSupport();

	UI::Dropdown<T>& get_dropdown();

	virtual void rebuild(const GameSettings& settings);
	virtual void fill(const GameSettings& settings);
	virtual void select_entry(const GameSettings& settings);

private:
	UI::Dropdown<T> dropdown_;
};

class TribeDropdownSupport : DropDownSupport<std::string> {
public:
	TribeDropdownSupport(UI::Panel* parent,
	                     const std::string& name,
	                     int32_t x,
	                     int32_t y,
	                     uint32_t w,
	                     uint32_t max_list_items,
	                     int button_dimension,
	                     const std::string& label);
	void rebuild(const GameSettings& settings) override;
	void fill(const GameSettings& settings) override;
	void select_entry(const GameSettings& settings) override;
};

#endif  // WL_UI_FSMENU_DROPDOWNSUPPORT_H
