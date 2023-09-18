void rdataframe_jitted_ttree() {
    ROOT::RDataFrame df("CollectionTree", "data/data_run2/DAOD_PHYSLITE.ttree.root");
    auto h = df.Filter("Sum(AnalysisJetsAuxDyn.pt > 40) > 1", "More than one jet with pt > 40")
               .Define("MET", [](ROOT::RVec<float> sumet) { return sumet[sumet.size() - 1] / 1000.; }, { "MET_Core_AnalysisMETAuxDyn.sumet" })
               .Histo1D({"", ";MET (GeV);N_{Events}", 100, 0, 200}, "MET");

    h->Reset();
}

void rdataframe_jitted_rntuple() {
    ROOT::RDataFrame df = ROOT::RDF::Experimental::FromRNTuple("CollectionTree", "data/data_run2/DAOD_PHYSLITE.rntuple.root");
    auto h = df.Filter("Sum(AnalysisJetsAuxDyn_pt > 40) > 1", "More than one jet with pt > 40")
               .Define("MET", [](ROOT::RVec<float> sumet) { return sumet[sumet.size() - 1] / 1000.; }, { "MET_Core_AnalysisMETAuxDyn_sumet" })
               .Histo1D({"", ";MET (GeV);N_{Events}", 100, 0, 200}, "MET");

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
