#include <TCanvas.h>
#include <TPad.h>
#include <TH1F.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TText.h>

#include <ROOT/RVec.hxx>

using namespace ROOT::VecOps;

const int N_EVENTS = 100016;
const int N_TASKS = 8;

float StdErr(ROOT::VecOps::RVec<float> vals) {
  int nVal = vals.size();
  float mean = ROOT::VecOps::Mean(vals);
  float s2 = ROOT::VecOps::Var(vals);
  float s = sqrt(s2);
  float t = abs(ROOT::Math::tdistribution_quantile(0.05 / 2., nVal - 1));
  return t * s / sqrt(nVal);
}

void plot(int task = 1) {
  gStyle->SetEndErrorSize(6);
  gStyle->SetOptTitle(1);
  gStyle->SetOptStat(0);

  std::map<std::string, std::pair<float, float>> throughputData;
  std::map<std::string, TGraphErrors*> throughputGraphs;
  float throughputMean, throughputErr;

  for (const auto &format : {"ttree", "rntuple"}) {
    std::string resultsFilePath = "results/" + std::to_string(task) + "_" + format + "_jitted.data";

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
    throughputMean = N_EVENTS / runtimeMean;
    float throughputMax = N_EVENTS / (runtimeMean - runtimeErr);
    float throughputMin = N_EVENTS / (runtimeMean + runtimeErr);
    throughputErr = (throughputMax - throughputMin) / 2.;

    std::cout << "\tMean = " << throughputMean << "\tError = " << throughputMin << std::endl;

    throughputData[format] = std::pair(throughputMean, throughputErr);

    float x;
    if (format == std::string("ttree"))
      x = 0.5;
    else if (format == std::string("rntuple"))
      x = 1.5;

    throughputGraphs[format] = new TGraphErrors();
    throughputGraphs[format]->SetPoint(0, x + 0, throughputMean);
    throughputGraphs[format]->SetPoint(1, x + 1.5, -1);
    throughputGraphs[format]->SetPointError(0, 0, throughputErr);
  }

  TCanvas *canvas = new TCanvas("canvas_throughput", "canvas_throughput", 1200, 1200);
  canvas->SetFillStyle(4000);
  canvas->cd();

  // TTree vs RNTuple read throughput speed
  auto pad = new TPad("pad", "pad", 0.0, 0.0, 1.0, 1.0);
  pad->SetTopMargin(0.12);
  pad->SetBottomMargin(0.08);
  pad->SetLeftMargin(0.18);
  pad->SetRightMargin(0.02);
  pad->SetFillStyle(4000);
  pad->SetFrameFillStyle(4000);
  pad->SetFillStyle(4000);
  pad->Draw();
  canvas->cd();

  float maxThroughput = throughputData["rntuple"].first + throughputData["rntuple"].second;
  // int binStart = 6;
  // int binInterval = 10;

  TH1F *helper = new TH1F("", "", 2, 0, 2);
  helper->GetXaxis()->SetTickSize(0);
  helper->GetXaxis()->SetNdivisions(4);
  helper->GetXaxis()->SetLabelOffset(0.02);
  helper->GetYaxis()->SetTickSize(0.01);
  helper->GetYaxis()->SetLabelSize(0.045);
  helper->GetYaxis()->SetTitle("Events / s");
  helper->GetYaxis()->SetTitleSize(0.05);
  helper->GetYaxis()->SetTitleOffset(1.8);
  helper->SetMinimum(0);
  helper->SetMaximum(maxThroughput * 1.2);

  float labelSize = 0.05;

  for (int i = 0; i <= 5; i++) {
    if (i == 2) {
      helper->GetXaxis()->ChangeLabel(i, -1, labelSize, 21, -1, -1, "TTree");
    } else if (i == 4) {
      helper->GetXaxis()->ChangeLabel(i, -1, labelSize, 21, -1, -1, "RNTuple");
    } else {
      helper->GetXaxis()->ChangeLabel(i, -1, 0);
    }
  }

  pad->cd();
  gPad->SetGridy();
  helper->Draw();

  for (const auto &format : {"ttree", "rntuple"}) {
    double x, y;
    throughputGraphs[format]->GetPoint(0, x, y);

    throughputGraphs[format]->SetLineColor(12);
    throughputGraphs[format]->SetMarkerColor(12);
    if (format == std::string("ttree"))
      throughputGraphs[format]->SetFillColor(kBlue - 4);
    else
      throughputGraphs[format]->SetFillColor(kRed - 4);
    throughputGraphs[format]->SetLineWidth(2);
    throughputGraphs[format]->Draw("B1");
    throughputGraphs[format]->Draw("P");

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
  title.DrawLatexNDC(0.5, 0.975, Form("PHYSLITE ADL benchmark, task %d", task));

  canvas->Print(Form("results/task_%d.pdf", task));
}

void plot_results() {
  for (int i = 1; i <= 8; ++i) {
    plot(i);
  }
}
