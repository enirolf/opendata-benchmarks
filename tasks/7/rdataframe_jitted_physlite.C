template <typename T> using Vec = const ROOT::RVec<T>&;

ROOT::RVec<int> find_isolated_jets(Vec<float> eta1, Vec<float> phi1, Vec<float> pt2, Vec<float> eta2, Vec<float> phi2)
{
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
        if (dr < 0.4) mask[i1] = 0;
    }
    return mask;
}

void rdataframe_jitted_ttree() {
    ROOT::RDataFrame df("CollectionTree", "root://eosuser.cern.ch//eos/user/f/fdegeus/adl-atlas/DAOD_PHYSLITE.ttree.root");
    auto h = df.Define("nJet", [](Vec<float> pts){return pts.size();}, {"AnalysisJetsAuxDyn.pt"})
               .Filter("nJet > 0", "At least one jet")
               .Define("goodJet_ptcut", "(AnalysisJetsAuxDyn.pt / 1000.) > 30")
               .Define("goodJet_antiMuon", find_isolated_jets, {"AnalysisJetsAuxDyn.eta", "AnalysisJetsAuxDyn.phi", "AnalysisMuonsAuxDyn.pt", "AnalysisMuonsAuxDyn.eta", "AnalysisMuonsAuxDyn.phi"})
               .Define("goodJet_antiElectron", find_isolated_jets, {"AnalysisJetsAuxDyn.eta", "AnalysisJetsAuxDyn.phi", "AnalysisElectronsAuxDyn.pt", "AnalysisElectronsAuxDyn.eta", "AnalysisElectronsAuxDyn.phi"})
               .Define("goodJet", "goodJet_ptcut && goodJet_antiMuon && goodJet_antiElectron")
               .Filter("Sum(goodJet) > 0")
               .Define("goodJet_sumPt", "Sum(AnalysisJetsAuxDyn.pt[goodJet]) / 1000.")
               .Histo1D({"", ";Jet p_{T} sum (GeV);N_{Events}", 100, 15, 200}, "goodJet_sumPt");

    TCanvas cTTree;
    h->Draw();
    cTTree.SaveAs("7_rdf_physlite_ttree_jitted.png");
}

void rdataframe_jitted_rntuple(bool mt = false) {
    ROOT::RDataFrame df = ROOT::RDF::Experimental::FromRNTuple("CollectionTree", "root://eosuser.cern.ch//eos/user/f/fdegeus/adl-atlas/DAOD_PHYSLITE.rntuple.root");
    auto h = df.Filter("nJet > 0", "At least one jet")
           .Define("goodJet_ptcut", "Jet_pt > 30")
           .Define("goodJet_antiMuon", find_isolated_jets, {"Jet_eta", "Jet_phi", "Muon_pt", "Muon_eta", "Muon_phi"})
           .Define("goodJet_antiElectron", find_isolated_jets, {"Jet_eta", "Jet_phi", "Electron_pt", "Electron_eta", "Electron_phi"})
           .Define("goodJet", "goodJet_ptcut && goodJet_antiMuon && goodJet_antiElectron")
           .Filter("Sum(goodJet) > 0")
           .Define("goodJet_sumPt", "Sum(Jet_pt[goodJet])")
           .Histo1D({"", ";Jet p_{T} sum (GeV);N_{Events}", 100, 15, 200}, "goodJet_sumPt");


    TCanvas cTTree;
    h->Draw();
    cTTree.SaveAs("7_rdf_physlite_rntuple_jitted.png");
}

void rdataframe_jitted_physlite() {
    // ROOT::EnableImplicitMT();
    rdataframe_jitted_ttree();
    rdataframe_jitted_rntuple();
}

