#include <iostream>
#include <string>

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RLogger.hxx"
#include "ROOT/RNTupleDS.hxx"
#include "TCanvas.h"

template <typename T> using Vec = const ROOT::RVec<T>&;

void rdataframe_ttree() {
  ROOT::RDataFrame df("CollectionTree", "data/data_run2/DAOD_PHYSLITE.ttree.root");

  auto h = df.Define("MET_pt", [](Vec<float> sumet) { return sumet[sumet.size() - 1] / 1000.; },
                     {"MET_Core_AnalysisMETAuxDyn.sumet"})
             .Histo1D({"", ";MET (GeV);N_{Events}", 100, 0, 200}, "MET_pt");

  TCanvas c;
  h->Draw();
  c.SaveAs("1_rdataframe_compiled_physlite_ttree.png");
}

void rdataframe_rntuple() {
  ROOT::RDataFrame df =
      ROOT::RDF::Experimental::FromRNTuple("CollectionTree", "data/data_run2/DAOD_PHYSLITE.rntuple.root");

  auto h = df.Define("MET_pt", [](Vec<float> sumet) { return sumet[sumet.size() - 1] / 1000.; },
                     {"MET_Core_AnalysisMETAuxDyn_sumet"})
             .Histo1D({"", ";MET (GeV);N_{Events}", 100, 0, 200}, "MET_pt");

  TCanvas c;
  h->Draw();
  c.SaveAs("1_rdataframe_compiled_physlite_rntuple.png");
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
