# GNUInstallDirs requires CMake >= 3.0.2
#include(GNUInstallDirs)

set(XDG_APPLICATION_ID "org.widelands.Widelands")

list(APPEND icon_sizes "16" "32" "48" "64" "128")
foreach (icon_size ${icon_sizes})
  #install(FILES ${CMAKE_SOURCE_DIR}/data/images/logos/wl-ico-${icon_size}.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/${icon_size}x${icon_size}/apps RENAME ${XDG_APPLICATION_ID}.png)
  install(FILES ${CMAKE_SOURCE_DIR}/data/images/logos/wl-ico-${icon_size}.png DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/${icon_size}x${icon_size}/apps RENAME ${XDG_APPLICATION_ID}.png)
endforeach (icon_size ${icon_sizes})
#install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}.6           DESTINATION ${CMAKE_INSTALL_MANDIR}/man6)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}.6           DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man6)
#install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${XDG_APPLICATION_ID}.desktop     DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${XDG_APPLICATION_ID}.desktop     DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
#install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${XDG_APPLICATION_ID}.appdata.xml DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${XDG_APPLICATION_ID}.appdata.xml DESTINATION ${CMAKE_INSTALL_PREFIX}/share/metainfo)

# TODO: check for privileges
# If 'make install' was run without superuser privileges, the user will get a "permission denied" error
# when 'gtk-update-icon-cache' is executed
find_program(GTK_UPDATE_ICON_CACHE NAMES gtk-update-icon-cache)
if (GTK_UPDATE_ICON_CACHE)
  #install(CODE "execute_process (COMMAND ${GTK_UPDATE_ICON_CACHE} -t -f ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor ||: )")
  install(CODE "execute_process (COMMAND ${GTK_UPDATE_ICON_CACHE} -t -f ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor ||: )")
else (GTK_UPDATE_ICON_CACHE)
  message(WARNING "gtk-update-icon-cache not found!")
endif (GTK_UPDATE_ICON_CACHE)
