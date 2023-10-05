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

  auto h = df.Filter("AnalysisMuonsAuxDyn.pt.size() >= 2", "At least two muons")
             .Define("Dimuon_mass", compute_dimuon_masses,
                     {"AnalysisMuonsAuxDyn.pt", "AnalysisMuonsAuxDyn.eta",
                      "AnalysisMuonsAuxDyn.phi", "AnalysisMuonsAuxDyn.charge"})
             .Filter("Sum(Dimuon_mass > 60 && Dimuon_mass < 120) > 0",
                     "At least one dimuon system with mass in range [60, 120]")
             .Define("MET_pt", "MET_Core_AnalysisMETAuxDyn.sumet[MET_Core_AnalysisMETAuxDyn.sumet.size() - 1] / 1000.")
             .Histo1D({"", ";MET (GeV);N_{Events}", 100, 0, 200}, "MET_pt");

  TCanvas c;
  h->Draw();
  c.SaveAs("5_rdataframe_jitted_physlite_ttree.png");
}

void rdataframe_rntuple() {
  ROOT::RDataFrame df =
      ROOT::RDF::Experimental::FromRNTuple("CollectionTree", "data/DAOD_PHYSLITE.rntuple.root");

  auto h = df.Filter("AnalysisMuonsAuxDyn_pt.size() >= 2", "At least two muons")
             .Define("Dimuon_mass", compute_dimuon_masses,
                     {"AnalysisMuonsAuxDyn_pt", "AnalysisMuonsAuxDyn_eta",
                      "AnalysisMuonsAuxDyn_phi", "AnalysisMuonsAuxDyn_charge"})
             .Filter("Sum(Dimuon_mass > 60 && Dimuon_mass < 120) > 0",
                     "At least one dimuon system with mass in range [60, 120]")
             .Define("MET_pt", "MET_Core_AnalysisMETAuxDyn_sumet[MET_Core_AnalysisMETAuxDyn_sumet.size() - 1] / 1000.")
             .Histo1D({"", ";MET (GeV);N_{Events}", 100, 0, 200}, "MET_pt");

  TCanvas c;
  h->Draw();
  c.SaveAs("5_rdataframe_jitted_physlite_rntuple.png");
}

void rdataframe_jitted_physlite(std::string_view dataFormat) {
  auto verbosity =
      ROOT::Experimental::RLogScopedVerbosity(ROOT::Detail::RDF::RDFLogChannel(), ROOT::Experimental::ELogLevel::kInfo);

  if (dataFormat == "ttree")
    rdataframe_ttree();
  else if (dataFormat == "rntuple")
    rdataframe_rntuple();
}
