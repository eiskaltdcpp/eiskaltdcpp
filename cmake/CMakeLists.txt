# ---------------------------------------------------------------------------
# GET_ABSOLUTE_SOURCES
# Given a list of sources, return the corresponding absolute paths
# 'varname': name of the var the list of absolute paths should be stored into
# 'sources': list of source files

macro(GET_ABSOLUTE_SOURCES varname sources)

  set(${varname})
  foreach(file ${sources})
    get_filename_component(abs_file ${file} ABSOLUTE)
    if(NOT EXISTS ${abs_file})
      set(abs_file "${PROJECT_SOURCE_DIR}/${file}")
    endif(NOT EXISTS ${abs_file})
    set(${varname} ${${varname}} ${abs_file})
  endforeach(file)

endmacro(GET_ABSOLUTE_SOURCES)

# ---------------------------------------------------------------------------
# GET_RELATIVE_SOURCES
# Given a list of sources, return the corresponding relative paths to
# a directory.
# 'varname': name of the var the list of absolute paths should be stored into
# 'dir': path to the dir we want relative path from
# 'sources': list of *absolute* path to the source files

macro(GET_RELATIVE_SOURCES varname dir sources)

  get_filename_component(dir_abs ${dir} ABSOLUTE)

  set(${varname})
  foreach(file ${sources})
    file(RELATIVE_PATH rel_file "${dir}" "${file}")
    set(${varname} ${${varname}} ${rel_file})
  endforeach(file)

endmacro(GET_RELATIVE_SOURCES)

get_absolute_sources(abs_dcpp_sources "${DCPP_SRCS};${DCPP_HDRS}")
get_relative_sources(rel_dcpp_sources "${dcpp_SOURCE_DIR}/po" "${abs_dcpp_sources}")
file(MAKE_DIRECTORY "${dcpp_BINARY_DIR}/po")
set(dcpp_files_from "${dcpp_BINARY_DIR}/po/libeiskaltdcpp_srcs.txt")
string(REGEX REPLACE ";" "\n" dcpp_contents "${rel_dcpp_sources}")
file(WRITE "${dcpp_files_from}" "${dcpp_contents}")

if (USE_GTK OR USE_GTK3)
get_absolute_sources(abs_gtk_cpp_sources "${GTK_SRCS};${GTK_HDRS}")
get_relative_sources(rel_gtk_cpp_sources "${eiskaltdcpp-gtk_SOURCE_DIR}/po" "${abs_gtk_cpp_sources}")
file(MAKE_DIRECTORY "${eiskaltdcpp-gtk_BINARY_DIR}/po")
set(gtk_cpp_files_from "${eiskaltdcpp-gtk_BINARY_DIR}/po/eiskaltdcpp-gtk_cpp_srcs.txt")
string(REGEX REPLACE ";" "\n" gtk_cpp_contents "${rel_gtk_cpp_sources}")
file(WRITE "${gtk_cpp_files_from}" "${gtk_cpp_contents}")

get_absolute_sources(abs_gtk_ui_sources "${GTK_UI}")
get_relative_sources(rel_gtk_ui_sources "${eiskaltdcpp-gtk_SOURCE_DIR}/po" "${abs_gtk_ui_sources}")
set(gtk_ui_files_from "${eiskaltdcpp-gtk_BINARY_DIR}/po/eiskaltdcpp-gtk_ui_srcs.txt")
string(REGEX REPLACE ";" "\n" gtk_ui_contents "${rel_gtk_ui_sources}")
file(WRITE "${gtk_ui_files_from}" "${gtk_ui_contents}")
endif (USE_GTK OR USE_GTK3)
############ pot update. ###########

