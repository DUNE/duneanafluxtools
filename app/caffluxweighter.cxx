#include "duneanafluxtools/CAFInterface.h"
#include "duneanafluxtools/FluxWeighter.h"

#include "duneanaobj/StandardRecord/SRGlobal.h"
#include "duneanaobj/StandardRecord/StandardRecord.h"

#include "TFile.h"
#include "TTree.h"

// #define DAFT_VERBOSE

int main(int argc, char const *argv[]) {

  if (argc != 3) {
    bool ask_help = (argc > 1) && ((argv[1] == "-h") || (argv[1] == "--help") ||
                                   (argv[1] == "-?"));
    std::cout << "RUNLIKE: " << argv[0] << " input_caf.root output_caf.root"
              << std::endl;
    return !ask_help;
  }

  TFile *fin = TFile::Open(argv[1], "READ");
  if (!fin) {
    std::cout << "[ERROR]: Failed to open " << argv[1] << " for reading."
              << std::endl;
    return 1;
  }

  bool cafmakerdir = false;
  auto caf = fin->Get<TTree>("cafTree");
  if (!caf) {
    caf = fin->Get<TTree>("cafmaker/cafTree");
    cafmakerdir = true;
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

  TFile *fout = TFile::Open(argv[2], "RECREATE");
  if (cafmakerdir) {
    fout->mkdir("cafmaker");
  }
  TTree *cafout = new TTree("cafTree", "");
  cafout->Branch("rec", &SR);

  TTree *globalout = nullptr;
  caf::SRGlobal *gl = nullptr;
  auto global = fin->Get<TTree>("globalTree");
  if (global) {
    global->SetBranchAddress("global", &gl);
    global->GetEntry(0);
    gl->UpdateVersionInformation();
  } else {
    gl = new caf::SRGlobal();
  }
  globalout = new TTree("globalTree", "");
  globalout->Branch("global", &gl);

  auto &fw = FluxWeighter::Get();

  auto get_param_index = [&](std::string const &pname) -> int {
    for (size_t i = 0; i < gl->wgts.flux_params.size(); ++i) {
      if (gl->wgts.flux_params[i].name == pname) {
        return i;
      }
    }
    return -1;
  };

  std::vector<int> focussing_param_indexes, hadprod_param_indexes;
  int max_idx = 0;
  for (size_t fpi = 0; fpi < fw.GetNFocussingParams(); fpi++) {
    auto nm = fw.GetFocussingParamName(fpi);
    auto pidx = get_param_index(nm);
    if (pidx == -1) {
      caf::SRSystParamHeader ph;
      ph.vals = std::vector<float>{0, 1};
      ph.name = nm;
      ph.id = 3000 + fpi;
      focussing_param_indexes.push_back(gl->wgts.flux_params.size());
      gl->wgts.flux_params.push_back(ph);
    } else {
      focussing_param_indexes.push_back(pidx);
    }
    max_idx = std::max(max_idx, focussing_param_indexes.back());
  }

  for (size_t hppi = 0; hppi < fw.GetNHadProdPCAComponents(); hppi++) {
    auto nm =
        std::string("FluxHadronProductionPCAComponent_") + std::to_string(hppi);
    auto pidx = get_param_index(nm);
    if (pidx == -1) {
      caf::SRSystParamHeader ph;
      ph.vals = std::vector<float>{0, 1};
      ph.name = nm;
      ph.id = 3500 + hppi;
      hadprod_param_indexes.push_back(gl->wgts.flux_params.size());
      gl->wgts.flux_params.push_back(ph);
    } else {
      hadprod_param_indexes.push_back(pidx);
    }
    max_idx = std::max(max_idx, hadprod_param_indexes.back());
  }

  globalout->Fill();

  for (Long64_t i = 0; i < ents; ++i) {
    caf->GetEntry(i);

    bool is_neutrino_mode = (SR->beam.hornI > 0);

#ifdef DAFT_VERBOSE
    std::cout << "CAF SR: #" << i << std::endl;
#endif
    int nui = 0;
    for (auto &nu : SR->mc.nu) {
      auto ratios = ana::GetFluxWeights(nu, is_neutrino_mode);

#ifdef DAFT_VERBOSE
      std::cout << "  Neutrino # " << nui++ << ", PDG: " << nu.pdgorig
                << std::endl;
      std::cout << "  Focussing ratios: [ ";
      for (auto r : ratios.first) {
        std::cout << r << ", ";
      }
#endif

#ifdef DAFT_VERBOSE
      std::cout << " ]\n  HadronProduction ratios: [ ";
      for (auto r : ratios.second) {
        std::cout << r << ", ";
      }
      std::cout << " ]" << std::endl;
#endif

      if (nu.flux_systs.size() <= max_idx) {
        nu.flux_systs.resize(max_idx + 1);
      }

      for (size_t fpi = 0; fpi < fw.GetNFocussingParams(); fpi++) {
        nu.flux_systs[focussing_param_indexes[fpi]].weights =
            std::vector<float>{1, ratios.first[fpi]};
      }

      for (size_t hppi = 0; hppi < fw.GetNHadProdPCAComponents(); hppi++) {
        nu.flux_systs[hadprod_param_indexes[hppi]].weights =
            std::vector<float>{1, ratios.second[hppi]};
      }
    }
    cafout->Fill();
  }

  fout->Write();
  fout->Close();
}
