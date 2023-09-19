#include "ROOT/RDataFrame.hxx"
#include "TCanvas.h"

template <typename T> using Vec = const ROOT::RVec<T> &;

void rdataframe() {
  ROOT::RDataFrame df("Events", "root://eospublic.cern.ch//eos/root-eos/benchmark/Run2012B_SingleMu.root");

  auto goodJetPt = [](Vec<float> pt, Vec<float> eta) { return pt[abs(eta) < 1.0]; };

  auto h = df.Define("goodJet_pt", goodJetPt, {"Jet_pt", "Jet_eta"})
             .Histo1D<ROOT::RVec<float>>({"", ";Jet p_{T} (GeV);N_{Events}", 100, 15, 60}, "goodJet_pt");

  TCanvas c;
  h->Draw();
  c.SaveAs("3_rdataframe_compiled_nanoaod.png");
}

int main() {
  rdataframe();
  return 0;
}
