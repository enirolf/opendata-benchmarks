template <typename T> using Vec = const ROOT::RVec<T> &;

ROOT::RVec<int> find_isolated_jets(Vec<float> eta1, Vec<float> phi1, Vec<float> pt2, Vec<float> eta2, Vec<float> phi2) {
  ROOT::RVec<int> mask(eta1.size(), 1);
  if (eta2.size() == 0) {
    return mask;
  }

  const auto ptcut = pt2 > 10;
  const auto eta2_ptcut = eta2[ptcut];
  const auto phi2_ptcut = phi2[ptcut];
  if (eta2_ptcut.size() == 0) {
    return mask;
  }

  const auto c = ROOT::VecOps::Combinations(eta1, eta2_ptcut);
  for (auto i = 0u; i < c[0].size(); i++) {
    const auto i1 = c[0][i];
    const auto i2 = c[1][i];
    const auto dr = ROOT::VecOps::DeltaR(eta1[i1], eta2_ptcut[i2], phi1[i1], phi2_ptcut[i2]);
    if (dr < 0.4)
      mask[i1] = 0;
  }
  return mask;
}

void rdataframe_ttree() {
  ROOT::RDataFrame df("CollectionTree", "data/DAOD_PHYSLITE.ttree.root");

  auto h = df.Filter("AnalysisJetsAuxDyn.pt.size() > 0", "At least one jet")
             .Define("goodJet_ptcut", "(AnalysisJetsAuxDyn.pt / 1000.) > 30")
             .Define("goodJet_antiMuon", find_isolated_jets,
                     {"AnalysisJetsAuxDyn.eta", "AnalysisJetsAuxDyn.phi", "AnalysisMuonsAuxDyn.pt",
                      "AnalysisMuonsAuxDyn.eta", "AnalysisMuonsAuxDyn.phi"})
             .Define("goodJet_antiElectron", find_isolated_jets,
                     {"AnalysisJetsAuxDyn.eta", "AnalysisJetsAuxDyn.phi", "AnalysisElectronsAuxDyn.pt",
                      "AnalysisElectronsAuxDyn.eta", "AnalysisElectronsAuxDyn.phi"})
             .Define("goodJet", "goodJet_ptcut && goodJet_antiMuon && goodJet_antiElectron")
             .Filter("Sum(goodJet) > 0")
             .Define("goodJet_sumPt", "Sum(AnalysisJetsAuxDyn.pt[goodJet]) / 1000.")
             .Histo1D({"", ";Jet p_{T} sum (GeV);N_{Events}", 100, 15, 200}, "goodJet_sumPt");

  TCanvas c;
  h->Draw();
  c.SaveAs("7_rdataframe_jitted_physlite_ttree.png");
}

void rdataframe_rntuple(bool mt = false) {
  ROOT::RDataFrame df =
      ROOT::RDF::Experimental::FromRNTuple("CollectionTree", "data/DAOD_PHYSLITE.rntuple.root");

  auto h = df.Filter("AnalysisJetsAuxDyn_pt.size() > 0", "At least one jet")
             .Define("goodJet_ptcut", "(AnalysisJetsAuxDyn_pt / 1000.) > 30")
             .Define("goodJet_antiMuon", find_isolated_jets,
                     {"AnalysisJetsAuxDyn_eta", "AnalysisJetsAuxDyn_phi", "AnalysisMuonsAuxDyn_pt",
                      "AnalysisMuonsAuxDyn_eta", "AnalysisMuonsAuxDyn_phi"})
             .Define("goodJet_antiElectron", find_isolated_jets,
                     {"AnalysisJetsAuxDyn_eta", "AnalysisJetsAuxDyn_phi", "AnalysisElectronsAuxDyn_pt",
                      "AnalysisElectronsAuxDyn_eta", "AnalysisElectronsAuxDyn_phi"})
             .Define("goodJet", "goodJet_ptcut && goodJet_antiMuon && goodJet_antiElectron")
             .Filter("Sum(goodJet) > 0")
             .Define("goodJet_sumPt", "Sum(AnalysisJetsAuxDyn_pt[goodJet]) / 1000.")
             .Histo1D({"", ";Jet p_{T} sum (GeV);N_{Events}", 100, 15, 200}, "goodJet_sumPt");

  TCanvas c;
  h->Draw();
  c.SaveAs("7_rdataframe_jitted_physlite_rntuple.png");
}

void rdataframe_jitted_physlite(std::string_view dataFormat) {
  auto verbosity =
      ROOT::Experimental::RLogScopedVerbosity(ROOT::Detail::RDF::RDFLogChannel(), ROOT::Experimental::ELogLevel::kInfo);

  if (dataFormat == "ttree")
    rdataframe_ttree();
  else if (dataFormat == "rntuple")
    rdataframe_rntuple();
}
