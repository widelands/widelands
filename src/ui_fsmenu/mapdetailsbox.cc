#include "mapdetailsbox.h"

#include "base/i18n.h"
#include "logic/game_settings.h"
#include "logic/map.h"
#include "map_io/map_loader.h"

MapDetailsBox::MapDetailsBox(Panel* parent,
                             int32_t x,
                             int32_t y,
                             int32_t padding,
                             int32_t indent,
                             int32_t max_x,
                             int32_t max_y)
   : UI::Box(parent, x, y, UI::Box::Vertical, max_x, max_y),
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
	add(&title_, Resizing::kAlign, UI::Align::kCenter);
	title_box_.add(&map_name_, UI::Box::Resizing::kAlign, UI::Align::kLeft);
	title_box_.add_inf_space();
	title_box_.add(&select_map_, UI::Box::Resizing::kAlign, UI::Align::kRight);
	add(&title_box_, UI::Box::Resizing::kFullSize);
	add(&map_description_, UI::Box::Resizing::kFullSize);

	//	map_name_.set_font_scale(2);
}
MapDetailsBox::~MapDetailsBox() {
}

void MapDetailsBox::update(GameSettingsProvider* settings) {
	const GameSettings& game_settings = settings->settings();
	{
		// Translate the map's name
		const char* nomap = _("(no map)");
		i18n::Textdomain td("maps");
		map_name_.set_text(game_settings.mapname.size() != 0 ? _(game_settings.mapname) : nomap);
	}

	select_map_.set_visible(settings->can_change_map());
	select_map_.set_enabled(settings->can_change_map());

	load_map_info(settings);
}

/**
 * load map information and update the UI
 */
void MapDetailsBox::load_map_info(GameSettingsProvider* settings) {
	const GameSettings& game_settings = settings->settings();
	Widelands::Map map;  //  MapLoader needs a place to put its preload data

	std::unique_ptr<Widelands::MapLoader> ml = map.get_correct_loader(game_settings.mapfilename);
	//	if (!ml) {
	//		throw WLWarning("There was an error!", "The map file seems to be invalid!");
	//	}

	map.set_filename(game_settings.mapfilename);
	{
		i18n::Textdomain td("maps");
		ml->preload_map(true);
	}

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

void MapDetailsBox::set_select_map_action(std::function<void()> action) {
	select_map_.sigclicked.connect(action);
}
