#include "ROOT/RDataFrame.hxx"
#include "ROOT/RLogger.hxx"
#include "ROOT/RNTupleDS.hxx"

#include "TCanvas.h"
#include "TTreePerfStats.h"
#include "TString.h"

#include <unistd.h>

void rdataframe_ttree(std::string_view fileName) {
  auto file = std::unique_ptr<TFile>(TFile::Open(std::string(fileName).c_str()));
  auto tree = std::unique_ptr<TTree>(file->Get<TTree>("Events"));
  auto treeStats = std::make_unique<TTreePerfStats>("ioperf", tree.get());
  ROOT::RDataFrame df(*tree);

  auto h = df.Histo1D<ROOT::RVec<float>>({"", ";Jet p_{T} (GeV);N_{Events}", 100, 15, 60}, "Jet_pt");

  TCanvas c;
  h->Draw();
  c.SaveAs("2_rdataframe_compiled_ttree.png");
  treeStats->Print();
}

void rdataframe_rntuple(std::string_view fileName) {
  ROOT::RDataFrame df = ROOT::RDF::Experimental::FromRNTuple("Events", fileName);

  auto h = df.Histo1D<ROOT::RVec<float>>({"", ";Jet p_{T} (GeV);N_{Events}", 100, 15, 60}, "Jet_pt");

  TCanvas c;
  h->Draw();
  c.SaveAs("2_rdataframe_compiled_rntuple.png");
}

[[noreturn]] static void usage(char *argv0) {
  printf("Usage: %s [-h] [-j nthreads] (ttree|rntuple) filename\n\n", argv0);
  printf("Options:\n");
  printf("  -h\t\t\tPrint this text\n");
  printf("  -j nthreads\t\tNumber of threads to use (default: 1)\n");
  exit(0);
}

int main(int argc, char *argv[]) {
  int nThreads = 1;
  int c;
  while ((c = getopt(argc, argv, "hj:")) != -1) {
    switch (c) {
    case 'j':
      nThreads = std::atoi(optarg);
      break;
    case 'h':
    default:
      usage(argv[0]);
    }
  }

  if ((argc - optind) != 2){
    usage(argv[0]);
    return 1;
  }

  std::string format = argv[optind];
  std::string fileName = argv[optind + 1];

  auto verbosity =
      ROOT::Experimental::RLogScopedVerbosity(ROOT::Detail::RDF::RDFLogChannel(), ROOT::Experimental::ELogLevel::kInfo);

  if (format == "ttree") {
    rdataframe_ttree(fileName);
  } else if (format == "rntuple") {
    rdataframe_rntuple(fileName);
  } else {
    std::cerr << "Invalid data format specified (use 'ttree' or 'rntuple')" << std::endl;
  }
  return 0;
}
