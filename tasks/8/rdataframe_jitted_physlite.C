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
  auto file = std::unique_ptr<TFile>(TFile::Open("data/DAOD_PHYSLITE.ttree.root"));
  auto tree = std::unique_ptr<TTree>(file->Get<TTree>("CollectionTree"));
  auto treeStats = std::make_unique<TTreePerfStats>("ioperf", tree.get());
  ROOT::RDataFrame df(*tree);

  auto h = df.Filter("AnalysisElectronsAuxDyn.pt.size() + AnalysisMuonsAuxDyn.pt.size() > 2", "At least three leptons")
             .Define("Lepton_pt", "Concatenate(AnalysisMuonsAuxDyn.pt / 1000.f, AnalysisElectronsAuxDyn.pt / 1000.f)")
             .Define("Lepton_eta", "Concatenate(AnalysisMuonsAuxDyn.eta, AnalysisElectronsAuxDyn.eta)")
             .Define("Lepton_phi", "Concatenate(AnalysisMuonsAuxDyn.phi, AnalysisElectronsAuxDyn.phi)")
             .Define("Muon_mass", "return Map(AnalysisMuonsAuxDyn.pt, [](float p){ return 0.1056583755f; })")
             .Define("Lepton_mass", "Concatenate(Muon_mass, AnalysisElectronsAuxDyn.m / 1000.f)")
             .Define("Lepton_charge", "Concatenate(AnalysisMuonsAuxDyn.charge, AnalysisElectronsAuxDyn.charge)")
             .Define("Lepton_flavour",
                     "Concatenate(ROOT::RVec<int>(AnalysisMuonsAuxDyn.pt.size(), 0), "
                                 "ROOT::RVec<int>(AnalysisElectronsAuxDyn.pt.size(), 1))")
             .Define("AdditionalLepton_idx", additional_lepton_idx,
                     {"Lepton_pt", "Lepton_eta", "Lepton_phi", "Lepton_mass", "Lepton_charge", "Lepton_flavour"})
             .Filter("AdditionalLepton_idx != 99999", "No valid lepton pair found.")
             .Define("MET_pt", "MET_Core_AnalysisMETAuxDyn.sumet[MET_Core_AnalysisMETAuxDyn.sumet.size() - 1] / 1000.")
             .Define("MET_phi", "DeltaPhi(MET_Core_AnalysisMETAuxDyn.mpx, "
                                "MET_Core_AnalysisMETAuxDyn.mpy)[MET_Core_AnalysisMETAuxDyn.mpx.size() - 1]")
             .Define("TransverseMass",
                     "sqrt(2.0 * Lepton_pt[AdditionalLepton_idx] * MET_pt "
                          "* (1.0 - cos(ROOT::VecOps::DeltaPhi(MET_phi, Lepton_phi[AdditionalLepton_idx]))))")
             .Histo1D({"", ";Transverse mass (GeV);N_{Events}", 100, 0, 200}, "TransverseMass");

  TCanvas c;
  h->Draw();
  c.SaveAs("8_rdataframe_jitted_physlite_ttree.png");
  treeStats->Print();
}

void rdataframe_rntuple() {
  ROOT::RDataFrame df =
      ROOT::RDF::Experimental::FromRNTuple("CollectionTree", "data/DAOD_PHYSLITE.rntuple.root");

  auto h = df.Filter("AnalysisElectronsAuxDyn_pt.size() + AnalysisMuonsAuxDyn_pt.size() > 2", "At least three leptons")
             .Define("Lepton_pt", "Concatenate(AnalysisMuonsAuxDyn_pt / 1000.f, AnalysisElectronsAuxDyn_pt / 1000.f)")
             .Define("Lepton_eta", "Concatenate(AnalysisMuonsAuxDyn_eta, AnalysisElectronsAuxDyn_eta)")
             .Define("Lepton_phi", "Concatenate(AnalysisMuonsAuxDyn_phi, AnalysisElectronsAuxDyn_phi)")
             .Define("Muon_mass", "return Map(AnalysisMuonsAuxDyn_pt, [](float p){ return 0.1056583755f; })")
             .Define("Lepton_mass", "Concatenate(Muon_mass, AnalysisElectronsAuxDyn_m / 1000.f)")
             .Define("Lepton_charge", "Concatenate(AnalysisMuonsAuxDyn_charge, AnalysisElectronsAuxDyn_charge)")
             .Define("Lepton_flavour", "Concatenate(ROOT::RVec<int>(AnalysisMuonsAuxDyn_pt.size(), 0), "
                                       "ROOT::RVec<int>(AnalysisElectronsAuxDyn_pt.size(), 1))")
             .Define("AdditionalLepton_idx", additional_lepton_idx,
                     {"Lepton_pt", "Lepton_eta", "Lepton_phi", "Lepton_mass", "Lepton_charge", "Lepton_flavour"})
             .Filter("AdditionalLepton_idx != 99999", "No valid lepton pair found.")
             .Define("MET_pt", "MET_Core_AnalysisMETAuxDyn_sumet[MET_Core_AnalysisMETAuxDyn_sumet.size() - 1] / 1000.")
             .Define("MET_phi", "DeltaPhi(MET_Core_AnalysisMETAuxDyn_mpx, "
                               "MET_Core_AnalysisMETAuxDyn_mpy)[MET_Core_AnalysisMETAuxDyn_mpx.size() - 1]")
             .Define("TransverseMass",
                     "sqrt(2.0 * Lepton_pt[AdditionalLepton_idx] * MET_pt "
                          "* (1.0 - cos(ROOT::VecOps::DeltaPhi(MET_phi, Lepton_phi[AdditionalLepton_idx]))))")
             .Histo1D({"", ";Transverse mass (GeV);N_{Events}", 100, 0, 200}, "TransverseMass");

  TCanvas c;
  h->Draw();
  c.SaveAs("8_rdataframe_jitted_physlite_rntuple.png");
}

void rdataframe_jitted_physlite(std::string_view dataFormat) {
  auto verbosity =
      ROOT::Experimental::RLogScopedVerbosity(ROOT::Detail::RDF::RDFLogChannel(), ROOT::Experimental::ELogLevel::kInfo);

  if (dataFormat == "ttree")
    rdataframe_ttree();
  else if (dataFormat == "rntuple")
    rdataframe_rntuple();
}
