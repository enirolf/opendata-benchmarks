void rdataframe_ttree() {
  ROOT::RDataFrame df("Events", "data/nanoaod.ttree.root");

  auto h = df.Histo1D({"", ";Jet p_{T} (GeV);N_{Events}", 100, 15, 60}, "Jet_pt");

  TCanvas c;
  h->Draw();
  c.SaveAs("2_rdataframe_jitted_nanoaod_ttree.png");
}

void rdataframe_rntuple() {
  ROOT::RDataFrame df = ROOT::RDF::Experimental::FromRNTuple("Events", "data/nanoaod.rntuple.root");

  auto h = df.Histo1D({"", ";Jet p_{T} (GeV);N_{Events}", 100, 15, 60}, "Jet_pt");

  TCanvas c;
  h->Draw();
  c.SaveAs("2_rdataframe_jitted_nanoaod_rntuple.png");
}

void rdataframe_jitted_nanoaod(std::string_view dataFormat) {
  auto verbosity =
      ROOT::Experimental::RLogScopedVerbosity(ROOT::Detail::RDF::RDFLogChannel(), ROOT::Experimental::ELogLevel::kInfo);

  if (dataFormat == "ttree")
    rdataframe_ttree();
  else if (dataFormat == "rntuple")
    rdataframe_rntuple();
}
