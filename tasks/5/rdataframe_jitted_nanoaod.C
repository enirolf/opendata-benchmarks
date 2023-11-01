template <typename T> using Vec = const ROOT::RVec<T> &;
using FourVector = ROOT::Math::PtEtaPhiMVector;

auto compute_dimuon_masses(Vec<float> pt, Vec<float> eta, Vec<float> phi, Vec<float> mass, Vec<int> charge) {
  ROOT::RVec<float> masses;
  const auto c = ROOT::VecOps::Combinations(pt, 2);
  for (auto i = 0u; i < c[0].size(); i++) {
    const auto i1 = c[0][i];
    const auto i2 = c[1][i];
    if (charge[i1] == charge[i2])
      continue;
    const FourVector p1(pt[i1], eta[i1], phi[i1], mass[i1]);
    const FourVector p2(pt[i2], eta[i2], phi[i2], mass[i2]);
    masses.push_back((p1 + p2).mass());
  }
  return masses;
};

void rdataframe_ttree() {
  auto file = std::unique_ptr<TFile>(TFile::Open("data/nanoaod_1M.ttree.root"));
  auto tree = std::unique_ptr<TTree>(file->Get<TTree>("Events"));
  auto treeStats = std::make_unique<TTreePerfStats>("ioperf", tree.get());
  ROOT::RDataFrame df(*tree);

  auto h = df.Filter("nMuon >= 2", "At least two muons")
             .Define("Dimuon_mass", compute_dimuon_masses,
                     {"Muon_pt", "Muon_eta", "Muon_phi", "Muon_mass", "Muon_charge"})
             .Filter("Sum(Dimuon_mass > 60 && Dimuon_mass < 120) > 0",
                     "At least one dimuon system with mass in range [60, 120]")
             .Histo1D({"", ";MET (GeV);N_{Events}", 100, 0, 200}, "MET_pt");

  TCanvas c;
  h->Draw();
  c.SaveAs("5_rdataframe_jitted_nanoaod_ttree.png");
  treeStats->Print();
}

void rdataframe_rntuple() {
  ROOT::RDataFrame df = ROOT::RDF::Experimental::FromRNTuple("Events", "data/nanoaod_1M.rntuple.root");

  auto h = df.Filter("nMuon >= 2", "At least two muons")
             .Define("Dimuon_mass", compute_dimuon_masses,
                     {"Muon_pt", "Muon_eta", "Muon_phi", "Muon_mass", "Muon_charge"})
             .Filter("Sum(Dimuon_mass > 60 && Dimuon_mass < 120) > 0",
                     "At least one dimuon system with mass in range [60, 120]")
             .Histo1D({"", ";MET (GeV);N_{Events}", 100, 0, 200}, "MET_pt");

  TCanvas c;
  h->Draw();
  c.SaveAs("5_rdataframe_jitted_nanoaod_rntuple.png");
}


void rdataframe_jitted_nanoaod(std::string_view dataFormat) {
  auto verbosity =
      ROOT::Experimental::RLogScopedVerbosity(ROOT::Detail::RDF::RDFLogChannel(), ROOT::Experimental::ELogLevel::kInfo);

  if (dataFormat == "ttree")
    rdataframe_ttree();
  else if (dataFormat == "rntuple")
    rdataframe_rntuple();
}
