void rdataframe_jitted_ttree() {
    ROOT::RDataFrame df("CollectionTree", "data/data_run2/DAOD_PHYSLITE.ttree.root");
    auto goodJetPt = [](const ROOT::RVec<float> &pt, const ROOT::RVec<float> &eta) { return pt[abs(eta) < 1.0] / 1000.; };
    auto h = df.Define("goodJet_pt", goodJetPt,
                       {"AnalysisJetsAuxDyn.pt", "AnalysisJetsAuxDyn.eta"})
                 .Histo1D({"", ";Jet p_{T} (GeV);N_{Events}", 100, 15, 60},
                          "goodJet_pt");

    h->Reset();
}

void rdataframe_jitted_rntuple() {
    ROOT::RDataFrame df = ROOT::RDF::Experimental::FromRNTuple("CollectionTree", "data/data_run2/DAOD_PHYSLITE.rntuple.root");
    auto goodJetPt = [](const ROOT::RVec<float> &pt, const ROOT::RVec<float> &eta) { return pt[abs(eta) < 1.0] / 1000.; };
    auto h = df.Define("goodJet_pt", goodJetPt,
                       {"AnalysisJetsAuxDyn_pt", "AnalysisJetsAuxDyn_eta"})
                 .Histo1D({"", ";Jet p_{T} (GeV);N_{Events}", 100, 15, 60},
                          "goodJet_pt");

    h->Reset();
}

void rdataframe_jitted_physlite(std::string_view storeKind) {
    auto verbosity = ROOT::Experimental::RLogScopedVerbosity(ROOT::Detail::RDF::RDFLogChannel(),
                                                           ROOT::Experimental::ELogLevel::kInfo);

    if (storeKind == "ttree")
        rdataframe_jitted_ttree();
    else if (storeKind == "rntuple")
        rdataframe_jitted_rntuple();
}
