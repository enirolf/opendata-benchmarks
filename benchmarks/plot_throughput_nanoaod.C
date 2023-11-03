#include <TCanvas.h>
#include <TPad.h>
#include <TH1F.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TText.h>

#include <ROOT/RVec.hxx>
#include <ROOT/RNTuple.hxx>

using namespace ROOT::VecOps;
using ROOT::Experimental::RNTupleReader;

float StdErr(ROOT::VecOps::RVec<float> vals) {
  int nVal = vals.size();
  float mean = ROOT::VecOps::Mean(vals);
  float s2 = ROOT::VecOps::Var(vals);
  float s = sqrt(s2);
  float t = abs(ROOT::Math::tdistribution_quantile(0.05 / 2., nVal - 1));
  return t * s / sqrt(nVal);
}

size_t getNEvents() {
  auto ntuple = RNTupleReader::Open("Events", "data/nanoaod_1M.rntuple.root");
  return ntuple->GetNEntries();
}

void plot(int task, std::string_view taskKind) {
  gStyle->SetEndErrorSize(6);
  gStyle->SetOptTitle(1);
  gStyle->SetOptStat(0);

  std::map<std::string, std::pair<float, float>> throughputData;
  std::map<std::string, TGraphErrors> throughputGraphs;
  float throughputMean, throughputErr;

  const float nEvents = getNEvents();
  std::cout << "** # events =  " << nEvents << std::endl;


  for (const auto &format : {"ttree", "rntuple"}) {
    std::string resultsFilePath = "results/nanoaod/" + std::to_string(task) + "_" + format + "_" + std::string(taskKind) + ".data";

    std::cout << "** Reading from " << resultsFilePath << std::endl;

    std::ifstream resultsFile(resultsFilePath);

    std::vector<float> runtimes;
    float taskRuntime;

    while (resultsFile >> taskRuntime) {
      runtimes.push_back(taskRuntime);
    }

    RVec<float> runtimeVec(runtimes.begin(), runtimes.end());

    float runtimeMean = Mean(runtimeVec);
    float runtimeErr  = StdErr(runtimeVec);
    throughputMean = nEvents / runtimeMean;
    float throughputMax = nEvents / (runtimeMean - runtimeErr);
    float throughputMin = nEvents / (runtimeMean + runtimeErr);
    throughputErr = (throughputMax - throughputMin) / 2.;

    std::cout << "\tMean = " << throughputMean << "\tError = " << throughputErr << std::endl;

    throughputData[format] = std::pair(throughputMean, throughputErr);

    float x;
    if (format == std::string("ttree"))
      x = 0.5;
    else if (format == std::string("rntuple"))
      x = 1.5;

    throughputGraphs[format] = TGraphErrors();
    throughputGraphs[format].SetPoint(0, x + 0, throughputMean);
    throughputGraphs[format].SetPoint(1, x + 1.5, -1);
    throughputGraphs[format].SetPointError(0, 0, throughputErr);
  }

  TCanvas canvas(Form("c_%d_throughput_%s", task, std::string(taskKind).c_str()),
                 Form("c_%d_throughput_%s", task, std::string(taskKind).c_str()), 1200, 1200);
  canvas.SetFillStyle(4000);
  canvas.cd();

  // TTree vs RNTuple read throughput speed
  TPad pad("pad", "pad", 0.0, 0.0, 1.0, 1.0);
  pad.SetTopMargin(0.12);
  pad.SetBottomMargin(0.08);
  pad.SetLeftMargin(0.12);
  pad.SetRightMargin(0.02);
  pad.SetFillStyle(4000);
  pad.SetFrameFillStyle(4000);
  pad.SetFillStyle(4000);
  pad.Draw();
  canvas.cd();

  float maxThroughput = throughputData["rntuple"].first + throughputData["rntuple"].second;

  TGaxis::SetMaxDigits(4);
  TH1F helper("", "", 2, 0, 2);
  helper.GetXaxis()->SetTickSize(0);
  helper.GetXaxis()->SetNdivisions(4);
  helper.GetXaxis()->SetLabelOffset(0.02);
  helper.GetYaxis()->SetTickSize(0.01);
  helper.GetYaxis()->SetLabelSize(0.045);
  helper.GetYaxis()->SetTitle("Events / s");
  helper.GetYaxis()->SetTitleSize(0.05);
  helper.GetYaxis()->SetTitleOffset(1.);
  helper.SetMinimum(0);
  helper.SetMaximum(12e6);

  float labelSize = 0.05;

  for (int i = 0; i <= 5; i++) {
    if (i == 2) {
      helper.GetXaxis()->ChangeLabel(i, -1, labelSize, 21, -1, -1, "TTree");
    } else if (i == 4) {
      helper.GetXaxis()->ChangeLabel(i, -1, labelSize, 21, -1, -1, "RNTuple");
    } else {
      helper.GetXaxis()->ChangeLabel(i, -1, 0);
    }
  }

  pad.cd();
  gPad->SetGridy();
  helper.Draw();

  for (const auto &format : {"ttree", "rntuple"}) {
    double x, y;
    throughputGraphs[format].GetPoint(0, x, y);

    throughputGraphs[format].SetLineColor(12);
    throughputGraphs[format].SetMarkerColor(12);
    if (format == std::string("ttree"))
      throughputGraphs[format].SetFillColor(kBlue - 4);
    else
      throughputGraphs[format].SetFillColor(kRed - 4);
    throughputGraphs[format].SetLineWidth(2);
    throughputGraphs[format].Draw("B1");
    throughputGraphs[format].Draw("P");

    if (format == std::string("rntuple")) {
      y /= throughputData["ttree"].first;

      std::ostringstream val;
      val.precision(1);
      val << "#times" << std::fixed << y;

      TLatex valLabel;
      valLabel.SetTextColor(kWhite);
      valLabel.SetTextSize(0.07);
      valLabel.SetTextAlign(21);
      valLabel.DrawLatex(x, maxThroughput * 0.025, val.str().c_str());
    }
  }

  TText annotation;
  annotation.SetTextSize(0.03);
  annotation.SetTextAlign(13);
  annotation.SetTextFont(42);
  annotation.DrawTextNDC(0.88, 0.91, "95% CL");

  TLatex title;
  title.SetTextSize(0.05);
  title.SetTextAlign(23);
  title.SetTextFont(42);
  title.DrawLatexNDC(0.5, 0.975, Form("NanoAOD ADL benchmark, task %d", task));

  canvas.Print(Form("results/nanoaod/%d_throughput_%s.pdf", task, std::string(taskKind).c_str()));
  TFile f(Form("results/nanoaod/%d_throughput_%s.root", task, std::string(taskKind).c_str()), "RECREATE");
  canvas.Write();
}

void plot_throughput_nanoaod() {
  for (int i = 1; i <= 8; ++i) {
    plot(i, "compiled");
    // plot(i, "jitted");
  }
}
