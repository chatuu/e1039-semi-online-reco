/**
 * @file RecoE1039DataKMagOn.C
 * @author Chatura Kuruppu (ckuruppu@fnal.gov)
 * @brief This script is made to run the reconstruction with K-Mag On
 * @version 0.1
 * @date 2024-09-26
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <iostream>
#include <ctime>
#include <TSystem.h>

R__LOAD_LIBRARY(libinterface_main)
R__LOAD_LIBRARY(libfun4all)
R__LOAD_LIBRARY(libg4detectors)
R__LOAD_LIBRARY(libg4eval)
R__LOAD_LIBRARY(libcalibrator)
R__LOAD_LIBRARY(libktracker)

/*
This macro takes severl external input files to run:
1. geom.root is the standard geometry dump from running the Fun4Sim macro;
2. `DST_in` (data.root) should be the DST file generated either by decoder or by simulation.

This is an example script intended to demonstrate how to run SQReco in a minimalistic fashion, it is NOT
suitable for production use and users should develop their own reconstruction macro for their own analysis.
*/


/**
 * @brief This function is created to run K-Mag on condition
 * 
 * @param runID const int run number
 * @param spillID const int spill number
 * @param infile  std::string input file with path
 * @param fac const float division factor for occupancy
 * @return int exit status. Must be 0 for proper reconstruction
 */
int RecoE1039DataKMagOn(const int runID, const int spillID, std::string infile = "./fileset/digit_run_028694_spill_001415238.root", const float fac=4.0)
{

  // Get the current time
  std::time_t currentTime = std::time(nullptr);

  // Convert the time to a string
  std::cout << "Start Time: " << std::asctime(std::localtime(&currentTime));
 
  std::string dest_path = "/data4/e1039_data/semi_online_reco/kmag-on/test/coarseFalse/run_" + std::to_string(runID) + "/spill_" + std::to_string(spillID) + "/";
  std::string outfile = dest_path + "DST.root";
  std::string evalloc = dest_path + "eval.root";
  std::string vtxevalloc = dest_path + "vtx_eval.root";
  const bool cosmic = false;

  const bool legacy_rec_container = true;
  const double FMAGSTR = -1.044;
  const double KMAGSTR = -1.025;

  recoConsts* rc = recoConsts::instance();
  rc->set_IntFlag("RUNNUMBER", runID);
  rc->set_DoubleFlag("FMAGSTR", FMAGSTR);
  rc->set_DoubleFlag("KMAGSTR", KMAGSTR);
  printf("fac value: %f", fac);
  
  //printf("final: %i",int(350/fac)); // number of hits per chamber. Suggestion make it half
  //printf("final: %i",int(170/fac));
  //printf("final: %i",int(140/fac));
  //printf("final: %i",int(140/fac)); // This is the end line to set occupancy. Talk to Kun

  // Defining Occupancy requirement. if fac==1, then Occupancy become the default occupancy used by SeaQuest experiment
  rc->set_IntFlag("MaxHitsDC0" , int(350/fac)); // number of hits per chamber. Suggestion make it half
  rc->set_IntFlag("MaxHitsDC1" , int(350/fac));
  rc->set_IntFlag("MaxHitsDC2" , int(170/fac));
  rc->set_IntFlag("MaxHitsDC3p", int(140/fac));
  rc->set_IntFlag("MaxHitsDC3m", int(140/fac)); // This is the end line to set occupancy. Talk to Kun
 
  if(cosmic)
  {
    rc->init("cosmic");
    rc->set_BoolFlag("COARSE_MODE", true);
    rc->set_DoubleFlag("KMAGSTR", 0.);
    rc->set_DoubleFlag("FMAGSTR", 0.);
  }

  rc->set_BoolFlag("COARSE_MODE", false);
  
  rc->set_CharFlag("AlignmentMille", "$E1039_RESOURCE2/dummy/align_mille.txt");
  rc->set_CharFlag("AlignmentHodo", "$E1039_RESOURCE2/dummy/alignment_hodo.txt");
  rc->set_CharFlag("AlignmentProp", "$E1039_RESOURCE2/dummy/alignment_prop.txt");
  rc->set_CharFlag("Calibration", "$E1039_RESOURCE2/dummy/calibration.txt");

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

  SQReco* reco = new SQReco();
  reco->Verbosity(0);
  reco->set_legacy_rec_container(legacy_rec_container);
  reco->set_geom_file_name((string)gSystem->Getenv("E1039_RESOURCE") + "/geometry/geom_run005433.root");
  reco->set_enable_KF(true); //Kalman filter not needed for the track finding, disabling KF saves a lot of initialization time
  reco->setInputTy(SQReco::E1039);    //options are SQReco::E906 and SQReco::E1039
  reco->setFitterTy(SQReco::KFREF);  //not relavant for the track finding
  reco->set_evt_reducer_opt("none"); //if not provided, event reducer will be using JobOptsSvc to intialize; to turn off, set it to "none"

  reco->set_enable_eval_dst(true); // Added to avoid processing errors
  reco->add_eval_list(3);

  reco->set_enable_eval(true);
  reco->set_eval_file_name(evalloc);
  if(cosmic)
  {
    reco->add_eval_list(0);    //output of cosmic reco is contained in the eval output for now
    reco->add_eval_list(1);    //output of cosmic reco is contained in the eval output for now
    reco->add_eval_list(2);    //output of cosmic reco is contained in the eval output for now
    reco->add_eval_list(3);    //output of cosmic reco is contained in the eval output for now

  }

  se->registerSubsystem(reco);

  VertexFit* vtx_fit = new VertexFit();
  vtx_fit->set_eval_file_name(vtxevalloc);
  se->registerSubsystem(vtx_fit);

  Fun4AllInputManager* in = new Fun4AllDstInputManager("DSTIN");
  in->Verbosity(0);
  in->fileopen(infile.c_str());
  se->registerInputManager(in);

  Fun4AllDstOutputManager* out = new Fun4AllDstOutputManager("DSTOUT", outfile);
  se->registerOutputManager(out);

  se->run(0);

  // finish job - close and save output files
  se->End();
  se->PrintTimer();

  delete se;
  // Get the current time
  std::time_t endTime = std::time(nullptr);

  // Convert the time to a string
  std::cout << "End Time: " << std::asctime(std::localtime(&endTime));
 
  printf("\nAll Done.!");
  printf("\n0");
  gSystem->Exit(0);
  return 0;
}
