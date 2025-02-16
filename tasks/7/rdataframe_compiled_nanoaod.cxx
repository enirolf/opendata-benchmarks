#include "ROOT/RDataFrame.hxx"
#include "ROOT/RLogger.hxx"
#include "ROOT/RNTupleDS.hxx"

#include "TCanvas.h"
#include "TTreePerfStats.h"
#include "TString.h"

template <typename T> using Vec = const ROOT::RVec<T> &;

ROOT::RVec<int> find_isolated_jets(Vec<float> eta1, Vec<float> phi1, Vec<float> pt2, Vec<float> eta2, Vec<float> phi2) {
  ROOT::RVec<int> mask(eta1.size(), 1);
  if (eta2.size() == 0) {
    return mask;
  }

  const auto ptcut = pt2 > 10;
  const auto eta2_ptcut = eta2[ptcut];
  const auto phi2_ptcut = phi2[ptcut];
  if (eta2_ptcut.size() == 0) {
    return mask;
  }

  const auto c = ROOT::VecOps::Combinations(eta1, eta2_ptcut);
  for (auto i = 0u; i < c[0].size(); i++) {
    const auto i1 = c[0][i];
    const auto i2 = c[1][i];
    const auto dr = ROOT::VecOps::DeltaR(eta1[i1], eta2_ptcut[i2], phi1[i1], phi2_ptcut[i2]);
    if (dr < 0.4)
      mask[i1] = 0;
  }
  return mask;
}

void rdataframe_ttree() {
  auto file = std::unique_ptr<TFile>(TFile::Open("data/nanoaod_1M.ttree.root"));
  auto tree = std::unique_ptr<TTree>(file->Get<TTree>("Events"));
  auto treeStats = std::make_unique<TTreePerfStats>("ioperf", tree.get());
  ROOT::RDataFrame df(*tree);
  auto h =
      df.Filter([](unsigned int n) { return n > 0; }, {"nJet"}, "At least one jet")
        .Define("goodJet_ptcut", [](Vec<float> pt) { return pt > 30; }, {"Jet_pt"})
        .Define("goodJet_antiMuon", find_isolated_jets, {"Jet_eta", "Jet_phi", "Muon_pt", "Muon_eta", "Muon_phi"})
        .Define("goodJet_antiElectron", find_isolated_jets,
                {"Jet_eta", "Jet_phi", "Electron_pt", "Electron_eta", "Electron_phi"})
        .Define("goodJet",
                [](Vec<int> pt, Vec<int> muon, Vec<int> electron) {
                  return pt && muon && electron;
                },
                {"goodJet_ptcut", "goodJet_antiMuon", "goodJet_antiElectron"})
        .Filter([](Vec<int> good) { return Sum(good) > 0; }, {"goodJet"})
        .Define("goodJet_sumPt",
                [](Vec<int> good, Vec<float> pt) { return Sum(pt[good]); },
                {"goodJet", "Jet_pt"})
        .Histo1D<float>({"", ";Jet p_{T} sum (GeV);N_{Events}", 100, 15, 200}, "goodJet_sumPt");

  TCanvas c;
  h->Draw();
  c.SaveAs("7_rdataframe_compiled_nanoaod_ttree.png");
  treeStats->Print();
}

void rdataframe_rntuple() {
  ROOT::RDataFrame df = ROOT::RDF::Experimental::FromRNTuple("Events", "data/nanoaod_1M.rntuple.root");

  auto h =
      df.Filter([](unsigned int n) { return n > 0; }, {"nJet"}, "At least one jet")
        .Define("goodJet_ptcut", [](Vec<float> pt) { return pt > 30; }, {"Jet_pt"})
        .Define("goodJet_antiMuon", find_isolated_jets, {"Jet_eta", "Jet_phi", "Muon_pt", "Muon_eta", "Muon_phi"})
        .Define("goodJet_antiElectron", find_isolated_jets,
                {"Jet_eta", "Jet_phi", "Electron_pt", "Electron_eta", "Electron_phi"})
        .Define("goodJet",
                [](Vec<int> pt, Vec<int> muon, Vec<int> electron) {
                  return pt && muon && electron;
                },
                {"goodJet_ptcut", "goodJet_antiMuon", "goodJet_antiElectron"})
        .Filter([](Vec<int> good) { return Sum(good) > 0; }, {"goodJet"})
        .Define("goodJet_sumPt",
                [](Vec<int> good, Vec<float> pt) { return Sum(pt[good]); },
                {"goodJet", "Jet_pt"})
        .Histo1D<float>({"", ";Jet p_{T} sum (GeV);N_{Events}", 100, 15, 200}, "goodJet_sumPt");

  TCanvas c;
  h->Draw();
  c.SaveAs("7_rdataframe_compiled_nanoaod_rntuple.png");
}

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    std::cerr << "Please provide the data format ('ttree' or 'rntuple')" << std::endl;
    return 1;
  }

  auto verbosity =
      ROOT::Experimental::RLogScopedVerbosity(ROOT::Detail::RDF::RDFLogChannel(), ROOT::Experimental::ELogLevel::kInfo);

  std::string dataFormat = std::string(argv[1]);

  if (dataFormat == "ttree") {
    rdataframe_ttree();
  } else if (dataFormat == "rntuple") {
    rdataframe_rntuple();
  } else {
    std::cerr << "Invalid data format specified (use 'ttree' or 'rntuple')" << std::endl;
  }
  return 0;
}