if(UPDATE_PO)
		# Generate pot file for c++ data.
		add_custom_command(
			OUTPUT ${dcpp_SOURCE_DIR}/po/libeiskaltdcpp.pot
			COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE}
				--add-comments=TRANSLATORS
				--files-from="${dcpp_files_from}"
				--copyright-holder=\"EiskaltDC++ team\"
				--msgid-bugs-address=\"http://code.google.com/p/eiskaltdc/issues/list\"
				--from-code=UTF-8
				--keyword=_
				--keyword=N_
				--keyword=F_
				--keyword=F_:2 --flag=F_:2:pass-c-format
				--keyword=FN_
				--output=${dcpp_SOURCE_DIR}/po/libeiskaltdcpp.pot
			# replace the chartype
			COMMAND sed -i
				s/charset=CHARSET/charset=UTF-8/
				${dcpp_SOURCE_DIR}/po/libeiskaltdcpp.pot
			DEPENDS "${dcpp_files_from}"
			WORKING_DIRECTORY ${dcpp_SOURCE_DIR}/po
			COMMENT "pot-update [libeiskaltdcpp]: Generated source pot file."
		)
		if (USE_GTK OR USE_GTK3)
		add_custom_command(
			OUTPUT ${eiskaltdcpp-gtk_SOURCE_DIR}/po/eiskaltdcpp-gtk.cpp.pot
			COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE}
				--add-comments=TRANSLATORS
				--files-from="${gtk_cpp_files_from}"
				--copyright-holder=\"EiskaltDC++ team\"
				--msgid-bugs-address=\"http://code.google.com/p/eiskaltdc/issues/list\"
				--from-code=UTF-8
				--keyword=_
				--keyword=N_
				--keyword=F_
				--keyword=F_:2 --flag=F_:2:pass-c-format
				--keyword=FN_
				--output="${eiskaltdcpp-gtk_SOURCE_DIR}/po/eiskaltdcpp-gtk.cpp.pot"
			# replace the chartype
			COMMAND sed -i
				s/charset=CHARSET/charset=UTF-8/
				${eiskaltdcpp-gtk_SOURCE_DIR}/po/eiskaltdcpp-gtk.cpp.pot
			DEPENDS "${gtk_cpp_files_from}"
			WORKING_DIRECTORY ${eiskaltdcpp-gtk_SOURCE_DIR}/po
			COMMENT "pot-update [eiskaltdcpp-gtk.cpp]: Generated source pot file."
		)

		# Generate pot file for ui files.
		    add_custom_command(
			OUTPUT ${eiskaltdcpp-gtk_SOURCE_DIR}/po/eiskaltdcpp-gtk.ui.pot
			COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE}
				--add-comments=TRANSLATORS
				--files-from="${gtk_ui_files_from}"
				--copyright-holder=\"EiskaltDC++ team\"
				--msgid-bugs-address=\"http://code.google.com/p/eiskaltdc/issues/list\"
				--from-code=UTF-8
				--sort-output
				--language=Glade
				--keyword=translatable="yes"
				--output="${eiskaltdcpp-gtk_SOURCE_DIR}/po/eiskaltdcpp-gtk.ui.pot"
			# replace the chartype
			COMMAND sed -i
				s/charset=CHARSET/charset=UTF-8/
				${eiskaltdcpp-gtk_SOURCE_DIR}/po/eiskaltdcpp-gtk.ui.pot
			DEPENDS "${gtk_ui_files_from}"
			WORKING_DIRECTORY ${eiskaltdcpp-gtk_SOURCE_DIR}/po
			COMMENT "pot-update [eiskaltdcpp-gtk.ui]: Generated source pot file."
		    )

		# Merge both pot files
		add_custom_command(
			# The old function checked for differences in the time in the header see
			# what we need to do with it.
			OUTPUT ${eiskaltdcpp-gtk_SOURCE_DIR}/po/eiskaltdcpp-gtk.pot

			COMMAND ${GETTEXT_MSGCAT_EXECUTABLE}
				--sort-output
					${eiskaltdcpp-gtk_SOURCE_DIR}/po/eiskaltdcpp-gtk.cpp.pot
					${eiskaltdcpp-gtk_SOURCE_DIR}/po/eiskaltdcpp-gtk.ui.pot
				--output ${eiskaltdcpp-gtk_SOURCE_DIR}/po/eiskaltdcpp-gtk.pot

			COMMAND ${CMAKE_COMMAND} -E remove
					${eiskaltdcpp-gtk_SOURCE_DIR}/po/eiskaltdcpp-gtk.cpp.pot
					${eiskaltdcpp-gtk_SOURCE_DIR}/po/eiskaltdcpp-gtk.ui.pot

			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
			DEPENDS
					${eiskaltdcpp-gtk_SOURCE_DIR}/po/eiskaltdcpp-gtk.cpp.pot
					${eiskaltdcpp-gtk_SOURCE_DIR}/po/eiskaltdcpp-gtk.ui.pot
			COMMENT "pot-update [eiskaltdcpp-gtk]: Generated pot file."
		)
		endif (USE_GTK OR USE_GTK3)
		# Update / generate the po files for all languages
		foreach(LANG ${LANGUAGES})

			### Generate new po file.

			# For some reason CMake is rather happy to delete the po file in
			# some cases. Too avoid that problem only generate the init rule
			# if the po file doesn't exist. The case where a po file used to
			# exist and no longer exists should never occur

			if(NOT EXISTS ${dcpp_SOURCE_DIR}/po/${LANG}.po)
				add_custom_command(
					OUTPUT ${dcpp_SOURCE_DIR}/po/${LANG}.po
					COMMAND ${GETTEXT_MSGINIT_EXECUTABLE}
							--no-translator
							--input=libeiskaltdcpp.pot
							--output-file=${LANG}.po
							--locale=${LANG}
					WORKING_DIRECTORY ${dcpp_SOURCE_DIR}/po
					DEPENDS
						${dcpp_SOURCE_DIR}/po/libeiskaltdcpp.pot
					COMMENT
						"pot-update [libeiskaltdcpp-${LANG}]: Initialized po file."
				)
			endif(NOT EXISTS ${dcpp_SOURCE_DIR}/po/${LANG}.po)

			add_custom_command(
				# The pot-update depends on a dummy file which we promise to
				# make. We don't make this dummy file but the pot-update
				# still works.
				OUTPUT ${dcpp_SOURCE_DIR}/po/${LANG}.po.dummy
				COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE}
						--backup=none
						-U ${LANG}.po
						${dcpp_SOURCE_DIR}/po/libeiskaltdcpp.pot
				DEPENDS
					${dcpp_SOURCE_DIR}/po/libeiskaltdcpp.pot
					${dcpp_SOURCE_DIR}/po/${LANG}.po
				WORKING_DIRECTORY ${dcpp_SOURCE_DIR}/po
				COMMENT "pot-update [dcpp-${LANG}]: Updated po file."
			)

			if (USE_GTK OR USE_GTK3)
			if(NOT EXISTS ${eiskaltdcpp-gtk_SOURCE_DIR}/po/${LANG}.po)
				add_custom_command(
					OUTPUT ${eiskaltdcpp-gtk_SOURCE_DIR}/po/${LANG}.po
					COMMAND ${GETTEXT_MSGINIT_EXECUTABLE}
							--no-translator
							--input=eiskaltdcpp-gtk.pot
							--output-file=${LANG}.po
							--locale=${LANG}
					WORKING_DIRECTORY ${eiskaltdcpp-gtk_SOURCE_DIR}/po
					DEPENDS
						${eiskaltdcpp-gtk_SOURCE_DIR}/po/eiskaltdcpp-gtk.pot
					COMMENT
						"pot-update [eiskaltdcpp-gtk-${LANG}]: Initialized po file."
				)
			endif(NOT EXISTS ${eiskaltdcpp-gtk_SOURCE_DIR}/po/${LANG}.po)

			### Update existing po file.
			add_custom_command(
				# The pot-update depends on a dummy file which we promise to
				# make. We don't make this dummy file but the pot-update
				# still works.
				OUTPUT ${eiskaltdcpp-gtk_SOURCE_DIR}/po/${LANG}.po.dummy
				COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE}
						--backup=none
						-U ${LANG}.po
						${eiskaltdcpp-gtk_SOURCE_DIR}/po/eiskaltdcpp-gtk.pot
				DEPENDS
					${eiskaltdcpp-gtk_SOURCE_DIR}/po/eiskaltdcpp-gtk.pot
					${eiskaltdcpp-gtk_SOURCE_DIR}/po/${LANG}.po
				WORKING_DIRECTORY ${eiskaltdcpp-gtk_SOURCE_DIR}/po
				COMMENT "pot-update [eiskaltdcpp-gtk-${LANG}]: Updated po file."
			)
			SET(pot-update-SRC ${pot-update-SRC} ${eiskaltdcpp-gtk_SOURCE_DIR}/po/${LANG}.po.dummy )
			endif (USE_GTK OR USE_GTK3)

			SET(pot-update-SRC ${pot-update-SRC} ${dcpp_SOURCE_DIR}/po/${LANG}.po.dummy)
		endforeach(LANG ${LANGUAGES})

		# Add to target list
		SET(pot-update-SRC
			${pot-update-SRC}
			${dcpp_SOURCE_DIR}/po/libeiskaltdcpp.pot # should depend on languages

		)
		if (USE_GTK OR USE_GTK3)
		SET(pot-update-SRC
			${pot-update-SRC}
			${eiskaltdcpp-gtk_SOURCE_DIR}/po/eiskaltdcpp-gtk.pot
		)
		endif (USE_GTK OR USE_GTK3)

	# The target to call
	add_custom_target(pot-update
		COMMENT "pot-update: Done."
		DEPENDS ${pot-update-SRC}
	)

	##### po update per language #####
	foreach(LANG ${LANGUAGES})
			add_custom_command(
				# The pot-update depends on a dummy file which we promise to
				# make. We don't make this dummy file but the pot-update
				# still works.
				OUTPUT ${dcpp_SOURCE_DIR}/po/${LANG}.po.lang.dummy
				# After the po file is updated it might look not entirely as
				# wanted, for example poedit reorders the file. Use msgattrib
				# to reformat the file, use a helper file to do so.
				COMMAND ${GETTEXT_MSGATTRIB_EXECUTABLE}
						--output ${LANG}.po.tmp
						${LANG}.po
				COMMAND ${CMAKE_COMMAND} -E copy
						${LANG}.po.tmp
						${LANG}.po
				COMMAND ${CMAKE_COMMAND} -E remove
						${LANG}.po.tmp
				# Now merge with the pot file.
				COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE}
						--backup=none
						-U ${LANG}.po
						libeiskaltdcpp.pot
				WORKING_DIRECTORY ${dcpp_SOURCE_DIR}/po
				COMMENT "po-update [libeiskaltdcpp-${LANG}]: Updated po file."
			)
			if (USE_GTK OR USE_GTK3)
			add_custom_command(
				# The pot-update depends on a dummy file which we promise to
				# make. We don't make this dummy file but the pot-update
				# still works.
				OUTPUT ${eiskaltdcpp-gtk_SOURCE_DIR}/po/${LANG}.po.lang.dummy
				# After the po file is updated it might look not entirely as
				# wanted, for example poedit reorders the file. Use msgattrib
				# to reformat the file, use a helper file to do so.
				COMMAND ${GETTEXT_MSGATTRIB_EXECUTABLE}
						--output ${LANG}.po.tmp
						${LANG}.po
				COMMAND ${CMAKE_COMMAND} -E copy
						${LANG}.po.tmp
						${LANG}.po
				COMMAND ${CMAKE_COMMAND} -E remove
						${LANG}.po.tmp
				# Now merge with the pot file.
				COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE}
						--backup=none
						-U ${LANG}.po
						eiskaltdcpp-gtk.pot
				WORKING_DIRECTORY ${eiskaltdcpp-gtk_SOURCE_DIR}/po
				COMMENT "po-update [eiskaltdcpp-gtk-${LANG}]: Updated po file."
			)
			set(po-update-LANG-SRC ${po-update-LANG-SRC}
				${eiskaltdcpp-gtk_SOURCE_DIR}/po/${LANG}.po.lang.dummy
			)
			endif (USE_GTK OR USE_GTK3)
			set(po-update-LANG-SRC ${po-update-LANG-SRC}
				${dcpp_SOURCE_DIR}/po/${LANG}.po.lang.dummy
			)

		add_custom_target(po-update-${LANG}
			COMMENT "po-update ${LANG}: Done."
			DEPENDS ${po-update-LANG-SRC}
		)
	endforeach(LANG ${LANGUAGES})

