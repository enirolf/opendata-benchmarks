#include "Math/Vector4D.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RLogger.hxx"
#include "ROOT/RNTupleDS.hxx"

#include "TCanvas.h"
#include "TTreePerfStats.h"
#include "TString.h"

#include <unistd.h>

template <typename T> using Vec = const ROOT::RVec<T> &;
using ROOT::Math::XYZTVector;

ROOT::RVec<std::size_t> find_trijet(Vec<XYZTVector> jets) {
  constexpr std::size_t n = 3;
  float distance = 1e9;
  const auto top_mass = 172.5;
  std::size_t idx1 = 0, idx2 = 1, idx3 = 2;

  for (std::size_t i = 0; i <= jets.size() - n; i++) {
    auto p1 = jets[i];
    for (std::size_t j = i + 1; j <= jets.size() - n + 1; j++) {
      auto p2 = jets[j];
      for (std::size_t k = j + 1; k <= jets.size() - n + 2; k++) {
        auto p3 = jets[k];
        const auto tmp_mass = (p1 + p2 + p3).mass();
        const auto tmp_distance = std::abs(tmp_mass - top_mass);
        if (tmp_distance < distance) {
          distance = tmp_distance;
          idx1 = i;
          idx2 = j;
          idx3 = k;
        }
      }
    }
  }
  return {idx1, idx2, idx3};
}

float trijet_pt(Vec<float> pt, Vec<float> eta, Vec<float> phi, Vec<float> mass, Vec<std::size_t> idx) {
  auto p1 = ROOT::Math::PtEtaPhiMVector(pt[idx[0]], eta[idx[0]], phi[idx[0]], mass[idx[0]]);
  auto p2 = ROOT::Math::PtEtaPhiMVector(pt[idx[1]], eta[idx[1]], phi[idx[1]], mass[idx[1]]);
  auto p3 = ROOT::Math::PtEtaPhiMVector(pt[idx[2]], eta[idx[2]], phi[idx[2]], mass[idx[2]]);
  return (p1 + p2 + p3).pt();
}

void rdataframe_ttree(std::string_view fileName) {
  using ROOT::Math::PtEtaPhiMVector;
  using ROOT::VecOps::Construct;

  auto file = std::unique_ptr<TFile>(TFile::Open(std::string(fileName).c_str()));
  auto tree = std::unique_ptr<TTree>(file->Get<TTree>("Events"));
  auto treeStats = std::make_unique<TTreePerfStats>("ioperf", tree.get());
  ROOT::RDataFrame df(*tree);

  auto df2 = df.Filter([](unsigned int n) { return n >= 3; }, {"nJet"}, "At least three jets")
               .Define("JetXYZT",
                       [](Vec<float> pt, Vec<float> eta, Vec<float> phi, Vec<float> m) {
                         return Construct<XYZTVector>(Construct<PtEtaPhiMVector>(pt, eta, phi, m));
                       },
                       {"Jet_pt", "Jet_eta", "Jet_phi", "Jet_mass"})
               .Define("Trijet_idx", find_trijet, {"JetXYZT"});

  auto h1 = df2.Define("Trijet_pt", trijet_pt, {"Jet_pt", "Jet_eta", "Jet_phi", "Jet_mass", "Trijet_idx"})
               .Histo1D<float>({"", ";Trijet pt (GeV);N_{Events}", 100, 15, 40}, "Trijet_pt");

  auto h2 = df2.Define("Trijet_leadingBtag",
                      [](Vec<float> btag, Vec<size_t> idx) { return Max(Take(btag, idx)); },
                      {"Jet_btag", "Trijet_idx"})
               .Histo1D<float>({"", ";Trijet leading b-tag;N_{Events}", 100, 0, 1}, "Trijet_leadingBtag");

  TCanvas c;
  c.Divide(2, 1);
  c.cd(1);
  h1->Draw();
  c.cd(2);
  h2->Draw();
  c.SaveAs("6_rdataframe_compiled_ttree.png");
  treeStats->Print();
}

void rdataframe_rntuple(std::string_view fileName) {
  using ROOT::Math::PtEtaPhiMVector;
  using ROOT::VecOps::Construct;

  ROOT::RDataFrame df = ROOT::RDF::Experimental::FromRNTuple("Events", fileName);

  auto df2 = df.Filter([](unsigned int n) { return n >= 3; }, {"nJet"}, "At least three jets")
               .Define("JetXYZT",
                       [](Vec<float> pt, Vec<float> eta, Vec<float> phi, Vec<float> m) {
                         return Construct<XYZTVector>(Construct<PtEtaPhiMVector>(pt, eta, phi, m));
                       },
                       {"Jet_pt", "Jet_eta", "Jet_phi", "Jet_mass"})
               .Define("Trijet_idx", find_trijet, {"JetXYZT"});

  auto h1 = df2.Define("Trijet_pt", trijet_pt, {"Jet_pt", "Jet_eta", "Jet_phi", "Jet_mass", "Trijet_idx"})
               .Histo1D<float>({"", ";Trijet pt (GeV);N_{Events}", 100, 15, 40}, "Trijet_pt");

  auto h2 = df2.Define("Trijet_leadingBtag",
                      [](Vec<float> btag, Vec<size_t> idx) { return Max(Take(btag, idx)); },
                      {"Jet_btag", "Trijet_idx"})
               .Histo1D<float>({"", ";Trijet leading b-tag;N_{Events}", 100, 0, 1}, "Trijet_leadingBtag");

  TCanvas c;
  c.Divide(2, 1);
  c.cd(1);
  h1->Draw();
  c.cd(2);
  h2->Draw();
  c.SaveAs("6_rdataframe_compiled_rntuple.png");
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
