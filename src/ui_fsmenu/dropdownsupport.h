#ifndef WL_UI_FSMENU_DROPDOWNSUPPORT_H
#define WL_UI_FSMENU_DROPDOWNSUPPORT_H
#include <memory>
#include <string>

#include "logic/game_settings.h"
#include "ui_basic/dropdown.h"

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
	                UI::ButtonStyle button_style,
	                GameSettingsProvider* const settings,
	                PlayerSlot id)
	   : dropdown_(parent,
	               name,
	               x,
	               y,
	               w,
	               max_list_items,
	               button_dimension,
	               label,
	               type,
	               style,
	               button_style),
	     settings_(settings),
	     id_(id),
	     selection_locked_(false) {
		dropdown_.set_disable_style(UI::ButtonDisableStyle::kFlat);
		dropdown_.selected.connect([this]() { on_dropdown_selected(); });
	}
	virtual ~DropDownSupport() {
	}

	UI::Panel* get_dropdown() {
		return &dropdown_;
	}
	virtual void selection_action() {
	}

	virtual void rebuild() = 0;

	void set_visible(bool visible) {
		dropdown_.set_visible(visible);
	}
	void set_enabled(bool enable) {
		dropdown_.set_enabled(enable);
	}

protected:
	UI::Dropdown<T> dropdown_;
	GameSettingsProvider* const settings_;
	PlayerSlot const id_;
	bool selection_locked_;

private:
	void on_dropdown_selected() {
		selection_locked_ = true;
		selection_action();
		selection_locked_ = false;
	}
};

class TribeDropdownSupport : public DropDownSupport<std::string> {
public:
	TribeDropdownSupport(UI::Panel* parent,
	                     const std::string& name,
	                     int32_t x,
	                     int32_t y,
	                     uint32_t w,
	                     int button_dimension,
	                     GameSettingsProvider* const settings,
	                     PlayerSlot id);
	void rebuild() override;

private:
	void selection_action() override;
};

class PlayerTypeDropdownSupport : public DropDownSupport<std::string> {
public:
	PlayerTypeDropdownSupport(UI::Panel* parent,
	                    const std::string& name,
	                    int32_t x,
	                    int32_t y,
	                    uint32_t w,
	                    int button_dimension,
	                    GameSettingsProvider* const settings,
	                    PlayerSlot id);
	void rebuild() override;

private:
	void fill();
	void select_entry();
	void selection_action() override;
};

class StartTypeDropdownSupport : public DropDownSupport<uintptr_t> {
public:
	StartTypeDropdownSupport(UI::Panel* parent,
	                    const std::string& name,
	                    int32_t x,
	                    int32_t y,
	                    uint32_t w,
	                    int button_dimension,
	                    GameSettingsProvider* const settings,
	                    PlayerSlot id);

	void rebuild() override;

private:
	void fill();
	void selection_action() override;
};

class TeamDropdown : public DropDownSupport<uintptr_t> {
public:
	TeamDropdown(UI::Panel* parent,
	             const std::string& name,
	             int32_t x,
	             int32_t y,
	             uint32_t w,
	             int button_dimension,
	             GameSettingsProvider* const settings,
	             PlayerSlot id);

	void rebuild() override;

private:
	void selection_action() override;
};

#endif  // WL_UI_FSMENU_DROPDOWNSUPPORT_H
