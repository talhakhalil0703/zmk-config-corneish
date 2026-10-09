if(CONFIG_BOARD_CORNEISH_ZEN_V2_LEFT AND CONFIG_ZMK_DISPLAY
   AND CONFIG_CUSTOM_WIDGET_OUTPUT_STATUS)
  set(corneish_output_source "${BOARD_DIR}/widgets/output_status.c")
  get_target_property(corneish_app_sources app SOURCES)
  list(FIND corneish_app_sources "${corneish_output_source}" corneish_output_index)

  if(corneish_output_index EQUAL -1)
    message(FATAL_ERROR "Cannot find the Corne-ish Zen output widget to replace")
  endif()

  list(REMOVE_ITEM corneish_app_sources "${corneish_output_source}")
  set_property(TARGET app PROPERTY SOURCES "${corneish_app_sources}")
  target_sources(app PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config/widgets/output_status.c")
  message(STATUS
    "Using custom output status: ${CMAKE_CURRENT_SOURCE_DIR}/config/widgets/output_status.c")
endif()
