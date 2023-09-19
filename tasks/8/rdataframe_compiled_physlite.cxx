#include <iostream>
#include <string>

#include "Math/Vector4D.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RLogger.hxx"
#include "ROOT/RNTupleDS.hxx"
#include "TCanvas.h"

template <typename T> using Vec = const ROOT::RVec<T> &;
using FourVector = ROOT::Math::PtEtaPhiMVector;
constexpr static unsigned int PLACEHOLDER_VALUE = 99999;

unsigned int additional_lepton_idx(Vec<float> pt, Vec<float> eta, Vec<float> phi, Vec<float> mass, Vec<float> charge,
                                   Vec<int> flavour) {
  const auto c = Combinations(pt, 2);
  float best_mass = PLACEHOLDER_VALUE;
  unsigned int best_i1 = PLACEHOLDER_VALUE;
  unsigned int best_i2 = PLACEHOLDER_VALUE;
  const auto z_mass = 91.2;
  const auto make_p4 = [&](std::size_t idx) {
    return ROOT::Math::PtEtaPhiMVector(pt[idx], eta[idx], phi[idx], mass[idx]);
  };

  for (auto i = 0u; i < c[0].size(); i++) {
    const auto i1 = c[0][i];
    const auto i2 = c[1][i];
    if (charge[i1] == charge[i2])
      continue;
    if (flavour[i1] != flavour[i2])
      continue;
    const auto tmp_mass = (make_p4(i1) + make_p4(i2)).mass();
    if (std::abs(tmp_mass - z_mass) < std::abs(best_mass - z_mass)) {
      best_mass = tmp_mass;
      best_i1 = i1;
      best_i2 = i2;
    }
  }

  if (best_i1 == PLACEHOLDER_VALUE)
    return PLACEHOLDER_VALUE;

  float max_pt = -999;
  unsigned int lep_idx = PLACEHOLDER_VALUE;
  for (auto i = 0u; i < pt.size(); i++) {
    if (i != best_i1 && i != best_i2 && pt[i] > max_pt) {
      max_pt = pt[i];
      lep_idx = i;
    }
  }

  return lep_idx;
}

void rdataframe_ttree() {
  ROOT::RDataFrame df("CollectionTree", "data/data_run2/DAOD_PHYSLITE.ttree.root");

  auto concatF = [](Vec<float> a, Vec<float> b) { return Concatenate(a, b); };
  auto concatI = [](Vec<int> a, Vec<int> b) { return Concatenate(a, b); };

  auto transverseMass = [](Vec<float> Lepton_pt, Vec<float> Lepton_phi, float MET_pt, float MET_phi, unsigned int idx) {
    return sqrt(2.0 * Lepton_pt[idx] * MET_pt * (1.0 - cos(ROOT::VecOps::DeltaPhi(MET_phi, Lepton_phi[idx]))));
  };

  auto h = df.Filter([](Vec<float> muons, Vec<float> elecs) { return muons.size() + elecs.size() > 2; },
                     {"AnalysisMuonsAuxDyn.pt", "AnalysisElectronsAuxDyn.pt"}, "At least three leptons")
             .Define("Lepton_pt", [](Vec<float> m, Vec<float> e) { return Concatenate(m, e) / 1000.f; },
                     {"AnalysisMuonsAuxDyn.pt", "AnalysisElectronsAuxDyn.pt"})
             .Define("Lepton_eta", concatF, {"AnalysisMuonsAuxDyn.eta", "AnalysisElectronsAuxDyn.eta"})
             .Define("Lepton_phi", concatF, {"AnalysisMuonsAuxDyn.phi", "AnalysisElectronsAuxDyn.phi"})
             .Define("Muon_mass", [](Vec<float> pts) { return Map(pts, [](float p) { return 0.1056583755f; }); },
                     {"AnalysisMuonsAuxDyn.pt"})
             .Define("Lepton_mass", [](Vec<float> m, Vec<float> e) { return Concatenate(m, e / 1000.f); },
                     {"Muon_mass", "AnalysisElectronsAuxDyn.m"})
             .Define("Lepton_charge", concatF, {"AnalysisMuonsAuxDyn.charge", "AnalysisElectronsAuxDyn.charge"})
             .Define("Lepton_flavour",
                     [](Vec<float> muons, Vec<float> elecs) {
                       return Concatenate(ROOT::RVec<int>(muons.size(), 0), ROOT::RVec<int>(elecs.size(), 1));
                     },
                     {"AnalysisMuonsAuxDyn.pt", "AnalysisElectronsAuxDyn.pt"})
             .Define("AdditionalLepton_idx", additional_lepton_idx,
                     {"Lepton_pt", "Lepton_eta", "Lepton_phi", "Lepton_mass", "Lepton_charge", "Lepton_flavour"})
             .Filter([](unsigned int idx) { return idx != PLACEHOLDER_VALUE; }, {"AdditionalLepton_idx"},
                     "No valid lepton pair found.")
             .Define("MET_pt", [](Vec<float> sumet) { return sumet[sumet.size() - 1] / 1000.f; },
                     {"MET_Core_AnalysisMETAuxDyn.sumet"})
             .Define("MET_phi", [](Vec<float> metX, Vec<float> metY) { return DeltaPhi(metX, metY)[metX.size() - 1]; },
                     {"MET_Core_AnalysisMETAuxDyn.mpx", "MET_Core_AnalysisMETAuxDyn.mpy"})
             .Define("TransverseMass", transverseMass,
                     {"Lepton_pt", "Lepton_phi", "MET_pt", "MET_phi", "AdditionalLepton_idx"})
             .Histo1D({"", ";Transverse mass (GeV);N_{Events}", 100, 0, 200}, "TransverseMass");

  TCanvas c;
  h->Draw();
  c.SaveAs("8_rdataframe_compiled_physlite_ttree.png");
}

