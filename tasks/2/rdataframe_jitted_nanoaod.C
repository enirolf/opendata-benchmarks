void rdataframe_ttree(std::string_view fileName) {
  auto file = std::unique_ptr<TFile>(TFile::Open(std::string(fileName).c_str()));
  auto tree = std::unique_ptr<TTree>(file->Get<TTree>("Events"));
  auto treeStats = std::make_unique<TTreePerfStats>("ioperf", tree.get());
  ROOT::RDataFrame df(*tree);

  auto h = df.Histo1D({"", ";Jet p_{T} (GeV);N_{Events}", 100, 15, 60}, "Jet_pt");

  TCanvas c;
  h->Draw();
  c.SaveAs("2_rdataframe_jitted_ttree.png");
  treeStats->Print();
}

void rdataframe_rntuple(std::string_view fileName) {
  ROOT::RDataFrame df = ROOT::RDF::Experimental::FromRNTuple("Events", fileName);

  auto h = df.Histo1D({"", ";Jet p_{T} (GeV);N_{Events}", 100, 15, 60}, "Jet_pt");

  TCanvas c;
  h->Draw();
  c.SaveAs("2_rdataframe_jitted_rntuple.png");
}

void rdataframe_jitted_nanoaod(std::string_view dataFormat, std::string_view fileName, int nThreads = 1) {
  ROOT::EnableImplicitMT(nThreads);
  auto verbosity =
      ROOT::Experimental::RLogScopedVerbosity(ROOT::Detail::RDF::RDFLogChannel(), ROOT::Experimental::ELogLevel::kInfo);

  if (dataFormat == "ttree")
    rdataframe_ttree(fileName);
  else if (dataFormat == "rntuple")
    rdataframe_rntuple(fileName);
}
