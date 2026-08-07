add_executable(caffluxweighter caffluxweighter.cxx)
target_link_libraries(caffluxweighter PUBLIC duneanafluxtools::all duneanaobj::StandardRecord ROOT::Tree)

install(TARGETS caffluxweighter DESTINATION bin)
