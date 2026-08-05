#pragma once

#include "duneanafluxtools/FluxWeighter.h"

#include "duneanaobj/StandardRecord/SRTrueInteraction.h"

#include <utility>
#include <vector>

namespace ana {

std::pair<std::vector<float>, std::vector<float>>
GetFluxWeights(caf::SRTrueInteraction const &nu, bool is_neutrino_mode) {
  auto &fw = FluxWeighter::Get();

  bool isND = (nu.baseline < 1000); // m
  auto nc = fw.GetNuConfig(nu.pdgorig, isND, is_neutrino_mode);

  constexpr static auto cm2m = 1E-2;

  auto fb = fw.GetFocussingBin(nu.pdgorig, nu.E, nu.vtx.x * cm2m, nc);
  auto hpb = fw.GetHadProdBin(nu.pdgorig, nu.E, nu.vtx.x * cm2m, nc);

  std::pair<std::vector<float>, std::vector<float>> ratios;
  for (size_t fpi = 0; fpi < fw.GetNFocussingParams(); fpi++) {
    ratios.first.push_back(fw.GetFluxFocussingWeight(fpi, 1, fb, nc));
  }
  for (size_t hppi = 0; hppi < fw.GetNHadProdPCAComponents(); hppi++) {
    ratios.second.push_back(fw.GetFluxHadProdWeight(hppi, 1, hpb, nc));
  }
  return ratios;
}
} // namespace ana
