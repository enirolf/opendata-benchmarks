void rdataframe_jitted_ttree() {
    ROOT::RDataFrame df("CollectionTree", "root://eosuser.cern.ch//eos/user/f/fdegeus/adl-atlas/DAOD_PHYSLITE.ttree.root");
    auto h = df.Define("MET", [](ROOT::RVec<float> sumet) { return sumet[sumet.size() - 1] / 1000.; }, { "MET_Core_AnalysisMETAuxDyn.sumet" })
               .Histo1D({"", ";MET (GeV);N_{Events}", 100, 0, 200}, "MET");


    TCanvas cTTree;
    h->Draw();
    cTTree.SaveAs("1_rdf_physlite_ttree_jitted.png");
}

void rdataframe_jitted_rntuple() {
    ROOT::RDataFrame df = ROOT::RDF::Experimental::FromRNTuple("CollectionTree", "root://eosuser.cern.ch//eos/user/f/fdegeus/adl-atlas/DAOD_PHYSLITE.rntuple.root");
    auto h = df.Define("MET", [](ROOT::RVec<float> sumet) { return sumet[sumet.size() - 1] / 1000.; }, { "MET_Core_AnalysisMETAuxDyn_sumet" })
               .Histo1D({"", ";MET (GeV);N_{Events}", 100, 0, 200}, "MET");


    TCanvas cRNTuple;
    h->Draw();
    cRNTuple.SaveAs("1_rdf_physlite_rntuple_jitted.png");
}

void rdataframe_jitted_physlite() {
    // ROOT::EnableImplicitMT();
    rdataframe_jitted_ttree();
    rdataframe_jitted_rntuple();
}
