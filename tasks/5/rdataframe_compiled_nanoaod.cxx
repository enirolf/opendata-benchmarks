#include "Math/Vector4D.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RLogger.hxx"
#include "ROOT/RNTupleDS.hxx"

#include "TCanvas.h"
#include "TTreePerfStats.h"
#include "TString.h"

#include <unistd.h>

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

void rdataframe_ttree(std::string_view fileName) {
  auto file = std::unique_ptr<TFile>(TFile::Open(std::string(fileName).c_str()));
  auto tree = std::unique_ptr<TTree>(file->Get<TTree>("Events"));
  auto treeStats = std::make_unique<TTreePerfStats>("ioperf", tree.get());
  ROOT::RDataFrame df(*tree);

  auto h = df.Filter([](unsigned int n) { return n >= 2; }, {"nMuon"}, "At least two muons")
             .Define("Dimuon_mass", compute_dimuon_masses,
                     {"Muon_pt", "Muon_eta", "Muon_phi", "Muon_mass", "Muon_charge"})
             .Filter([](Vec<float> mass) { return Sum(mass > 60 && mass < 120) > 0; }, {"Dimuon_mass"},
                     "At least one dimuon system with mass in range [60, 120]")
             .Histo1D<float>({"", ";MET (GeV);N_{Events}", 100, 0, 200}, "MET_pt");

  TCanvas c;
  h->Draw();
  c.SaveAs("5_rdataframe_compiled_ttree.png");
  treeStats->Print();
}

void rdataframe_rntuple(std::string_view fileName) {
  ROOT::RDataFrame df = ROOT::RDF::Experimental::FromRNTuple("Events", fileName);

  auto h = df.Filter([](unsigned int n) { return n >= 2; }, {"nMuon"}, "At least two muons")
             .Define("Dimuon_mass", compute_dimuon_masses,
                     {"Muon_pt", "Muon_eta", "Muon_phi", "Muon_mass", "Muon_charge"})
             .Filter([](Vec<float> mass) { return Sum(mass > 60 && mass < 120) > 0; }, {"Dimuon_mass"},
                     "At least one dimuon system with mass in range [60, 120]")
             .Histo1D<float>({"", ";MET (GeV);N_{Events}", 100, 0, 200}, "MET_pt");

  TCanvas c;
  h->Draw();
  c.SaveAs("5_rdataframe_compiled_rntuple.png");
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
