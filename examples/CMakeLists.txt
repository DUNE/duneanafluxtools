add_executable(duneanafluxtools-dumpratios duneanafluxtools-dumpratios.cxx)
target_link_libraries(duneanafluxtools-dumpratios PUBLIC duneanafluxtools::all duneanaobj::all ROOT::Gpad ROOT::Tree)

install(TARGETS duneanafluxtools-dumpratios DESTINATION bin)