endif(UPDATE_PO)

if(CREATE_MO)

	if(UPDATE_PO)
		# Only set the verbose flag for maintainers.
		set(GETTEXT_MSGFMT_PARAMETER -v)
	endif(UPDATE_PO)

	foreach(LANG ${LANGUAGES})

			add_custom_command(
				OUTPUT ${dcpp_BINARY_DIR}/locale/${LANG}/LC_MESSAGES
				COMMAND ${CMAKE_COMMAND} -E make_directory
						${dcpp_BINARY_DIR}/locale/${LANG}/LC_MESSAGES
				COMMENT "mo-update [${LANG}]: Creating locale directory."
			)
			add_custom_command(
				OUTPUT ${dcpp_BINARY_DIR}/locale/${LANG}/LC_MESSAGES/libeiskaltdcpp.mo
				COMMAND ${GETTEXT_MSGFMT_EXECUTABLE}
						${GETTEXT_MSGFMT_PARAMETER}
						-o ${dcpp_BINARY_DIR}/locale/${LANG}/LC_MESSAGES/libeiskaltdcpp.mo ${LANG}.po
				DEPENDS
					${dcpp_BINARY_DIR}/locale/${LANG}/LC_MESSAGES
					${dcpp_SOURCE_DIR}/po/${LANG}.po
				WORKING_DIRECTORY ${dcpp_SOURCE_DIR}/po
				COMMENT "mo-update [dcpp-${LANG}]: Creating mo file."
			)
			if (USE_GTK OR USE_GTK3)
			add_custom_command(
				OUTPUT ${eiskaltdcpp-gtk_BINARY_DIR}/locale/${LANG}/LC_MESSAGES
				COMMAND ${CMAKE_COMMAND} -E make_directory
						${eiskaltdcpp-gtk_BINARY_DIR}/locale/${LANG}/LC_MESSAGES
				COMMENT "mo-update [${LANG}]: Creating locale directory."
			)

			add_custom_command(
				OUTPUT ${eiskaltdcpp-gtk_BINARY_DIR}/locale/${LANG}/LC_MESSAGES/eiskaltdcpp-gtk.mo
				COMMAND ${GETTEXT_MSGFMT_EXECUTABLE}
						${GETTEXT_MSGFMT_PARAMETER}
						-o ${eiskaltdcpp-gtk_BINARY_DIR}/locale/${LANG}/LC_MESSAGES/eiskaltdcpp-gtk.mo ${LANG}.po
				DEPENDS
					${eiskaltdcpp-gtk_BINARY_DIR}/locale/${LANG}/LC_MESSAGES
					${eiskaltdcpp-gtk_SOURCE_DIR}/po/${LANG}.po
				WORKING_DIRECTORY ${eiskaltdcpp-gtk_SOURCE_DIR}/po
				COMMENT "mo-update [eiskaltdcpp-gtk-${LANG}]: Creating mo file."
			)

			set(mo-update-SRC ${mo-update-SRC}
			    ${eiskaltdcpp-gtk_BINARY_DIR}/locale/${LANG}/LC_MESSAGES/eiskaltdcpp-gtk.mo
			)
			set(mo-update-LANG-SRC ${mo-update-LANG-SRC}
			    ${eiskaltdcpp-gtk_BINARY_DIR}/locale/${LANG}/LC_MESSAGES/eiskaltdcpp-gtk.mo
			)
			endif (USE_GTK OR USE_GTK3)

			set(mo-update-SRC ${mo-update-SRC}
			    ${dcpp_BINARY_DIR}/locale/${LANG}/LC_MESSAGES/libeiskaltdcpp.mo
			)
			set(mo-update-LANG-SRC ${mo-update-LANG-SRC}
			    ${dcpp_BINARY_DIR}/locale/${LANG}/LC_MESSAGES/libeiskaltdcpp.mo
			)

		if(UPDATE_PO)
			add_custom_target(mo-update-${LANG}
				COMMENT "mo-update ${LANG}: Done."
				DEPENDS ${mo-update-LANG-SRC}
			)
			set(mo-update-LANG-SRC "")
		endif(UPDATE_PO)

	endforeach(LANG ${LANGUAGES})

	# The target to call
	add_custom_target(mo-update ALL
		COMMENT "mo-update: Done."
		DEPENDS ${mo-update-SRC}
	)

	#set_directory_properties(properties
		#ADDITIONAL_MAKE_CLEAN_FILES ${dcpp_BINARY_DIR}/locale
		#${eiskaltdcpp-gtk_BINARY_DIR}/locale
	#)

endif(CREATE_MO)
