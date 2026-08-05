include(fetchduneanaobj)

if(NOT DEFINED DUNE_ANAOBJ_BRANCH OR "${DUNE_ANAOBJ_BRANCH}x" STREQUAL "x")
  set(DUNE_ANAOBJ_BRANCH v04_00_00)
endif()
fetchduneanaobj(${DUNE_ANAOBJ_BRANCH})

add_executable(caffluxweighter caffluxweighter.cxx)
target_link_libraries(caffluxweighter PUBLIC duneanafluxtools::all duneanaobj::all ROOT::Tree)

install(TARGETS caffluxweighter DESTINATION bin)
