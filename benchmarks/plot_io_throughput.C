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

void plot_per_task(std::string_view dataset, int task, std::vector<float> ttreeThroughput, std::vector<float> rntupleThroughput) {
  RVec<float> ttreeThroughputVec(ttreeThroughput.begin(), ttreeThroughput.end());
  RVec<float> rntupleThroughputVec(rntupleThroughput.begin(), rntupleThroughput.end());

  std::cout << dataset << " task " << task << std::endl;
  std::cout << "\tTTree: " << Mean(ttreeThroughputVec) << " +/- " << StdErr(ttreeThroughputVec) << std::endl;
  std::cout << "\tRNTuple: " << Mean(rntupleThroughputVec) << " +/- " << StdErr(rntupleThroughputVec) << std::endl;

  std::map<std::string, TGraphErrors> throughputGraphs;
  float throughputMean, throughputErr;

  throughputGraphs["ttree"] = TGraphErrors();
  throughputGraphs["ttree"].SetPoint(0, 0.5, Mean(ttreeThroughputVec));
  throughputGraphs["ttree"].SetPoint(1, 2.0, -1);
  throughputGraphs["ttree"].SetPointError(0, 0, StdErr(ttreeThroughputVec));

  throughputGraphs["rntuple"] = TGraphErrors();
  throughputGraphs["rntuple"].SetPoint(0, 1.5, Mean(rntupleThroughputVec));
  throughputGraphs["rntuple"].SetPoint(1, 3.0, -1);
  throughputGraphs["rntuple"].SetPointError(0, 0, StdErr(rntupleThroughputVec));

  TCanvas canvas(Form("c_%d_%s", task, std::string(dataset).c_str()),
                 Form("c_%d_%s", task, std::string(dataset).c_str()), 1200, 1200);
  canvas.SetFillStyle(4000);
  canvas.cd();

  // TTree vs RNTuple read throughput speed
  TPad pad("pad", "pad", 0.0, 0.0, 1.0, 1.0);
  pad.SetTopMargin(0.12);
  pad.SetBottomMargin(0.08);
  pad.SetLeftMargin(0.18);
  pad.SetRightMargin(0.02);
  pad.SetFillStyle(4000);
  pad.SetFrameFillStyle(4000);
  pad.SetFillStyle(4000);
  pad.Draw();
  canvas.cd();

  float maxThroughput = Mean(rntupleThroughputVec) + StdErr(rntupleThroughputVec);

  TH1F helper("", "", 2, 0, 2);
  helper.GetXaxis()->SetTickSize(0);
  helper.GetXaxis()->SetNdivisions(4);
  helper.GetXaxis()->SetLabelOffset(0.02);
  helper.GetYaxis()->SetTickSize(0.01);
  helper.GetYaxis()->SetLabelSize(0.045);
  helper.GetYaxis()->SetTitle("MB / s");
  helper.GetYaxis()->SetTitleSize(0.05);
  helper.GetYaxis()->SetTitleOffset(1.8);
  helper.SetMinimum(0);
  helper.SetMaximum(maxThroughput * 1.2);

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
      y /= Mean(ttreeThroughputVec);

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
  if (dataset == "nanoaod")
    title.DrawLatexNDC(0.5, 0.975, Form("NanoAOD raw I/O throughput, task %d", task));
  else
    title.DrawLatexNDC(0.5, 0.975, Form("DAOD_PHYSLITE raw I/O throughput, task %d", task));

  canvas.Print(Form("results/io_throughput/%d_%s.pdf", task, std::string(dataset).c_str()));
  TFile f(Form("results/io_throughput/%d_%s.root", task, std::string(dataset).c_str()), "RECREATE");
  canvas.Write();
}


void plot_per_dataset(std::string_view dataset, std::map<std::string, std::array<std::vector<float>, 8>> data) {
  for (int i = 0; i < 8; ++i) {
    if (i == 5)
      continue; // skip task 6

    plot_per_task(dataset, i + 1, data["ttree"][i], data["rntuple"][i]);
  }
}

void plot_io_throughput() {
  gStyle->SetEndErrorSize(6);
  gStyle->SetOptTitle(1);
  gStyle->SetOptStat(0);

  std::map<std::string, std::map<std::string, std::array<std::vector<float>, 8>>> throughputData;

  std::string resultsFilePath = "results/read_rates.data";
  std::ifstream resultsFile(resultsFilePath);
  std::string dataset, format;
  int task;
  float throughput;

  while (resultsFile >> dataset >> format >> task >> throughput) {
      throughputData[dataset][format][task - 1].push_back(throughput);
  }

  for (const auto &[dataset, data] : throughputData) {
    plot_per_dataset(dataset, data);
  }
}
