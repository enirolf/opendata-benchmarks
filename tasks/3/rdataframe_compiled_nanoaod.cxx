#include "ROOT/RDataFrame.hxx"
#include "ROOT/RLogger.hxx"
#include "ROOT/RNTupleDS.hxx"

#include "TCanvas.h"
#include "TString.h"

template <typename T> using Vec = const ROOT::RVec<T> &;

void rdataframe_ttree() {
  ROOT::RDataFrame df("Events", "data/nanoaod.ttree.root");

  auto goodJetPt = [](Vec<float> pt, Vec<float> eta) { return pt[abs(eta) < 1.0]; };

  auto h = df.Define("goodJet_pt", goodJetPt, {"Jet_pt", "Jet_eta"})
             .Histo1D<ROOT::RVec<float>>({"", ";Jet p_{T} (GeV);N_{Events}", 100, 15, 60}, "goodJet_pt");

  TCanvas c;
  h->Draw();
  c.SaveAs("3_rdataframe_compiled_nanoaod_ttree.png");
}

void rdataframe_rntuple() {
  ROOT::RDataFrame df = ROOT::RDF::Experimental::FromRNTuple("Events", "data/nanoaod.rntuple.root");

  auto goodJetPt = [](Vec<float> pt, Vec<float> eta) { return pt[abs(eta) < 1.0]; };

  auto h = df.Define("goodJet_pt", goodJetPt, {"Jet_pt", "Jet_eta"})
             .Histo1D<ROOT::RVec<float>>({"", ";Jet p_{T} (GeV);N_{Events}", 100, 15, 60}, "goodJet_pt");

  TCanvas c;
  h->Draw();
  c.SaveAs("3_rdataframe_compiled_nanoaod_rntuple.png");
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
