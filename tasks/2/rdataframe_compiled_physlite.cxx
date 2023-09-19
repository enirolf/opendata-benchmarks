#include <iostream>
#include <string>

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RLogger.hxx"
#include "ROOT/RNTupleDS.hxx"
#include "TCanvas.h"

template <typename T> using Vec = const ROOT::RVec<T>&;

void rdataframe_ttree() {
  ROOT::RDataFrame df("CollectionTree", "data/data_run2/DAOD_PHYSLITE.ttree.root");

  auto h = df.Redefine("AnalysisJetsAuxDyn.pt", [](Vec<float> pts) { return pts / 1000.; }, {"AnalysisJetsAuxDyn.pt"})
             .Histo1D({"ttree", ";Jet p_{T} (GeV);N_{Events}", 100, 15, 60}, "AnalysisJetsAuxDyn.pt");

  TCanvas c;
  h->Draw();
  c.SaveAs("2_rdataframe_compiled_physlite_ttree.png");
}

void rdataframe_rntuple() {
  ROOT::RDataFrame df =
      ROOT::RDF::Experimental::FromRNTuple("CollectionTree", "data/data_run2/DAOD_PHYSLITE.rntuple.root");

  auto h = df.Redefine("AnalysisJetsAuxDyn_pt", [](Vec<float> pts) { return pts / 1000.; }, {"AnalysisJetsAuxDyn_pt"})
             .Histo1D({"rntuple", ";Jet p_{T} (GeV);N_{Events}", 100, 15, 60}, "AnalysisJetsAuxDyn_pt");

  TCanvas c;
  h->Draw();
  c.SaveAs("2_rdataframe_compiled_physlite_rntuple.png");
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
