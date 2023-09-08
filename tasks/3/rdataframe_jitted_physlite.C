void rdataframe_jitted_ttree() {
    ROOT::RDataFrame df("CollectionTree", "root://eosuser.cern.ch//eos/user/f/fdegeus/adl-atlas/DAOD_PHYSLITE.ttree.root");
    auto goodJetPt = [](const ROOT::RVec<float> &pt, const ROOT::RVec<float> &eta) { return pt[abs(eta) < 1.0] / 1000.; };
    auto h = df.Define("goodJet_pt", goodJetPt,
                       {"AnalysisJetsAuxDyn.pt", "AnalysisJetsAuxDyn.eta"})
                 .Histo1D({"", ";Jet p_{T} (GeV);N_{Events}", 100, 15, 60},
                          "goodJet_pt");

    TCanvas cTTree;
    h->Draw();
    cTTree.SaveAs("3_rdf_physlite_ttree_jitted.png");
}

void rdataframe_jitted_rntuple() {
    ROOT::RDataFrame df = ROOT::RDF::Experimental::FromRNTuple("CollectionTree", "root://eosuser.cern.ch//eos/user/f/fdegeus/adl-atlas/DAOD_PHYSLITE.rntuple.root");
    auto goodJetPt = [](const ROOT::RVec<float> &pt, const ROOT::RVec<float> &eta) { return pt[abs(eta) < 1.0] / 1000.; };
    auto h = df.Define("goodJet_pt", goodJetPt,
                       {"AnalysisJetsAuxDyn_pt", "AnalysisJetsAuxDyn_eta"})
                 .Histo1D({"", ";Jet p_{T} (GeV);N_{Events}", 100, 15, 60},
                          "goodJet_pt");

    TCanvas cRNTuple;
    h->Draw();
    cRNTuple.SaveAs("3_rdf_physlite_rntuple_jitted.png");
}

void rdataframe_jitted_physlite() {
    // ROOT::EnableImplicitMT();
    rdataframe_jitted_ttree();
    rdataframe_jitted_rntuple();
}
