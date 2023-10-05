#include <iostream>
#include <string>

#include "Math/Vector4D.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RLogger.hxx"
#include "ROOT/RNTupleDS.hxx"
#include "TCanvas.h"

template <typename T> using Vec = const ROOT::RVec<T> &;
using FourVector = ROOT::Math::PtEtaPhiMVector;

auto compute_dimuon_masses(Vec<float> pt, Vec<float> eta, Vec<float> phi, Vec<float> charge) {
  const float mass = 0.1056583755;
  ROOT::RVec<float> masses;
  const auto c = ROOT::VecOps::Combinations(pt, 2);
  for (auto i = 0u; i < c[0].size(); i++) {
    const auto i1 = c[0][i];
    const auto i2 = c[1][i];
    if (charge[i1] == charge[i2])
      continue;
    const FourVector p1(pt[i1] / 1000., eta[i1], phi[i1], mass);
    const FourVector p2(pt[i2] / 1000., eta[i2], phi[i2], mass);
    masses.push_back((p1 + p2).mass());
  }
  return masses;
};

void rdataframe_ttree() {
  ROOT::RDataFrame df("CollectionTree", "data/DAOD_PHYSLITE.ttree.root");

  auto h = df.Filter([](Vec<float> pt) { return pt.size() >= 2; }, {"AnalysisMuonsAuxDyn.pt"}, "At least two muons")
             .Define("Dimuon_mass", compute_dimuon_masses,
                     {"AnalysisMuonsAuxDyn.pt", "AnalysisMuonsAuxDyn.eta",
                      "AnalysisMuonsAuxDyn.phi", "AnalysisMuonsAuxDyn.charge"})
             .Filter([](Vec<float> mass) { return Sum(mass > 60 && mass < 120) > 0; }, {"Dimuon_mass"},
                     "At least one dimuon system with mass in range [60, 120]")
             .Define("MET_pt", [](Vec<float> sumet) { return sumet[sumet.size() - 1] / 1000.f; },
                     {"MET_Core_AnalysisMETAuxDyn.sumet"})
             .Histo1D<float>({"", ";MET (GeV);N_{Events}", 100, 0, 200}, "MET_pt");

  TCanvas c;
  h->Draw();
  c.SaveAs("5_rdataframe_compiled_physlite_ttree.png");
}

void rdataframe_rntuple() {
  ROOT::RDataFrame df =
      ROOT::RDF::Experimental::FromRNTuple("CollectionTree", "data/DAOD_PHYSLITE.rntuple.root");

  auto h = df.Filter([](Vec<float> pt) { return pt.size() >= 2; }, {"AnalysisMuonsAuxDyn_pt"}, "At least two muons")
             .Define("Dimuon_mass", compute_dimuon_masses,
                     {"AnalysisMuonsAuxDyn_pt", "AnalysisMuonsAuxDyn_eta",
                      "AnalysisMuonsAuxDyn_phi", "AnalysisMuonsAuxDyn_charge"})
             .Filter([](Vec<float> mass) { return Sum(mass > 60 && mass < 120) > 0; }, {"Dimuon_mass"},
                     "At least one dimuon system with mass in range [60, 120]")
             .Define("MET_pt", [](Vec<float> sumet) { return sumet[sumet.size() - 1] / 1000.f; },
                     {"MET_Core_AnalysisMETAuxDyn_sumet"})
             .Histo1D<float>({"", ";MET (GeV);N_{Events}", 100, 0, 200}, "MET_pt");

  TCanvas c;
  h->Draw();
  c.SaveAs("5_rdataframe_compiled_physlite_rntuple.png");
}

int main(int argc, char const *argv[]) {
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
