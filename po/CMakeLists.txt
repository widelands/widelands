if (${CMAKE_BUILD_TYPE} STREQUAL Release)
  add_custom_target (
    lang ALL
  )
else (${CMAKE_BUILD_TYPE} STREQUAL Release)
  add_custom_target (
    lang_dummy ALL
    COMMAND cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/locale
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )
  add_custom_target (
    lang
  )
endif (${CMAKE_BUILD_TYPE} STREQUAL Release)

macro(CreateLocale locale_name)
  set (commands "")
  file (GLOB_RECURSE pofiles *.po)
  foreach (pofile ${pofiles})
    get_filename_component(pofile_name ${pofile} NAME_WE)
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" pofile_subpath ${pofile})
    get_filename_component(pofile_path ${pofile} PATH)
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" template_name ${pofile_path})
    if (${pofile_name} STREQUAL ${locale_name})
      add_custom_command (
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../locale/${locale_name}/LC_MESSAGES/${template_name}.mo
	COMMAND cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/../locale/${locale_name}/LC_MESSAGES
	COMMAND msgmerge -q --no-wrap ${pofile} ${CMAKE_CURRENT_SOURCE_DIR}/${template_name}/${template_name}.pot -o ${CMAKE_CURRENT_BINARY_DIR}/${locale_name}_${template_name}.tmp.po
	COMMAND msgfmt -o ${CMAKE_CURRENT_BINARY_DIR}/../locale/${locale_name}/LC_MESSAGES/${template_name}.mo ${CMAKE_CURRENT_BINARY_DIR}/${locale_name}_${template_name}.tmp.po
	COMMAND cmake -E remove ${CMAKE_CURRENT_BINARY_DIR}/${locale_name}_${template_name}.tmp.po
        DEPENDS ${pofile}
        COMMENT "Generating translations for locale ${locale_name} category ${template_name}"
      )
      list(APPEND commands ${CMAKE_CURRENT_BINARY_DIR}/../locale/${locale_name}/LC_MESSAGES/${template_name}.mo)
    endif (${pofile_name} STREQUAL ${locale_name})
  endforeach (pofile ${pofiles})
  add_custom_target (
    lang_${locale_name}
    DEPENDS ${commands}
    COMMENT "Generating translations for locale ${locale_name}"
  )
  add_custom_target (
    checklang_${locale_name}
    COMMAND cmake -Dlang=${locale_name} -Dpobasedir=${CMAKE_CURRENT_SOURCE_DIR} -Dlocalebasedir=${CMAKE_CURRENT_BINARY_DIR}/../locale -P ${CMAKE_CURRENT_SOURCE_DIR}/CheckLocale.cmake
    COMMENT "Checking translations for locale ${locale_name}"
#    WORKING_DIRECTORY=${CMAKE_CURRENT_SOURCE_DIR}
  )
  add_dependencies(lang checklang_${locale_name})
  add_dependencies(checklang_${locale_name} lang_${locale_name})
endmacro(CreateLocale locale_name)

set (languages_found "")
file (GLOB_RECURSE pofiles *.po)
foreach(pofile ${pofiles})
  get_filename_component(pofile ${pofile} NAME_WE)
  list(APPEND languages_found ${pofile})
endforeach(pofile ${pofiles})

list(SORT languages_found)
list(REMOVE_DUPLICATES languages_found)

foreach(language ${languages_found})
  CreateLocale(${language})
endforeach(language ${languages_found})