void rdataframe_rntuple() {
  ROOT::RDataFrame df =
      ROOT::RDF::Experimental::FromRNTuple("CollectionTree", "data/data_run2/DAOD_PHYSLITE.rntuple.root");

  auto concatF = [](Vec<float> a, Vec<float> b) { return Concatenate(a, b); };
  auto concatI = [](Vec<int> a, Vec<int> b) { return Concatenate(a, b); };

  auto transverseMass = [](Vec<float> Lepton_pt, Vec<float> Lepton_phi, float MET_pt, float MET_phi, unsigned int idx) {
    return sqrt(2.0 * Lepton_pt[idx] * MET_pt * (1.0 - cos(ROOT::VecOps::DeltaPhi(MET_phi, Lepton_phi[idx]))));
  };

  auto h = df.Filter([](Vec<float> muons, Vec<float> elecs) { return muons.size() + elecs.size() > 2; },
                   {"AnalysisMuonsAuxDyn_pt", "AnalysisElectronsAuxDyn_pt"}, "At least three leptons")
             .Define("Lepton_pt", [](Vec<float> m, Vec<float> e) { return Concatenate(m, e) / 1000.f; },
                     {"AnalysisMuonsAuxDyn_pt", "AnalysisElectronsAuxDyn_pt"})
             .Define("Lepton_eta", concatF, {"AnalysisMuonsAuxDyn_eta", "AnalysisElectronsAuxDyn_eta"})
             .Define("Lepton_phi", concatF, {"AnalysisMuonsAuxDyn_phi", "AnalysisElectronsAuxDyn_phi"})
             .Define("Muon_mass", [](Vec<float> pts) { return Map(pts, [](float p) { return 0.1056583755f; }); },
                     {"AnalysisMuonsAuxDyn_pt"})
             .Define("Lepton_mass", [](Vec<float> m, Vec<float> e) { return Concatenate(m, e / 1000.f); },
                     {"Muon_mass", "AnalysisElectronsAuxDyn_m"})
             .Define("Lepton_charge", concatF, {"AnalysisMuonsAuxDyn_charge", "AnalysisElectronsAuxDyn_charge"})
             .Define("Lepton_flavour",
                     [](Vec<float> muons, Vec<float> elecs) {
                       return Concatenate(ROOT::RVec<int>(muons.size(), 0), ROOT::RVec<int>(elecs.size(), 1));
                     },
                     {"AnalysisMuonsAuxDyn_pt", "AnalysisElectronsAuxDyn_pt"})
             .Define("AdditionalLepton_idx", additional_lepton_idx,
                     {"Lepton_pt", "Lepton_eta", "Lepton_phi", "Lepton_mass", "Lepton_charge", "Lepton_flavour"})
             .Filter([](unsigned int idx) { return idx != PLACEHOLDER_VALUE; }, {"AdditionalLepton_idx"},
                     "No valid lepton pair found.")
             .Define("MET_pt", [](Vec<float> sumet) { return sumet[sumet.size() - 1] / 1000.f; },
                     {"MET_Core_AnalysisMETAuxDyn_sumet"})
             .Define("MET_phi", [](Vec<float> metX, Vec<float> metY) { return DeltaPhi(metX, metY)[metX.size() - 1]; },
                     {"MET_Core_AnalysisMETAuxDyn_mpx", "MET_Core_AnalysisMETAuxDyn_mpy"})
             .Define("TransverseMass", transverseMass,
                     {"Lepton_pt", "Lepton_phi", "MET_pt", "MET_phi", "AdditionalLepton_idx"})
             .Histo1D({"", ";Transverse mass (GeV);N_{Events}", 100, 0, 200}, "TransverseMass");

  TCanvas c;
  h->Draw();
  c.SaveAs("8_rdataframe_compiled_physlite_rntuple.png");
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
