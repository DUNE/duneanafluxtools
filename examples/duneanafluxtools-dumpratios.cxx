#include "duneanaobj/StandardRecord/SRGlobal.h"
#include "duneanaobj/StandardRecord/StandardRecord.h"

#include "TFile.h"
#include "TTree.h"

#include <functional>
#include <numeric>
#include <string>
#include <vector>

struct FluxDialDetails {
  int idx;
  float param_val0, param_val1;
};

std::vector<FluxDialDetails> GetFluxDialDetails(caf::SRGlobal const &gl) {

  auto get_param_index = [&](std::string const &pname) -> int {
    for (size_t i = 0; i < gl.wgts.params.size(); ++i) {
      if (gl.wgts.params[i].name == pname) {
        return i;
      }
    }
    return -1;
  };

  std::vector<FluxDialDetails> dialdetails;
  for (auto const &pname : std::vector<std::string>{

       }) {
    int idx = get_param_index(pname);
    dialdetails.push_back(FluxDialDetails{idx, gl.wgts.params[idx].vals[0],
                                          gl.wgts.params[idx].vals[1]});
  }
  return dialdetails;
}

std::vector<float>
GetDialWeights(caf::SRTrueInteraction const &nu,
               std::vector<FluxDialDetails> const &flux_dial_details,
               std::vector<float> const &param_values) {

  std::vector<float> wghts(1, flux_dial_details.size());
  for (size_t i = 0; i < flux_dial_details.size(); ++i) {
    auto const &fdd = flux_dial_details[i];
    auto const &ev_wghts = nu.syst_dials[fdd.idx].weights;
    // linear extrapolation from 2 known knot points
    wghts[i] =
        ev_wghts[0] + (param_values[i] / (fdd.param_val1 - fdd.param_val0)) *
                          (ev_wghts[1] - ev_wghts[0]);
  }
  return wghts;
}

float GetWeight(caf::SRTrueInteraction const &nu,
                std::vector<FluxDialDetails> const &flux_dial_details,
                std::vector<float> const &param_values) {
  auto const &wghts = GetDialWeights(nu, flux_dial_details, param_values);
  return std::reduce(wghts.begin(), wghts.end(), 1.0, std::multiplies<float>());
}

int main(int argc, char const *argv[]) {

  if (argc != 2) {
    bool ask_help = (argc > 1) && ((argv[1] == "-h") || (argv[1] == "--help") ||
                                   (argv[1] == "-?"));
    std::cout << "RUNLIKE: " << argv[0] << " input_caf.root" << std::endl;
    return !ask_help;
  }

  TFile *fin = TFile::Open(argv[1], "READ");
  if (!fin) {
    std::cout << "[ERROR]: Failed to open " << argv[1] << " for reading."
              << std::endl;
    return 1;
  }

  auto caf = fin->Get<TTree>("cafTree");
  if (!caf) {
    caf = fin->Get<TTree>("cafmaker/cafTree");
  }
  if (!caf) {
    std::cout << "[ERROR]: Failed to read cafTree or cafmaker/cafTree from "
              << argv[1] << "." << std::endl;

    return 1;
  }

  caf::StandardRecord *SR = nullptr;
  caf->SetBranchAddress("rec", &SR);
  Long64_t ents = caf->GetEntries();
  std::cout << "Input cafTree has " << ents << " entries." << std::endl;

  caf::SRGlobal *gl = nullptr;
  auto global = fin->Get<TTree>("globalTree");
  if (!global) {
    global = fin->Get<TTree>("cafmaker/globalTree");
  }
  if (!global) {
    std::cout
        << "[ERROR]: Failed to read globalTree or cafmaker/globalTree from "
        << argv[1] << "." << std::endl;

    return 1;
  }
  global->SetBranchAddress("global", &gl);
  global->GetEntry(0);

  auto flux_dial_details = GetFluxDialDetails(*gl);

  for (Long64_t i = 0; i < ents; ++i) {
    caf->GetEntry(i);

    for (auto &nu : SR->mc.nu) {
      // nu.E
      for (float var :
           std::vector<float>{-3, -2, -0.5, -0.0001, 0.75, 1.75, 2.75}) {
        auto wv =
            GetDialWeights(nu, flux_dial_details,
                           std::vector<float>(flux_dial_details.size(), var));
      }
    }
  }
}
