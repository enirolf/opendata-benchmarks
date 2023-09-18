void rdataframe_ttree() {
    ROOT::RDataFrame df("CollectionTree", "data/data_run2/DAOD_PHYSLITE.ttree.root");
    auto h = df.Filter("Sum(AnalysisJetsAuxDyn.pt > 40) > 1", "More than one jet with pt > 40")
               .Define("MET_pt", "MET_Core_AnalysisMETAuxDyn.sumet[MET_Core_AnalysisMETAuxDyn.sumet.size() - 1] / 1000.")
               .Histo1D({"", ";MET (GeV);N_{Events}", 100, 0, 200}, "MET_pt");

    TCanvas c;
    h->Draw();
    c.SaveAs("4_rdataframe_jitted_physlite_ttree.png");
}

void rdataframe_rntuple() {
    ROOT::RDataFrame df = ROOT::RDF::Experimental::FromRNTuple("CollectionTree", "data/data_run2/DAOD_PHYSLITE.rntuple.root");
    auto h = df.Filter("Sum(AnalysisJetsAuxDyn_pt > 40) > 1", "More than one jet with pt > 40")
               .Define("MET_pt", "MET_Core_AnalysisMETAuxDyn_sumet[MET_Core_AnalysisMETAuxDyn_sumet.size() - 1] / 1000.")
               .Histo1D({"", ";MET (GeV);N_{Events}", 100, 0, 200}, "MET_pt");

    TCanvas c;
    h->Draw();
    c.SaveAs("4_rdataframe_jitted_physlite_rntuple.png");
}

void rdataframe_jitted_physlite(std::string_view dataFormat) {
    auto verbosity = ROOT::Experimental::RLogScopedVerbosity(ROOT::Detail::RDF::RDFLogChannel(),
                                                           ROOT::Experimental::ELogLevel::kInfo);

    if (dataFormat == "ttree")
        rdataframe_ttree();
    else if (dataFormat == "rntuple")
        rdataframe_rntuple();
}
