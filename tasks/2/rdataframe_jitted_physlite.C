void rdataframe_ttree() {
  ROOT::RDataFrame df("CollectionTree", "data/data_run2/DAOD_PHYSLITE.ttree.root");

  auto h = df.Redefine("AnalysisJetsAuxDyn.pt", "AnalysisJetsAuxDyn.pt / 1000.")
             .Histo1D({"ttree", ";Jet p_{T} (GeV);N_{Events}", 100, 15, 60}, "AnalysisJetsAuxDyn.pt");

  TCanvas c;
  h->Draw();
  c.SaveAs("2_rdataframe_jitted_physlite_ttree.png");
}

void rdataframe_rntuple() {
  ROOT::RDataFrame df =
      ROOT::RDF::Experimental::FromRNTuple("CollectionTree", "data/data_run2/DAOD_PHYSLITE.rntuple.root");

  auto h = df.Redefine("AnalysisJetsAuxDyn_pt", "AnalysisJetsAuxDyn_pt / 1000.")
             .Histo1D({"rntuple", ";Jet p_{T} (GeV);N_{Events}", 100, 15, 60}, "AnalysisJetsAuxDyn_pt");

  TCanvas c;
  h->Draw();
  c.SaveAs("2_rdataframe_jitted_physlite_rntuple.png");
}

void rdataframe_jitted_physlite(std::string_view dataFormat) {
  auto verbosity =
      ROOT::Experimental::RLogScopedVerbosity(ROOT::Detail::RDF::RDFLogChannel(), ROOT::Experimental::ELogLevel::kInfo);

  if (dataFormat == "ttree")
    rdataframe_ttree();
  else if (dataFormat == "rntuple")
    rdataframe_rntuple();
}
