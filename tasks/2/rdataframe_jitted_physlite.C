void rdataframe_jitted_ttree() {
    ROOT::RDataFrame df("CollectionTree", "data/data_run2/DAOD_PHYSLITE.ttree.root");

    auto h = df.Redefine("AnalysisJetsAuxDyn.pt",
                         [](ROOT::RVec<float> pts) { return pts / 1000.; },
                         {"AnalysisJetsAuxDyn.pt"})
                 .Histo1D({"ttree", ";Jet p_{T} (GeV);N_{Events}", 100, 15, 60},
                          "AnalysisJetsAuxDyn.pt");

    h->Reset();
}

void rdataframe_jitted_rntuple() {
    ROOT::RDataFrame df = ROOT::RDF::Experimental::FromRNTuple("CollectionTree", "data/data_run2/DAOD_PHYSLITE.rntuple.root");
    auto h = df.Redefine("AnalysisJetsAuxDyn_pt",
                         [](ROOT::RVec<float> pts) { return pts / 1000.; },
                         {"AnalysisJetsAuxDyn_pt"})
                 .Histo1D({"rntuple", ";Jet p_{T} (GeV);N_{Events}", 100, 15, 60},
                          "AnalysisJetsAuxDyn_pt");
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
