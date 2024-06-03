#include <iostream>
#include <sys/stat.h> // For mkdir function
#include <TSystem.h>
#include <top/G4_InsensitiveVolumes.C>
#include <top/G4_SensitiveDetectors.C>

R__LOAD_LIBRARY(libinterface_main)
R__LOAD_LIBRARY(libfun4all)
R__LOAD_LIBRARY(libg4detectors)
R__LOAD_LIBRARY(libg4eval)
R__LOAD_LIBRARY(libcalibrator)
R__LOAD_LIBRARY(libktracker)

/*
This macro takes severl external input files to run:
1. geom.root is the standard geometry dump from running the Fun4Sim macro;
2. e906_run7.opts is provided
3. digit_028692_009.root is E906 run6 data, can be found at /pnfs/e906/production/digit/R009/02/86

This is an example script intended to demonstrate how to run SQReco in a minimalistic fashion, it is NOT
suitable for production use and users should develop their own reconstruction macro for their own analysis.
*/

int RecoE906Data(const int runID, const int spillID, std::string infile = "./fileset/digit_run_028694_spill_001415238.root", const float fac=5.0)
{
  //std::string dest_path = Format("/data4/e1039_data/semi_online_reco/run_%i/spill_%i/DST.root", runID, spillID);
 
 
  std::string dest_path = "/data4/e1039_data/semi_online_reco/run_" + std::to_string(runID) + "/spill_" + std::to_string(spillID) + "/";
  std::string outfile = dest_path + "DST.root";
  //std::string outfile = "Run_" + std::to_string(runID) + "_spill_" + std::to_string(spillID)+"_DST.root";
  /*int status = mkdir(dest_path.c_str(), 0777);
  if (status == 0) 
  {
    std::cout << "Directory created successfully.\n";
  } 
  else 
  {
    std::cerr << "Failed to create directory: " << "\n";
  }*/
  std::cout << dest_path << std::endl;
  recoConsts* rc = recoConsts::instance();
  rc->set_IntFlag("RUNNUMBER", 6); // To select the plane geometry for E906 Run 6.
  rc->set_DoubleFlag("FMAGSTR", -1.044); // -1.054;
  rc->set_DoubleFlag("KMAGSTR", -1.025); // -0.951;
  rc->set_CharFlag("TRIGGER_L1", "78");
  rc->set_DoubleFlag("RejectWinDC0" , 0.12);
  rc->set_DoubleFlag("RejectWinDC1" , 0.25);
  rc->set_DoubleFlag("RejectWinDC2" , 0.15);
  rc->set_DoubleFlag("RejectWinDC3p", 0.16);
  rc->set_DoubleFlag("RejectWinDC3m", 0.14);

  printf("fac: %f", fac);
  
  //printf("final: %i",int(350/fac)); // number of hits per chamber. Suggestion make it half
  //printf("final: %i",int(170/fac));
  //printf("final: %i",int(140/fac));
  //printf("final: %i",int(140/fac)); // This is the end line to set occupancy. Talk to Kun


  rc->set_IntFlag("MaxHitsDC0" , int(350/fac)); // number of hits per chamber. Suggestion make it half
  rc->set_IntFlag("MaxHitsDC1" , int(350/fac));
  rc->set_IntFlag("MaxHitsDC2" , int(170/fac));
  rc->set_IntFlag("MaxHitsDC3p", int(140/fac));
  rc->set_IntFlag("MaxHitsDC3m", int(140/fac)); // This is the end line to set occupancy. Talk to Kun
  
  //rc->set_IntFlag("MaxHitsDC0" , 7); // number of hits per chamber. Suggestion make it half/rc->Print();
  //rc->set_IntFlag("MaxHitsDC1" , 7);
  //rc->set_IntFlag("MaxHitsDC2" , 3);                                                        
  //rc->set_IntFlag("MaxHitsDC3p", 2);                                                        
  //rc->set_IntFlag("MaxHitsDC3m", 2); // This is the end line to set occupancy. Talk to Kun /rc->set_IntFlag("MaxHitsDC1" , 175);
  //rc->set_IntFlag("MaxHitsDC2" , 85);
  //rc->set_IntFlag("MaxHitsDC3p", 70);
  //rc->set_IntFlag("MaxHitsDC3m", 70);


  //rc->set_IntFlag("MaxHitsDC0" , 70); // number of hits per chamber. Suggestion make it half
  //rc->set_IntFlag("MaxHitsDC1" , 70);
  //rc->set_IntFlag("MaxHitsDC2" , 34);
  //rc->set_IntFlag("MaxHitsDC3p", 28);
  //rc->set_IntFlag("MaxHitsDC3m", 28); 
  rc->Print();

  Fun4AllServer* se = Fun4AllServer::instance();
  //se->Verbosity(100);
  ///
  /// Geometry.  You might create and use "geom.root" instead.
  ///
  PHG4Reco* g4reco = new PHG4Reco();
  g4reco->set_field_map();
  g4reco->SetWorldSizeX(1000);
  g4reco->SetWorldSizeY(1000);
  g4reco->SetWorldSizeZ(5000);
  g4reco->SetWorldShape("G4BOX");
  g4reco->SetWorldMaterial("G4_AIR");
  g4reco->SetPhysicsList("FTFP_BERT");
  SetupInsensitiveVolumes(g4reco);
  SetupSensitiveDetectors(g4reco);
  se->registerSubsystem(g4reco);

  ///
  /// Calibrator
  ///
  CalibHitElementPos* cal_ele_pos = new CalibHitElementPos();
  se->registerSubsystem(cal_ele_pos);

  CalibDriftDist* cal_drift_dist = new CalibDriftDist();
  cal_drift_dist->Verbosity(1);
  se->registerSubsystem(cal_drift_dist);

  ///
  /// Reconstruction
  ///
  SQReco* reco = new SQReco();
  //reco->Verbosity(100);
  //reco->set_geom_file_name("geom.root");
  reco->set_enable_KF(true); //Kalman filter not needed for the track finding, disabling KF saves a lot of initialization time
  reco->setInputTy(SQReco::E1039);    //options are SQReco::E906 and SQReco::E1039
  reco->setFitterTy(SQReco::KFREF);  //not relavant for the track finding
  reco->set_evt_reducer_opt("aoc"); //if not provided, event reducer will be using JobOptsSvc to intialize; to turn off, set it to "none"
  reco->set_enable_eval(true);
  reco->set_eval_file_name("eval.root");
  se->registerSubsystem(reco);

  VertexFit* vtx_fit = new VertexFit();
  se->registerSubsystem(vtx_fit);

  ///
  /// Input, output and run.
  ///
  Fun4AllSRawEventInputManager* in = new Fun4AllSRawEventInputManager("SRawEventIM");
  in->Verbosity(0);
  in->enable_E1039_translation();
  in->set_tree_name("save");
  in->set_branch_name("rawEvent");
  in->fileopen(infile);
  se->registerInputManager(in);

  Fun4AllDstOutputManager* out = new Fun4AllDstOutputManager("DSTOUT", outfile);
  se->registerOutputManager(out);
  // out->AddNode("SRawEvent");
  // out->AddNode("SRecEvent");

  //se->run(nEvents);
  se->run(0);
  PHGeomUtility::ExportGeomtry(se->topNode(), "geom.root");
  rc->WriteToFile("recoConsts.tsv");
  se->End();
  delete se;
  printf("Program ended successfully.!");
  printf("\n%d",0);
  gSystem->Exit(0);
  return 0;
}
