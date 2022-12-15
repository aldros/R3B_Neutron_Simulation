TVector3 GetDetPosition(double Angle, double Z, double D, double HalfWidth)
{
    /*Z - lab Z coordinate in cm of 1st fiber from Daniels drawing
      Angle - turning angle in deg
      D - offset in cm relative to central trajectory (along Angle) 
      HlafWidth - halfwidth of the fiber detector
      */

    double Z0 = 277.7; //cm from target middle to the central turning point in GLAD
    double a = Angle * TMath::Pi()/180.; //central turning angle in rad

    double pos_X = (Z - Z0)*TMath::Sin(a)/TMath::Cos(a) - D/TMath::Cos(a) - HalfWidth*TMath::Cos(a); 
    double pos_Z = Z + HalfWidth*TMath::Sin(a);
    double pos_Y = 0;

    return (TVector3(pos_X*(-1.), pos_Y, pos_Z));

}


void run_sim_LAND()
{
    TString transport = "TGeant4";
    Bool_t userPList = kFALSE; // option for TGeant4
    TString outFile = "Exp_2022/sim.root";
    TString parFile = "Exp_2022/par.root";

    Bool_t magnet = kTRUE;
    //Double_t fieldScale = -1672.0/3584.; //   
    //Double_t fieldScale = -1.0; //max field
    Double_t fieldScale = -0.35; 
    //Double_t fieldScale = -0.75; //max field

    TString generator1 = "box";
    TString generator2 = "ascii";
    TString generator3 = "p2p";
    TString generator4 = "r3b";
    //TString generator = generator1;
    TString generator = generator2;
    //TString generator = generator3;
    TString inputFile = "test.txt";
    
    Int_t nEvents = 10000;
    Bool_t storeTrajectories = kTRUE;
    Int_t randomSeed = 335566; // 0 for time-dependent random numbers

    // Target type
    TString target1 = "LeadTarget";
    TString target2 = "Para";
    TString target3 = "Para45";
    TString target4 = "LiH";
    TString targetType = target4;

    // Stable part ------------------------------------------------------------

    TString dir = getenv("VMCWORKDIR");

    // ----    Debug option   -------------------------------------------------
    gDebug = 0;

    // -----   Timer   --------------------------------------------------------
    TStopwatch timer;
    timer.Start();

    // -----   Create simulation run   ----------------------------------------
    FairRunSim* run = new FairRunSim();
    run->SetName(transport);            // Transport engine
    run->SetOutputFile(outFile.Data()); // Output file
    FairRuntimeDb* rtdb = run->GetRuntimeDb();

    //  R3B Special Physics List in G4 case
    if ((userPList == kTRUE) && (transport.CompareTo("TGeant4") == 0))
    {
        run->SetUserConfig("g4R3bConfig.C");
        run->SetUserCuts("SetCuts.C");
    }

    run->SetUserCuts("SetCuts.C");

    // -----   Create media   -------------------------------------------------
    run->SetMaterials("media_r3b.geo"); // Materials

    // -----   Create R3B geometry --------------------------------------------
    // R3B Cave definition
    FairModule* cave = new R3BCave("CAVE");
    //cave->SetGeometryFileName("r3b_cave_vacuum.geo");
    cave->SetGeometryFileName("r3b_cave.geo");
    run->AddModule(cave);

    // Target
    //run->AddModule(new R3BTarget(targetType, "target_" + targetType + ".geo.root"));
    //run->AddModule(new R3BTarget(targetType, "targetvacuumchamber_" + targetType + ".geo.root"));
    //run->AddModule(new R3BTarget(targetType,"targetvacuumchamber_only.geo.root"));

    //FOOT
    R3BTra *FOOT = new R3BTra("foot_2022.geo.root");
    //run->AddModule(FOOT);

    //Target+FOOT
    //R3BTra* tra = new R3BTra("targetvacuumchamber_foot_LiH.geo.root");
    R3BTra* tra = new R3BTra("targetvacuumchamber_foot_LiH.geo.root", {0.,0.,-2.5});
    run->AddModule(tra);
    
    //Target+AMS Sofia
    //R3BTra* tra = new R3BTra("targetvacuumchamber_ams_s455.geo.root", { targetPar->GetPosX(), targetPar->GetPosY(), targetPar->GetPosZ(), rtarget });
    //R3BTra* tra = new R3BTra("targetvacuumchamber_ams_s455.geo.root", {0.,0.,-0.75});
    //R3BTra* tra = new R3BTra("targetvacuumchamber_only.geo.root", {0.,0.,-0.75});
    //R3BTra* tra = new R3BTra("targetvacuumchamber_foot_2022.geo.root", { 0., 0., -0.75 });
    //tra->SetEnergyCut(1e-6); // 1 keV
    //run->AddModule(tra);
    
    //AMS
    R3BTra *AMS = new R3BTra("ams_s444.geo.root");
    //run->AddModule(AMS);
    
    // GLAD
    //run->AddModule(new R3BGladMagnet("glad_v17_flange.geo.root")); // GLAD should not be moved or rotated

    // Fi0adetector
    double angle = 14.;//GLAD rotation angle

    //Full Neuland
    //run->AddModule(new R3BNeuland("neuland_v12a_14m.geo.root", { 0., 0., 14. * 100. + 12 * 5. }));
    //run->AddModule(new R3BLand("neuland_v12a_14m.geo.root", { 0., 0., 14. * 100. + 12 * 5. }));
    //run->AddModule(new R3BNeuland("neuland_v3_13dp.geo.root", { 0., 0., 1520. + 13 * 5. }));
    //run->AddModule(new R3BNeuland("neuland_v3_13dp.geo.root", { 0., 0., 1500. + 13 * 5. }));
    run->AddModule(new R3BLand("land_v12a_10m.geo.root", { 0., 0., 1500. + 13 * 5. }));
    //1 double plane of Neuland for protons
    //TVector3 NL_pos = GetDetPosition(20, 700, -150., 0.);
    //run->AddModule(new R3BNeuland("neuland_1dp.geo.root",{NL_pos.X(), NL_pos.Y(),NL_pos.Z()}, {"" ,-90.0,45.,90.0}));

    // Fi10 detector
    TVector3 fi10pos = GetDetPosition(angle, 560, 0., 0.);
    //run->AddModule(new R3BFi11("double_big_fi11_3m.geo.root", {fi10pos.X(), fi10pos.Y(),fi10pos.Z()}, {"" ,90.0,90.0-angle,0.0}));

    //TOFD for fragments
    TVector3 tofd_pos_frag = GetDetPosition(angle, 1400, 0., 0.);
    run->AddModule(new R3BTofd("tofd_s454.geo.root", {tofd_pos_frag.X(), tofd_pos_frag.Y(), tofd_pos_frag.Z()}, {"" ,-90.0, angle, 90.0}));

    R3BCalifa* califa = new R3BCalifa("califa_2020.geo.root");
    califa->SelectGeometryVersion(2020);
    //run->AddModule(califa);
    
    //TOFD for protons
    //TVector3 tofd_pos_prot = GetDetPosition(20., 660, -120., 0.);
    //run->AddModule(new R3BTofd("tofd_s454.geo.root", {tofd_pos_prot.X(), tofd_pos_prot.Y(), tofd_pos_prot.Z()}, {"" ,-90.0, 20., 90.0}));

    // Big Fi11 detector made of vacuum 
    // TVector3 fi11pos = GetDetPosition(angle, 595.6, -1.8, -512.*0.05);
    // run->AddModule(new R3BFi11("big_fi11.geo.root", {fi11pos.X(), fi11pos.Y(), fi11pos.Z()}, {"" ,90.0,180+90.0-angle,0.0}));


    // -----   Create R3B  magnetic field ----------------------------------------
    // NB: <D.B>
    // If the Global Position of the Magnet is changed
    // the Field Map has to be transformed accordingly
    R3BGladFieldMap* magField = new R3BGladFieldMap("R3BGladMap");
    magField->SetScale(fieldScale);

    if (magnet == kTRUE)
    {
        run->SetField(magField);
    }
    else
    {
        run->SetField(NULL);
    }

    // -----   Create PrimaryGenerator   --------------------------------------
    // 1 - Create the Main API class for the Generator
    FairPrimaryGenerator* primGen = new FairPrimaryGenerator();

    if (generator.CompareTo("box") == 0){
      // 2- Define the BOX generator
      // Int_t pdgId = 2212;     // proton beam
      // Double32_t theta1 = 0.; // polar angle distribution
      // Double32_t theta2 = 180.;
      // Double32_t momentum = 1.5;
      // FairBoxGenerator* boxGen = new FairBoxGenerator(pdgId, 3);
      // boxGen->SetThetaRange(theta1, theta2);
      // boxGen->SetPRange(momentum, momentum * 1.2);
      // boxGen->SetPhiRange(0, 360);
      // boxGen->SetXYZ(0.0, 0.0, -1.5);
      // primGen->AddGenerator(boxGen);

      // // 128-Sn fragment
      // R3BIonGenerator* ionGen = new R3BIonGenerator(50, 128, 50, 10, 1.3);
      // ionGen->Beam.SetVertexDistribution(R3BDistribution3D::Prism(R3BDistribution2D::Circle({ 0., 0. }, 0.1), R3BDistribution1D::Delta(-300)));
      // primGen->AddGenerator(ionGen);

      // neutrons
      // FairBoxGenerator* boxGen_n = new FairBoxGenerator(2112, 3);
      // boxGen_n->SetThetaRange(theta1, theta2);
      // boxGen_n->SetPRange(momentum, momentum * 1.2);
      // boxGen_n->SetPhiRange(0, 360);
      // boxGen_n->SetXYZ(0.0, 0.0, -1.5);
      // primGen->AddGenerator(boxGen_n);

      R3Bp2pGenerator* gen = new R3Bp2pGenerator("/local/home/ar245054/FairSimulation/R3BRoot/macros/r3b/Input/quasi_p2p_12C.txt");
      primGen->AddGenerator(gen);
    }
    
    if (generator.CompareTo("ascii") == 0)
    {
      //R3BAsciiGenerator* gen = new R3BAsciiGenerator((dir + "/input/" + inputFile).Data());

      //R3BAsciiGenerator* gen = new R3BAsciiGenerator("/local/home/ar245054/FairSimulation/R3BRoot/macros/r3b/Input/Cocktail_12C_1p9GeV_prec0-0.5_new_combined.txt");
      //R3BAsciiGenerator* gen = new R3BAsciiGenerator("/local/home/ar245054/FairSimulation/R3BRoot/macros/r3b/Input/Cocktail_16C_1p25GeV_prec0-0.5_new_combined.txt");
      //R3BAsciiGenerator* gen = new R3BAsciiGenerator("/local/home/ar245054/FairSimulation/R3BRoot/macros/r3b/Input/quasi_ascii_p2p_test.out");

      //Anna SRC
      //R3BAsciiGenerator* gen = new R3BAsciiGenerator("/local/home/ar245054/FairSimulation/R3BRoot/macros/r3b/Input/quasi_ascii_12Cppn_1.25GeV.out");
      //R3BAsciiGenerator* gen = new R3BAsciiGenerator("/local/home/ar245054/FairSimulation/R3BRoot/macros/r3b/Input/quasi_ascii_12Cppn_1.25GeV_KinematicCut.out");
      //R3BAsciiGenerator* gen = new R3BAsciiGenerator("/local/home/ar245054/FairSimulation/R3BRoot/macros/r3b/Input/quasi_ascii_12Cppn_1.25GeV_exp.out");
      //R3BAsciiGenerator* gen = new R3BAsciiGenerator("/local/home/ar245054/FairSimulation/R3BRoot/macros/r3b/Input/quasi_ascii_12Cppn_1.25GeV_uniform.out");
      //R3BAsciiGenerator* gen = new R3BAsciiGenerator("/local/home/ar245054/FairSimulation/R3BRoot/macros/r3b/Input/quasi_12C_1.25GeV.out");
      //R3BAsciiGenerator* gen = new R3BAsciiGenerator("/local/home/ar245054/FairSimulation/R3BRoot/macros/r3b/Input/quasi_12C_1.25GeV_AllInOne.out");

      //Outgoing fragment Anna
      //R3BAsciiGenerator* gen = new R3BAsciiGenerator("/local/home/ar245054/FairSimulation/R3BRoot/macros/r3b/Input/12C_4x100ev.txt");

      //Neutron Decay
      R3BAsciiGenerator* gen = new R3BAsciiGenerator("/local/home/ar245054/FairSimulation/R3BRoot/macros/r3b/Input/test.out");
      
      //Olivier
      //R3BAsciiGenerator* gen = new R3BAsciiGenerator("/local/home/ar245054/FairSimulation/R3BRoot/macros/r3b/Input/quasi_ascii_18CP2P17B_550MeV.out");
      
      primGen->AddGenerator(gen);
    }

    // if(generator.CompareTo("p2p") == 0){
    //   R3Bp2pGenerator* gen = new R3Bp2pGenerator("/local/home/ar245054/FairSimulation/R3BRoot/macros/r3b/Input/quasi_p2p_12C_ascii.txt");
    //   primGen->AddGenerator(gen);
    // }
    
    run->SetGenerator(primGen);

    run->SetStoreTraj(storeTrajectories);

    FairLogger::GetLogger()->SetLogVerbosityLevel("LOW");
    FairLogger::GetLogger()->SetLogScreenLevel("ERROR");

    // -----   Initialize simulation run   ------------------------------------
    run->Init();
    TVirtualMC::GetMC()->SetRandom(new TRandom3(randomSeed));

    // ------  Increase nb of step for CALO
    Int_t nSteps = -15000;
    TVirtualMC::GetMC()->SetMaxNStep(nSteps);

    // -----   Runtime database   ---------------------------------------------
    R3BFieldPar* fieldPar = (R3BFieldPar*)rtdb->getContainer("R3BFieldPar");
    if (NULL != magField)
    {
        fieldPar->SetParameters(magField);
        fieldPar->setChanged();
    }
    Bool_t kParameterMerged = kTRUE;
    FairParRootFileIo* parOut = new FairParRootFileIo(kParameterMerged);
    parOut->open(parFile.Data());
    rtdb->setOutput(parOut);
    rtdb->saveOutput();
    rtdb->print();

    // -----   Start run   ----------------------------------------------------
    if (nEvents > 0)
    {
        run->Run(nEvents);
    }

    // -----   Finish   -------------------------------------------------------
    timer.Stop();
    Double_t rtime = timer.RealTime();
    Double_t ctime = timer.CpuTime();
    cout << endl << endl;
    cout << "Macro finished succesfully." << endl;
    cout << "Output file is " << outFile << endl;
    cout << "Parameter file is " << parFile << endl;
    cout << "Real time " << rtime << " s, CPU time " << ctime << "s" << endl << endl;

    cout << " Test passed" << endl;
    cout << " All ok " << endl;
    cout << dir  << endl;
    // Snap a picture of the geometry
    // If this crashes, set "OpenGL.SavePicturesViaFBO: no" in your .rootrc
    /*gStyle->SetCanvasPreferGL(kTRUE);
      gGeoManager->GetTopVolume()->Draw("ogl");
      TGLViewer* v = (TGLViewer*)gPad->GetViewer3D();
      v->SetStyle(TGLRnrCtx::kOutline);
      v->RequestDraw();
      v->SavePicture("run_sim-side.png");
      v->SetPerspectiveCamera(TGLViewer::kCameraPerspXOZ, 25., 0, 0, -90. * TMath::DegToRad(), 0. * TMath::DegToRad());
      v->SavePicture("run_sim-top.png");*/
}
