REM WARNING: Only touch this if you are highly masochistic
REM Add SDL/other library include dirs to your console settings

set myincludes=/Isrc /Isrc\ui\ui_basic /Isrc\ui\ui_fs_menus
set defines=/Dsnprintf=_snprintf /D__i386__ /Dstrcasecmp=stricmp /Dvsnprintf=_vsnprintf /DWIN32

cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\ui\ui_basic\ui_box.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\ui\ui_basic\ui_button.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\ui\ui_basic\ui_checkbox.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\ui\ui_basic\ui_editbox.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\ui\ui_basic\ui_icongrid.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\ui\ui_basic\ui_listselect.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\ui\ui_basic\ui_multilinetextarea.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\ui\ui_basic\ui_panel.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\ui\ui_basic\ui_progressbar.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\ui\ui_basic\ui_radiobutton.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\ui\ui_basic\ui_scrollbar.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\ui\ui_basic\ui_tabpanel.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\ui\ui_basic\ui_textarea.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\ui\ui_basic\ui_unique_window.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\ui\ui_basic\ui_window.cc


cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\ui\ui_fs_menus\fullscreen_menu_base.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\ui\ui_fs_menus\fullscreen_menu_fileview.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\ui\ui_fs_menus\fullscreen_menu_intro.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\ui\ui_fs_menus\fullscreen_menu_launchgame.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\ui\ui_fs_menus\fullscreen_menu_main.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\ui\ui_fs_menus\fullscreen_menu_mapselect.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\ui\ui_fs_menus\fullscreen_menu_options.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\ui\ui_fs_menus\fullscreen_menu_singleplayer.cc


cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\animation.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\bob.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\building.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\building_ui.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\cmd_queue.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\constructionsite.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\criterr.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\editor.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\editor_game_base.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\editor_menus.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\editor_tools.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\editor_tools_option_menus.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\editorinteractive.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\field.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\fieldaction.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\files.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\font.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\game.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\game_debug_ui.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\helper.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\immovable.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\instances.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\interactive_base.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\IntPlayer.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\main.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\map.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\mapview.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\militarysite.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\minimap.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\options.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\player.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\playerdescrgroup.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\productionsite.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\profile.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\s2map.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\setup.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\sw16_graphic.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\sw16_render.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\sw16_terrain.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\sw32_graphic.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\sw32_render.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\sw32_terrain.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\system.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\transport.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\tribe.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\ware.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\warehouse.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\waresdisplay.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\watchwindow.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\worker.cc
cl /c /nologo /Ox /TP /EHsc %myincludes% %defines% src\world.cc


REM ... from SDL
cl /c /nologo /Ox /TP /DUSE_MESSAGEBOX /EHsc %myincludes% %defines% SDL_win32_main.c
