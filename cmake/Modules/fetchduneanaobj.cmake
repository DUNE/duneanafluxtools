macro(fetchduneanaobj DUNE_ANAOBJ_BRANCH)

  include(FetchContent)
  FetchContent_Declare(
    duneanaobj
    GIT_REPOSITORY https://github.com/DUNE/duneanaobj.git
    GIT_TAG        ${DUNE_ANAOBJ_BRANCH}
  )


  # Check if population has already been performed
  FetchContent_GetProperties(duneanaobj)
  if(NOT duneanaobj_POPULATED)
    # Fetch the content using previously declared details
    FetchContent_Populate(duneanaobj)

  endif()

  include_directories(${duneanaobj_SOURCE_DIR})

  ROOT_GENERATE_DICTIONARY(StandardRecordDict
    ${duneanaobj_SOURCE_DIR}/duneanaobj/StandardRecord/StandardRecord.h
    LINKDEF
      ${duneanaobj_SOURCE_DIR}/duneanaobj/StandardRecord/classes_def.xml)

  file(GLOB SR_IMPL_FILES "${duneanaobj_SOURCE_DIR}/duneanaobj/StandardRecord/*.cxx")
  LIST(APPEND SR_IMPL_FILES ${CMAKE_CURRENT_BINARY_DIR}/StandardRecordDict.cxx)

  add_library(duneanaobj_StandardRecord SHARED ${SR_IMPL_FILES})
  target_link_libraries(duneanaobj_StandardRecord PUBLIC ROOT::MathCore ROOT::Physics)

  target_include_directories(duneanaobj_StandardRecord PUBLIC
    $<BUILD_INTERFACE:${duneanaobj_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>)
  target_include_directories(duneanaobj_StandardRecord PRIVATE
    $<BUILD_INTERFACE:${duneanaobj_SOURCE_DIR}/duneanaobj/StandardRecord> #root puts this in the dictionary
    )

  set_target_properties(duneanaobj_StandardRecord PROPERTIES EXPORT_NAME all)
  install(TARGETS duneanaobj_StandardRecord EXPORT duneanafluxtools-targets DESTINATION lib)

  file(GLOB SR_HEADER_FILES "${duneanaobj_SOURCE_DIR}/duneanaobj/StandardRecord/*.h")

  install(FILES ${SR_HEADER_FILES} DESTINATION include/duneanaobj/StandardRecord)
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/libStandardRecordDict_rdict.pcm
    ${CMAKE_CURRENT_BINARY_DIR}/libStandardRecordDict.rootmap
    DESTINATION lib)

  add_library(duneanaobj::all ALIAS duneanaobj_StandardRecord)

endmacro()
