template <typename T> using Vec = const ROOT::RVec<T>&;
using ROOT::Math::XYZTVector;
using ROOT::Math::PtEtaPhiMVector;
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


float trijet_pt(Vec<float> pt, Vec<float> eta, Vec<float> phi, Vec<float> mass, Vec<std::size_t> idx)
{
    auto p1 = ROOT::Math::PtEtaPhiMVector(pt[idx[0]] / 1000., eta[idx[0]], phi[idx[0]], mass[idx[0]] / 1000.);
    auto p2 = ROOT::Math::PtEtaPhiMVector(pt[idx[1]] / 1000., eta[idx[1]], phi[idx[1]], mass[idx[1]] / 1000.);
    auto p3 = ROOT::Math::PtEtaPhiMVector(pt[idx[2]] / 1000., eta[idx[2]], phi[idx[2]], mass[idx[2]] / 1000.);
    return (p1 + p2 + p3).pt();
}


void rdataframe_jitted_ttree() {
    ROOT::RDataFrame df("CollectionTree", "root://eosuser.cern.ch//eos/user/f/fdegeus/adl-atlas/DAOD_PHYSLITE.ttree.root");
    auto df2 = df.Define("nJet", [](Vec<float> pts){return pts.size();}, {"AnalysisJetsAuxDyn.pt"})
                 .Filter("nJet >= 3", "At least three jets")
                 .Define("JetXYZT", [](Vec<float> pt, Vec<float> eta, Vec<float> phi, Vec<float> m) {
                              return Construct<XYZTVector>(Construct<PtEtaPhiMVector>(pt, eta, phi, m));},
                         {"AnalysisJetsAuxDyn.pt", "AnalysisJetsAuxDyn.eta", "AnalysisJetsAuxDyn.phi", "AnalysisJetsAuxDyn.m"})
                 .Define("Trijet_idx", find_trijet, {"JetXYZT"});

    auto h1 = df2.Define("Trijet_pt", trijet_pt, {"AnalysisJetsAuxDyn.pt", "AnalysisJetsAuxDyn.eta", "AnalysisJetsAuxDyn.phi", "AnalysisJetsAuxDyn.m", "Trijet_idx"})
                 .Histo1D({"", ";Trijet pt (GeV);N_{Events}", 100, 15, 40}, "Trijet_pt");
    // auto h2 = df2.Define("Trijet_leadingBtag", "Max(Take(Jet_btag, Trijet_idx))")
    //              .Histo1D({"", ";Trijet leading b-tag;N_{Events}", 100, 0, 1}, "Trijet_leadingBtag");

    auto report = df2.Report();
    report->Print();

    TCanvas cTTree;
    // cTTree.Divide(2, 1);
    // cTTree.cd(1);
    h1->Draw();
    // cTTree.cd(2);
    // h2->Draw();
    cTTree.SaveAs("6_rdf_physlite_ttree_jitted.png");
}

void rdataframe_jitted_rntuple() {
    ROOT::RDataFrame df = ROOT::RDF::Experimental::FromRNTuple("CollectionTree", "root://eosuser.cern.ch//eos/user/f/fdegeus/adl-atlas/DAOD_PHYSLITE.rntuple.root");
    auto df2 = df.Define("nJet", [](Vec<float> pts){return pts.size();}, {"AnalysisJetsAuxDyn_pt"})
                 .Filter("nJet >= 3", "At least three jets")
                 .Define("JetXYZT", [](Vec<float> pt, Vec<float> eta, Vec<float> phi, Vec<float> m) {
                              return Construct<XYZTVector>(Construct<PtEtaPhiMVector>(pt, eta, phi, m));},
                         {"AnalysisJetsAuxDyn_pt", "AnalysisJetsAuxDyn_eta", "AnalysisJetsAuxDyn_phi", "AnalysisJetsAuxDyn_m"})
                 .Define("Trijet_idx", find_trijet, {"JetXYZT"});

    auto h1 = df2.Define("Trijet_pt", trijet_pt, {"AnalysisJetsAuxDyn_pt", "AnalysisJetsAuxDyn_eta", "AnalysisJetsAuxDyn_phi", "AnalysisJetsAuxDyn_m", "Trijet_idx"})
                 .Histo1D({"", ";Trijet pt (GeV);N_{Events}", 100, 15, 40}, "Trijet_pt");
    // auto h2 = df2.Define("Trijet_leadingBtag", "Max(Take(Jet_btag, Trijet_idx))")
    //              .Histo1D({"", ";Trijet leading b-tag;N_{Events}", 100, 0, 1}, "Trijet_leadingBtag");

    auto report = df2.Report();
    report->Print();

    TCanvas cRNTuple;
    // cRNTuple.Divide(2, 1);
    // cRNTuple.cd(1);
    h1->Draw();
    // cRNTuple.cd(2);
    // h2->Draw();
    cRNTuple.SaveAs("6_rdf_physlite_rntuple_jitted.png");
}

void rdataframe_jitted_physlite() {
    // ROOT::EnableImplicitMT();
    rdataframe_jitted_ttree();
    rdataframe_jitted_rntuple();
}
