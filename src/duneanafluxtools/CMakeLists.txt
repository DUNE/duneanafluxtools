add_library(duneanafluxtools_fluxweighter SHARED FluxWeighter.cxx)
target_link_libraries(duneanafluxtools_fluxweighter PUBLIC ROOT::Hist)

set_target_properties(duneanafluxtools_fluxweighter PROPERTIES
    PUBLIC_HEADER "FluxWeighter.h;CAFInterface.h"
    EXPORT_NAME FluxWeighter)

target_include_directories(duneanafluxtools_fluxweighter PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/..>
  $<INSTALL_INTERFACE:include>)

install(TARGETS duneanafluxtools_fluxweighter
        EXPORT duneanafluxtools-targets
        LIBRARY DESTINATION lib/
        PUBLIC_HEADER DESTINATION include/duneanafluxtools)

add_library(duneanafluxtools::fluxweighter ALIAS duneanafluxtools_fluxweighter)
