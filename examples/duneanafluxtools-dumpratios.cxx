#include "duneanaobj/StandardRecord/SRGlobal.h"
#include "duneanaobj/StandardRecord/StandardRecord.h"

#include "TCanvas.h"
#include "TColor.h"
#include "TFile.h"
#include "TH1.h"
#include "TTree.h"
#include "TStyle.h"

#include <functional>
#include <numeric>
#include <string>
#include <vector>

struct FluxDialDetails {
  std::string name;
  int idx;
  float param_val0, param_val1;
};

std::vector<std::string> dial_names = {
    "TargetUpstreamDegredation",
    "TargetTiltTransverseY",
    "TargetTiltTransverseX",
    "TargetLength",
    "TargetDisplaceTransverseY",
    "TargetDisplaceTransverseX",
    "TargetDensity",
    "ProtonBeamTransverseY",
    "ProtonBeamTransverseX",
    "ProtonBeamRadius",
    "ProtonBeamAngleY",
    "ProtonBeamAngleX",
    "HornWaterLayerThickness",
    "HornCurrent",
    "HornCTiltTransverseY",
    "HornCTiltTransverseX",
    "HornCEllipticityXInducedBField",
    "HornCEccentricityXInducedBField",
    "HornCDisplaceLongitudinalZ",
    "HornBTiltTransverseY",
    "HornBTiltTransverseX",
    "HornBEllipticityXInducedBField",
    "HornBDisplaceLongitudinalZ",
    "HornATiltTransverseY",
    "HornATiltTransverseX",
    "HornAEllipticityXInducedBField",
    "HornAEccentricityXInducedBField",
    "HornADisplaceLongitudinalZ",
    "DecayPipeTiltY",
    "DecayPipeTiltX",
    "DecayPipeRadius",
    "DecayPipeLength",
    "DecayPipeGeoBField",
    "DecayPipeEllipticalCrossSectionYB",
    "DecayPipeEllipticalCrossSectionXA",
    "DecayPipeDisplaceTransverseY",
    "DecayPipeDisplaceTransverseX",
    "DecayPipe3SegmentBowingY",
    "DecayPipe3SegmentBowingX",
    "HornCDisplaceTransverseY",
    "HornBDisplaceTransverseY",
    "HornADisplaceTransverseY",
    "HornCDisplaceTransverseX",
    "HornBDisplaceTransverseX",
    "HornADisplaceTransverseX",
    "FluxHadronProductionPCAComponent_0",
    "FluxHadronProductionPCAComponent_1",
    "FluxHadronProductionPCAComponent_2",
    "FluxHadronProductionPCAComponent_3",
    "FluxHadronProductionPCAComponent_4",
    "FluxHadronProductionPCAComponent_5",
    "FluxHadronProductionPCAComponent_6",
    "FluxHadronProductionPCAComponent_7",
    "FluxHadronProductionPCAComponent_8",
    "FluxHadronProductionPCAComponent_9",
    "FluxHadronProductionPCAComponent_10",
    "FluxHadronProductionPCAComponent_11",
    "FluxHadronProductionPCAComponent_12",
    "FluxHadronProductionPCAComponent_13",
    "FluxHadronProductionPCAComponent_14",
    "FluxHadronProductionPCAComponent_15",
    "FluxHadronProductionPCAComponent_16",
    "FluxHadronProductionPCAComponent_17",
    "FluxHadronProductionPCAComponent_18",
    "FluxHadronProductionPCAComponent_19",
    "FluxHadronProductionPCAComponent_20",
};

std::vector<FluxDialDetails> GetFluxDialDetails(caf::SRGlobal const &gl) {

  auto get_param_index = [&](std::string const &pname) -> int {
    for (size_t i = 0; i < gl.wgts.flux_params.size(); ++i) {
      if (gl.wgts.flux_params[i].name == pname) {
        return i;
      }
    }
    return -1;
  };

  std::vector<FluxDialDetails> dialdetails;
  for (auto const &pname : dial_names) {
    int idx = get_param_index(pname);
    dialdetails.push_back(FluxDialDetails{
        pname, idx, gl.wgts.flux_params[idx].vals[0], gl.wgts.flux_params[idx].vals[1]});
  }
  return dialdetails;
}

std::vector<float>
GetDialWeights(caf::SRTrueInteraction const &nu,
               std::vector<FluxDialDetails> const &flux_dial_details,
               std::vector<float> const &param_values) {
  std::vector<float> wghts(flux_dial_details.size(), 1);
  for (size_t i = 0; i < flux_dial_details.size(); ++i) {
    auto const &fdd = flux_dial_details[i];
    if (nu.flux_systs.size() <= fdd.idx) {
      throw std::runtime_error("Too few weights on SRTrueInteraction");
    }
    auto const &ev_wghts = nu.flux_systs[fdd.idx].weights;
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
  for (auto const &fdd : flux_dial_details) {
    std::cout << fdd.name << std::endl;
  }

  std::map<int, std::string> pdgnames = {
      {12, "nue"},
      {-12, "nuebar"},
      {14, "numu"},
      {-14, "numubar"},
  };

  std::vector<float> vars{-3, -2, -0.5, -0.0001, 0.75, 1.75, 2.75};

  std::vector<double> Stops{0, 0.5, 1}, Red{0, 0.5, 1}, Green{0, 0.5, 0},
      Blue{1, 0.5, 0};

  int fcolor = TColor::CreateGradientColorTable(
      3, Stops.data(), Red.data(), Green.data(), Blue.data(), vars.size());

  std::map<int, std::vector<std::vector<TH1D>>> histos;

  for (Long64_t i = 0; i < ents; ++i) {
    caf->GetEntry(i);

    for (auto &nu : SR->mc.nu) {
      if (!histos.count(nu.pdgorig)) {
        histos.emplace(nu.pdgorig, std::vector<std::vector<TH1D>>{});
      }

      for (size_t i = 0; i < vars.size(); ++i) {

        if (histos[nu.pdgorig].size() <= i) {
          histos[nu.pdgorig].emplace_back();
        }

        auto wv = GetDialWeights(
            nu, flux_dial_details,
            std::vector<float>(flux_dial_details.size(), vars[i]));

        for (size_t j = 0; j < wv.size(); ++j) {
          if (histos[nu.pdgorig][i].size() <= j) {
            histos[nu.pdgorig][i].emplace_back(
                Form("pdg%s_dial%i_var%i", pdgnames[nu.pdgorig].c_str(), i, j),
                Form("%s %s;E_{#nu} (GeV);Count", pdgnames[nu.pdgorig].c_str(),
                     dial_names[j].c_str()),
                20, 0, 10);
            histos[nu.pdgorig][i].back().SetDirectory(nullptr);
          }
          histos[nu.pdgorig][i][j].Fill(nu.E, wv[j]);
        }
      }
    }
  }

  gStyle->SetOptStat(false);

  TCanvas c1("c1", "");
  c1.Print("fluxvars.pdf[");
  for (auto &[pdgorig, hvv] : histos) {
    for (int di = 0; di < flux_dial_details.size(); ++di) {
      for (int vi = 0; vi < vars.size(); ++vi) {
        hvv[vi][di].SetLineColor(fcolor + vi);
        hvv[vi][di].Draw(vi == 0 ? "HIST" : "HISTSAME");
      }
      c1.Print("fluxvars.pdf");
    }
  }
  c1.Print("fluxvars.pdf]");
}
