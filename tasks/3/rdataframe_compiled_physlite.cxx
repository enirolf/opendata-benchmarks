#include <iostream>
#include <string>

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RLogger.hxx"
#include "ROOT/RNTupleDS.hxx"
#include "TCanvas.h"
#include "TTreePerfStats.h"

template <typename T> using Vec = const ROOT::RVec<T>&;

void rdataframe_ttree() {
  auto file = std::unique_ptr<TFile>(TFile::Open("data/DAOD_PHYSLITE.ttree.root"));
  auto tree = std::unique_ptr<TTree>(file->Get<TTree>("CollectionTree"));
  auto treeStats = std::make_unique<TTreePerfStats>("ioperf", tree.get());
  ROOT::RDataFrame df(*tree);

  auto goodJetPt = [](Vec<float> pt, Vec<float> eta) { return pt[abs(eta) < 1.0] / 1000.f; };

  auto h = df.Define("goodJet_pt", goodJetPt, {"AnalysisJetsAuxDyn.pt", "AnalysisJetsAuxDyn.eta"})
             .Histo1D<ROOT::RVec<float>>({"", ";Jet p_{T} (GeV);N_{Events}", 100, 15, 60}, "goodJet_pt");

  TCanvas c;
  h->Draw();
  c.SaveAs("3_rdataframe_compiled_physlite_ttree.png");
  treeStats->Print();
}

void rdataframe_rntuple() {
  ROOT::RDataFrame df =
      ROOT::RDF::Experimental::FromRNTuple("CollectionTree", "data/DAOD_PHYSLITE.rntuple.root");

  auto goodJetPt = [](Vec<float> pt, Vec<float> eta) { return pt[abs(eta) < 1.0] / 1000.f; };

  auto h = df.Define("goodJet_pt", goodJetPt, {"AnalysisJetsAuxDyn_pt", "AnalysisJetsAuxDyn_eta"})
             .Histo1D<ROOT::RVec<float>>({"", ";Jet p_{T} (GeV);N_{Events}", 100, 15, 60}, "goodJet_pt");
  TCanvas c;
  h->Draw();
  c.SaveAs("3_rdataframe_compiled_physlite_rntuple.png");
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
