#include "ui_fsmenu/mapdetailsbox.h"

#include "logic/game_settings.h"
#include "map_io/map_loader.h"

MapDetailsBox::MapDetailsBox(Panel* parent,
                             uint32_t,
                             uint32_t standard_element_height,
                             uint32_t padding,
                             int32_t max_x,
                             int32_t max_y)
   : UI::Box(parent, 0, 0, UI::Box::Vertical),
     title_(this,
            0,
            0,
            0,
            0,
            _("Map"),
            UI::Align::kCenter,
            g_gr->styles().font_style(UI::FontStyle::kFsGameSetupHeadings)),
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
                 "change_map_or_save",
                 0,
                 0,
                 standard_element_height,
                 standard_element_height,
                 UI::ButtonStyle::kFsMenuSecondary,
                 g_gr->images().get("images/wui/menus/toggle_minimap.png"),
                 _("Change map or saved game")),
     map_description_(
        this,
        0,
        0,
        UI::Scrollbar::kSize,  // min width must be set to avoid assertion failure...
        0,
        UI::PanelStyle::kFsMenu,
        "sample map description which might be a\n very long text so scrollbar is needed") {
	add(&title_, Resizing::kAlign, UI::Align::kCenter);
	add_space(3 * padding);
	title_box_.add(&map_name_, UI::Box::Resizing::kAlign, UI::Align::kLeft);
	title_box_.add_inf_space();
	title_box_.add(&select_map_, UI::Box::Resizing::kAlign, UI::Align::kRight);
	add(&title_box_, UI::Box::Resizing::kFullSize);
	add(&map_description_, UI::Box::Resizing::kExpandBoth);
}
MapDetailsBox::~MapDetailsBox() {
}

void MapDetailsBox::update(GameSettingsProvider* settings, Widelands::Map& map) {
	const GameSettings& game_settings = settings->settings();
	{
		// Translate the map's name
		const char* nomap = _("(no map)");
		i18n::Textdomain td("maps");
		map_name_.set_text(game_settings.mapname.size() != 0 ? _(game_settings.mapname) : nomap);
	}

	select_map_.set_visible(settings->can_change_map());
	select_map_.set_enabled(settings->can_change_map());

	show_map_description(map, settings);
}

void MapDetailsBox::set_select_map_action(std::function<void()> action) {
	select_map_.sigclicked.connect(action);
}
void MapDetailsBox::force_new_dimensions(float scale,
                                         uint32_t standard_element_width,
                                         uint32_t standard_element_height) {
	title_.set_font_scale(scale);
	map_name_.set_font_scale(scale);
	map_name_.set_fixed_width(standard_element_width - standard_element_height);
	select_map_.set_desired_size(standard_element_height, standard_element_height);
	map_description_.set_desired_size(0, 4 * standard_element_height);
	UI::Box::layout();
}
void MapDetailsBox::set_map_description_text(const std::string& text) {
	map_description_.set_text(text);
}
void MapDetailsBox::show_map_description(Widelands::Map& map, GameSettingsProvider* settings) {
	const GameSettings& game_settings = settings->settings();
	std::string infotext;
	infotext += std::string(_("Map details:")) + "\n";
	infotext += std::string("• ") +
	            (boost::format(_("Size: %1% x %2%")) % map.get_width() % map.get_height()).str() +
	            "\n";
	infotext += std::string("• ") +
	            (boost::format(ngettext("%u Player", "%u Players", game_settings.players.size())) %
	             static_cast<unsigned int>(game_settings.players.size()))
	               .str() +
	            "\n";
	if (game_settings.scenario)
		infotext += std::string("• ") + (boost::format(_("Scenario mode selected"))).str() + "\n";
	infotext += "\n";
	infotext += map.get_description();
	infotext += "\n";
	infotext += map.get_hint();

	map_description_.set_text(infotext);
}
