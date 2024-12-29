#include <TSystem.h>
#include <EvtFilter.h>


R__LOAD_LIBRARY(libinterface_main)
R__LOAD_LIBRARY(libfun4all)
R__LOAD_LIBRARY(libg4detectors)
R__LOAD_LIBRARY(libg4eval)
R__LOAD_LIBRARY(libcalibrator)
R__LOAD_LIBRARY(libktracker)
R__LOAD_LIBRARY(libevt_filter)
/*
This macro takes severl external input files to run:
1. geom.root is the standard geometry dump from running the Fun4Sim macro;
2. `DST_in` (data.root) should be the DST file generated either by decoder or by simulation.

This is an example script intended to demonstrate how to run SQReco in a minimalistic fashion, it is NOT
suitable for production use and users should develop their own reconstruction macro for their own analysis.
*/

int RecoE1039DataKMagOn(const int run_id, const std::string DST_in="data.root", const int nEvents = 0)
//int RecoE1039DataKMagOn(const std::string DST_in="data.root")
{
  const float fac = 1.0;

  const bool legacy_rec_container = true;
  const double FMAGSTR = -1.044;
  const double KMAGSTR = -1.025;

  recoConsts* rc = recoConsts::instance();
  rc->set_IntFlag("RUNNUMBER", run_id);
  rc->set_DoubleFlag("FMAGSTR", FMAGSTR);
  rc->set_DoubleFlag("KMAGSTR", KMAGSTR);
  rc->set_BoolFlag("COARSE_MODE", false);
  rc->set_BoolFlag("REQUIRE_MUID", false);
  rc->set_CharFlag("HIT_MASK_MODE", "X");

  rc->set_CharFlag("AlignmentMille", "alignment/align_mille_v10_a.txt"); // Using custom alignment according to the residue plots
  rc->set_CharFlag("AlignmentHodo", "$E1039_RESOURCE/alignment/dummy/alignment_hodo.txt");
  rc->set_CharFlag("AlignmentProp", "$E1039_RESOURCE/alignment/dummy/alignment_prop.txt");
  rc->set_CharFlag("Calibration", "$E1039_RESOURCE/alignment/dummy/calibration.txt");

  rc->set_IntFlag("MaxHitsDC0" , int(350/fac)); // number of hits per chamber. Suggestion make it half
  rc->set_IntFlag("MaxHitsDC1" , int(350/fac));
  rc->set_IntFlag("MaxHitsDC2" , int(170/fac));
  rc->set_IntFlag("MaxHitsDC3p", int(140/fac));
  rc->set_IntFlag("MaxHitsDC3m", int(140/fac)); // This is the end line to set occupancy. Talk to Kun

  /***************** Widening chamber window ************/
  rc->set_DoubleFlag("RejectWinDC0", 0.3);
  rc->set_DoubleFlag("RejectWinDC1", 0.5);
  rc->set_DoubleFlag("RejectWinDC2", 0.35);
  rc->set_DoubleFlag("RejectWinDC3p", 0.24);
  rc->set_DoubleFlag("RejectWinDC3m", 0.24);
  /******************************************************/

  rc->Print();

  Fun4AllServer* se = Fun4AllServer::instance();
  se->Verbosity(0);

  // Calibrator
  CalibHitElementPos* cal_ele_pos = new CalibHitElementPos();
  cal_ele_pos->CalibTriggerHit(false);
  se->registerSubsystem(cal_ele_pos);

  //GeomSvc* geom_svc = GeomSvc::instance();

  CalibDriftDist* cal_dd = new CalibDriftDist();
  se->registerSubsystem(cal_dd);

  /*********** Implementing Event Filter *************/
  //int event_id = 13080;
  // Event Filter
  //EvtFilter *evt_filter = new EvtFilter();
  //evt_filter->Verbosity(0);
  //evt_filter->set_event_id_req(event_id);
  //se->registerSubsystem(evt_filter);
  /*********** Implementing Event Filter *************/

  SQReco* reco = new SQReco();
  //reco->Verbosity(100);
  reco->Verbosity(0);
  reco->set_legacy_rec_container(legacy_rec_container);
  //reco->set_geom_file_name((string)gSystem->Getenv("E1039_RESOURCE") + "/geometry/geom_run005433.root");
  reco->set_geom_file_name("geom_run005433.root");
  //reco->set_geom_file_name("geom.root");
  reco->set_enable_KF(true); //Kalman filter not needed for the track finding, disabling KF saves a lot of initialization time
  reco->setInputTy(SQReco::E1039);    //options are SQReco::E906 and SQReco::E1039
  reco->setFitterTy(SQReco::KFREF);  //not relavant for the track finding
  reco->set_evt_reducer_opt("none"); //if not provided, event reducer will be using JobOptsSvc to intialize; to turn off, set it to "none"

  reco->set_enable_eval_dst(true); // Added to avoid processing errors
    //reco->add_eval_list(0);    //output of cosmic reco is contained in the eval output for now
    //reco->add_eval_list(1);    //output of cosmic reco is contained in the eval output for now
    //reco->add_eval_list(2);    //output of cosmic reco is contained in the eval output for now
  reco->add_eval_list(3);    //output of cosmic reco is contained in the eval output for now


  reco->set_enable_eval(true);
  reco->set_eval_file_name("eval.root");

  se->registerSubsystem(reco);
  //VertexFit* vtx_fit = new VertexFit();
  //se->registerSubsystem(vtx_fit);

  SQVertexing* vtx = new SQVertexing();
  //vtx->Verbosity(21);
  vtx->set_legacy_rec_container(true);
  //vtx->set_single_retracking(true);
  se->registerSubsystem(vtx);

  Fun4AllInputManager* in = new Fun4AllDstInputManager("DSTIN");
  in->Verbosity(0);
  in->fileopen(DST_in.c_str());
  se->registerInputManager(in);

  Fun4AllDstOutputManager* out = new Fun4AllDstOutputManager("DSTOUT", "DST.root");
  se->registerOutputManager(out);

  se->run(nEvents);

  // finish job - close and save output files
  se->End();
  se->PrintTimer();
  printf("\nAll done");

  delete se;

  printf("\n0");
  gSystem->Exit(0);
  
  return 0;

}
