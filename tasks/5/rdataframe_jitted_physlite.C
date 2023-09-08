template <typename T> using Vec = const ROOT::RVec<T>&;
using FourVector = ROOT::Math::PtEtaPhiMVector;

auto compute_dimuon_masses(Vec<float> pt, Vec<float> eta, Vec<float> phi, Vec<float> charge)
{
    const float mass = 0.1056583755;
    ROOT::RVec<float> masses;
    const auto c = ROOT::VecOps::Combinations(pt, 2);
    for (auto i = 0u; i < c[0].size(); i++) {
        const auto i1 = c[0][i];
        const auto i2 = c[1][i];
        if (charge[i1] == charge[i2]) continue;
        const FourVector p1(pt[i1] / 1000., eta[i1], phi[i1], mass);
        const FourVector p2(pt[i2] / 1000., eta[i2], phi[i2], mass);
        masses.push_back((p1 + p2).mass());
    }
    return masses;
};


void rdataframe_jitted_ttree() {
    ROOT::RDataFrame df("CollectionTree", "root://eosuser.cern.ch//eos/user/f/fdegeus/adl-atlas/DAOD_PHYSLITE.ttree.root");
    auto df2 = df.Define("nMuon", [](Vec<float> pts){return pts.size();}, {"AnalysisMuonsAuxDyn.pt"})
                 .Filter("nMuon >= 2", "At least two muons")
                 .Define("Dimuon_mass", compute_dimuon_masses, {"AnalysisMuonsAuxDyn.pt", "AnalysisMuonsAuxDyn.eta", "AnalysisMuonsAuxDyn.phi", "AnalysisMuonsAuxDyn.charge"})
                 .Filter("Sum(Dimuon_mass > 60 && Dimuon_mass < 120) > 0",
                         "At least one dimuon system with mass in range [60, 120]");

    auto report = df2.Report();
    report->Print();

    auto h = df2.Define("MET", [](ROOT::RVec<float> sumet) { return sumet[sumet.size() - 1] / 1000.; }, { "MET_Core_AnalysisMETAuxDyn.sumet" })
                .Histo1D({"", ";MET (GeV);N_{Events}", 100, 0, 200}, "MET");

    TCanvas cTTree;
    h->Draw();
    cTTree.SaveAs("5_rdf_physlite_ttree_jitted.png");
}

void rdataframe_jitted_rntuple() {
    ROOT::RDataFrame df = ROOT::RDF::Experimental::FromRNTuple("CollectionTree", "root://eosuser.cern.ch//eos/user/f/fdegeus/adl-atlas/DAOD_PHYSLITE.rntuple.root");
    auto df2 = df.Define("nMuon", [](Vec<float> pts){return pts.size();}, {"AnalysisMuonsAuxDyn_pt"})
                 .Filter("nMuon >= 2", "At least two muons")
                 .Define("Dimuon_mass", compute_dimuon_masses, {"AnalysisMuonsAuxDyn_pt", "AnalysisMuonsAuxDyn_eta", "AnalysisMuonsAuxDyn_phi", "AnalysisMuonsAuxDyn_charge"})
                 .Filter("Sum(Dimuon_mass > 60 && Dimuon_mass < 120) > 0",
                         "At least one dimuon system with mass in range [60, 120]");

    auto report = df2.Report();
    report->Print();

    auto h = df2.Define("MET", [](ROOT::RVec<float> sumet) { return sumet[sumet.size() - 1] / 1000.; }, { "MET_Core_AnalysisMETAuxDyn_sumet" })
                .Histo1D({"", ";MET (GeV);N_{Events}", 100, 0, 200}, "MET");

    TCanvas cRNTuple;
    h->Draw();
    cRNTuple.SaveAs("5_rdf_physlite_rntuple_jitted.png");
}

void rdataframe_jitted_physlite() {
    // ROOT::EnableImplicitMT();

    rdataframe_jitted_ttree();
    rdataframe_jitted_rntuple();
}
