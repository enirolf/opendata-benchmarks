#include <iostream>
#include <string>

#include "Math/Vector4D.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RLogger.hxx"
#include "ROOT/RNTupleDS.hxx"
#include "TCanvas.h"

template <typename T> using Vec = const ROOT::RVec<T> &;
using ROOT::Math::PtEtaPhiMVector;
using ROOT::Math::XYZTVector;
using ROOT::VecOps::Construct;

ROOT::RVec<std::size_t> find_trijet(Vec<XYZTVector> jets) {
  constexpr std::size_t n = 3;
  float distance = 1e9;
  const auto top_mass = 172.5;
  std::size_t idx1 = 0, idx2 = 1, idx3 = 2;

  for (std::size_t i = 0; i <= jets.size() - n; i++) {
    auto p1 = jets[i];
    for (std::size_t j = i + 1; j <= jets.size() - n + 1; j++) {
      auto p2 = jets[j];
      for (std::size_t k = j + 1; k <= jets.size() - n + 2; k++) {
        auto p3 = jets[k];
        const auto tmp_mass = (p1 + p2 + p3).mass();
        const auto tmp_distance = std::abs(tmp_mass - top_mass);
        if (tmp_distance < distance) {
          distance = tmp_distance;
          idx1 = i;
          idx2 = j;
          idx3 = k;
        }
      }
    }
  }
  return {idx1, idx2, idx3};
}

float trijet_pt(Vec<float> pt, Vec<float> eta, Vec<float> phi, Vec<float> mass, Vec<std::size_t> idx) {
  auto p1 = ROOT::Math::PtEtaPhiMVector(pt[idx[0]] / 1000., eta[idx[0]], phi[idx[0]], mass[idx[0]] / 1000.);
  auto p2 = ROOT::Math::PtEtaPhiMVector(pt[idx[1]] / 1000., eta[idx[1]], phi[idx[1]], mass[idx[1]] / 1000.);
  auto p3 = ROOT::Math::PtEtaPhiMVector(pt[idx[2]] / 1000., eta[idx[2]], phi[idx[2]], mass[idx[2]] / 1000.);
  return (p1 + p2 + p3).pt();
}

void rdataframe_ttree() {
  ROOT::RDataFrame df("CollectionTree", "data/DAOD_PHYSLITE.ttree.root");

  auto df2 = df.Filter([](Vec<float> pts) { return pts.size() >= 3; }, {"AnalysisJetsAuxDyn.pt"}, "At least three jets")
               .Define("JetXYZT",
                       [](Vec<float> pt, Vec<float> eta, Vec<float> phi, Vec<float> m) {
                         return Construct<XYZTVector>(Construct<PtEtaPhiMVector>(pt, eta, phi, m));
                       },
                       {"AnalysisJetsAuxDyn.pt", "AnalysisJetsAuxDyn.eta", "AnalysisJetsAuxDyn.phi", "AnalysisJetsAuxDyn.m"})
               .Define("Trijet_idx", find_trijet, {"JetXYZT"});

  auto h1 = df2.Define("Trijet_pt", trijet_pt,
                      {"AnalysisJetsAuxDyn.pt", "AnalysisJetsAuxDyn.eta",
                       "AnalysisJetsAuxDyn.phi", "AnalysisJetsAuxDyn.m", "Trijet_idx"})
               .Histo1D<float>({"", ";Trijet pt (GeV);N_{Events}", 100, 15, 40}, "Trijet_pt");

  TCanvas c;
  h1->Draw();
  c.SaveAs("6_rdataframe_compiled_physlite_ttree.png");
}

void rdataframe_rntuple() {
  ROOT::RDataFrame df =
      ROOT::RDF::Experimental::FromRNTuple("CollectionTree", "data/DAOD_PHYSLITE.rntuple.root");

  auto df2 = df.Filter([](Vec<float> pts) { return pts.size() >= 3; }, {"AnalysisJetsAuxDyn_pt"}, "At least three jets")
               .Define("JetXYZT",
                       [](Vec<float> pt, Vec<float> eta, Vec<float> phi, Vec<float> m) {
                         return Construct<XYZTVector>(Construct<PtEtaPhiMVector>(pt, eta, phi, m));
                       },
                       {"AnalysisJetsAuxDyn_pt", "AnalysisJetsAuxDyn_eta", "AnalysisJetsAuxDyn_phi", "AnalysisJetsAuxDyn_m"})
               .Define("Trijet_idx", find_trijet, {"JetXYZT"});

  auto h1 = df2.Define("Trijet_pt", trijet_pt,
                      {"AnalysisJetsAuxDyn_pt", "AnalysisJetsAuxDyn_eta",
                       "AnalysisJetsAuxDyn_phi", "AnalysisJetsAuxDyn_m", "Trijet_idx"})
               .Histo1D<float>({"", ";Trijet pt (GeV);N_{Events}", 100, 15, 40}, "Trijet_pt");

  TCanvas c;
  h1->Draw();
  c.SaveAs("6_rdataframe_compiled_physlite_rntuple.png");
}

int main(int argc, char const *argv[]) {
  std::cerr << "WARNING: This implementation is incomplete, do *NOT* use for benchmarking!" << std::endl;

  if (argc < 2) {
    std::cerr << "Please provide the data format ('ttree' or 'rntuple')" << std::endl;
    return 1;
  }

  auto verbosity =
      ROOT::Experimental::RLogScopedVerbosity(ROOT::Detail::RDF::RDFLogChannel(), ROOT::Experimental::ELogLevel::kInfo);

  std::string dataFormat = std::string(argv[1]);

  if (dataFormat == "ttree") {
    rdataframe_ttree();
  } else if (dataFormat == "rntuple") {
    rdataframe_rntuple();
  } else {
    std::cerr << "Invalid data format specified (use 'ttree' or 'rntuple')" << std::endl;
  }
  return 0;
}
