# duneanafluxtools

## Build

Depends on ROOT and duneanaobj. Will build duneanaobj for you. Configure like

```bash
cmake .. -DDUNE_ANAOBJ_BRANCH=v03_06_01
```

to specify a duneanaobj version.

```bash
make install
# adds bin/lib to PATH variables
# sets duneanafluxtools_ROOT needed at runtime to find the input file
source bin/setup.duneanafluxtools.sh
```

## Usage

These tools can be used in one of 2 main configurations, described below.

### CAF Weighter App

Use this approach to add flux weights to existing CAFs.

```bash
source /path/to/duneanafluxtools/prefix/bin/setup.duneanafluxtools.sh
caffluxweighter inputcaf.root outputcaf.root
```

### Runtime Library

Use this approach to calculate flux weights at runtime in your event
summarisation or analysis software:

```cmake
# CMakeLists.txt
find_package(duneanafluxtools REQUIRED)

# ...

target_link_libraries(mytarget PUBLIC duneanafluxtools::all)
```

* If you operate on `caf::StandardRecord` objects, there is a helpful interface

```c++
#include "duneanafluxtools/CAFInterface.h"

// ...

  for (auto const &nu : SR->mc.nu) {
    //ratios.first is a std::vector<float> of 1sigma shift weights for each focussing parameter
    //ratios.second is a std::vector<float> of 1sigma shift weights for each hadron production PCA component
    auto ratios = ana::GetFluxWeights(nu, is_neutrino_mode);
  }

```

* If you want to manually pass the required information

```c++
#include "duneanafluxtools/FluxWeighter.h"

// ...

  auto &fw = FluxWeighter::Get();

  bool is_neutrino_mode = // set the beam mode
  bool isND = // is this event at the ND or FD

  int nupdg = //get neutrino pdg (before oscillation if at FD)
  double E_GeV = //get neutrino energy
  double off_axis_pos_m = //get off axis position at (if at ND, otherwise ignored)

  auto nu_config = fw.GetNuConfig(nu.pdgorig, isND, is_neutrino_mode);
  auto focussing_syst_bin = fw.GetFocussingBin(nupdg, E_GeV, off_axis_pos_m, nu_config);
  auto hadprod_syst_bin = fw.GetHadProdBin(nupdg, E_GeV, off_axis_pos_m, nu_config);

  for (size_t fpi = 0; fpi < fw.GetNFocussingParams(); fpi++) {
    // do something with the weight
    fw.GetFluxFocussingWeight(fpi, 1, focussing_syst_bin, nu_config);
  }
  for (size_t hppi = 0; hppi < fw.GetNHadProdPCAComponents(); hppi++) {
    // do something with the weight
    fw.GetFluxHadProdWeight(hppi, 1, hadprod_syst_bin, nu_config);
  }

```
