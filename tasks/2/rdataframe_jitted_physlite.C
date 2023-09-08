void rdataframe_jitted_ttree() {
    ROOT::RDataFrame df("CollectionTree", "root://eosuser.cern.ch//eos/user/f/fdegeus/adl-atlas/DAOD_PHYSLITE.ttree.root");

    auto h = df.Redefine("AnalysisJetsAuxDyn.pt",
                         [](ROOT::RVec<float> pts) { return pts / 1000.; },
                         {"AnalysisJetsAuxDyn.pt"})
                 .Histo1D({"", ";Jet p_{T} (GeV);N_{Events}", 100, 15, 60},
                          "AnalysisJetsAuxDyn.pt");

    TCanvas cTTree;
    h->Draw();
    cTTree.SaveAs("2_rdf_physlite_ttree_jitted.png");
}

void rdataframe_jitted_rntuple() {
    ROOT::RDataFrame df = ROOT::RDF::Experimental::FromRNTuple("CollectionTree", "root://eosuser.cern.ch//eos/user/f/fdegeus/adl-atlas/DAOD_PHYSLITE.rntuple.root");
    auto h = df.Redefine("AnalysisJetsAuxDyn_pt",
                         [](ROOT::RVec<float> pts) { return pts / 1000.; },
                         {"AnalysisJetsAuxDyn_pt"})
                 .Histo1D({"", ";Jet p_{T} (GeV);N_{Events}", 100, 15, 60},
                          "AnalysisJetsAuxDyn_pt");

    TCanvas cTTree;
    h->Draw();
    cTTree.SaveAs("2_rdf_physlite_rntuple_jitted.png");
}

void rdataframe_jitted_physlite() {
    // ROOT::EnableImplicitMT();
    rdataframe_jitted_ttree();
    rdataframe_jitted_rntuple();
}
