import subprocess
import datetime
import sys
import os
import ROOT

runsneeded = [6061, ]

def GetNFiles(runnum):
    """This function returns number of files saved in run_<RunNum> directory located in decoded area.

    Args:
        runnum (int): run number

    Returns:
        int: number of files
    """
    files = os.listdir('/data2/e1039/dst/'+'run_{:06d}'.format(runnum))
    return len(files)


def GetDecoRunList(deco_loc):
    """This function returns a list of decoded runs (sorted) after run=4454 and number of splits > 1

    Args:
        deco_loc (str): Where the decoded files located?

    Returns:
        list: list of run numbers (sorted)
    """
    rundirs = [d for d in os.listdir(deco_loc) if os.path.isdir(os.path.join(os.path.join(deco_loc, d))) and 
                                                                len(os.listdir(os.path.join(deco_loc, d))) > 1]
    #runnums = [int(r[6:11]) for r in rundirs if int(r[6:11]) > 4454]
    runnums = [int(r[6:11]) for r in rundirs if int(r[6:11]) in runsneeded]
    runnums.sort()
    return runnums

#def GetRecoRunList(rootfile):
#    """Get a list of reconstructed run list (sorted) using the root file.
#
#    Args:
#        rootfile (str): Absolute path of the root file
#
#    Returns:
#        list: reconstructed run list (sorted)
#    """
#    infile = ROOT.TFile.Open(rootfile, 'r')
#    recoBranch = infile.Get('recoBranch')
#    reco_runs = []
#    for event in recoBranch:
#        reco_runs.append(int(event.run))
#
#    reco_runs.sort()
#    return reco_runs

def GetTime():
    """This function returns date and time stamp using the format: %Y-%m-%d_at_%H-%M-%S_CST

    Returns:
        str: Date and time stamp.
    """
    ti = datetime.datetime.now()
    start_time = ti.strftime('%Y-%m-%d_at_%H-%M-%S_CST')
    return start_time

#def GetDecoRunList(deco_loc):
#    """This function returns a list of decoded runs (sorted) after run=4454 and number of splits > 1
#
#    Args:
#        deco_loc (str): Where the decoded files located?
#
#    Returns:
#        list: list of run numbers (sorted)
#    """
#    rundirs = [d for d in os.listdir(deco_loc) if os.path.isdir(os.path.join(os.path.join(deco_loc, d))) and 
#                                                                len(os.listdir(os.path.join(deco_loc, d))) > 1]
#    runnums = [int(r[6:11]) for r in rundirs if int(r[6:11]) > 5717]
#    runnums.sort()
#    return runnums

def GetDecodedSpillList(run):
    """This function returns list of spills (sorted) for the given run number.

    Args:
        run (int): run number
    """
    path = '/data2/e1039/dst/run_{:06d}/'.format(run)
    nfiles = os.listdir(path)
    nspills = [int(s[17:26]) for s in nfiles]
    nspills.sort()
    #print(nspills)
    #print(finalspills)
    return nspills

def IsFileReconstructed(run, spill):
    isNewReco = False
    isOldReco = False
    """
    This function checks whether the run and the spill is reconstructed recently.

    Args:
        run (int): run id      
        spill (int): spill id

        return (bool) : True  = Yes 
                        False = No
    """
    
    path      = '/pnfs/e1039/persistent/users/ckuruppu/cosmic_recodata/'
    runinfo   = os.path.join(path, 'run_{:06d}/run_{:06d}_spill_{:09d}_spin'.format(run, run, spill))
    full_path = '/pnfs/e1039/persistent/users/ckuruppu/cosmic_recodata/run_{:06d}/run_{:06d}_spill_{:09d}_spin/out/result.root'.format(run, run, spill)
    logfile   = '/pnfs/e1039/persistent/users/ckuruppu/cosmic_recodata/run_{:06d}/run_{:06d}_spill_{:09d}_spin/log/log.txt'.format(run, run, spill)

    if ((os.path.isdir(runinfo) == True) and 
        (os.path.isfile(full_path) == True) and 
        (GetExitStatus(logfile) == 0)):
        #print("Run: {:06d} Spill: {:09d} New Reco State: True".format(run, spill))
        isNewReco = True

    else:
        path = '/pnfs/e1039/persistent/cosmic_recodata/'
        runinfo = os.path.join(path, 'run_{:06d}/run_{:06d}_spill_{:09d}_spin'.format(run, run, spill))
        full_path = '/pnfs/e1039/persistent/users/spinquestpro/cosmic_recodata/run_{:06d}/run_{:06d}_spill_{:09d}_spin/out/result.root'.format(run, run, spill)
        if (os.path.isdir(runinfo) == True and 
            os.path.isfile(full_path) == True):
            #print("Run: {} Spill: {} Old Reco State: True".format(run, spill))
            isOldReco = True

        else:
            #print("No reconstruction information for Run: {} Spill: {}.!".format(run, spill))
            return False

    return (isNewReco or isOldReco)

def GetRecoRunList():
    runList = []
    fileRecoState = False
    runRecoStat = 0
    decoList = GetDecoRunList('/data2/e1039/dst/')
    for run in decoList:
        nRecoSpills = 0
        decoSpills = GetDecodedSpillList(run)
        nDecoSpills = len(decoSpills)
        for spill in decoSpills:
            if (IsFileReconstructed(run, spill)):
                #print("Run: {:06d} Spill: {:09d} reconstructed successfully.!".format(run, spill))
                nRecoSpills += 1

        if (nDecoSpills == nRecoSpills):
            #print("Run: {:06d} completely reconstructed.!".format(run))
            runRecoStat = 2

        elif ((nDecoSpills > nRecoSpills) and (nRecoSpills != 0)):
            #print("Run: {:06d} partially reconstructed.!".format(run))
            runRecoStat = 1

        elif (nRecoSpills == 0):
            #print("Run: {:06d} not being reconstructed.!".format(run))
            runRecoStat = 0
            runList.append(run)
    
    runList.sort()
    newlist = [r for r in runList if r in runsneeded]
    newlist.sort()
    #return runList
    return newlist

def GetExitStatus(filePath):
    """This function reads the log file in the filepath and checks the exit status at the bottom

    Args:
        filePath (str): full path to the log file
    """
    with open (filePath, 'r') as file:
        return int(file.readlines()[-1])


def main():
    """This is the main function that utilizes all the resources 
       and execute shell commands at the end.
    """
    print(GetTime())
    deco_loc = '/data2/e1039/dst/'
    reco_loc = '/pnfs/e1039/persistent/users/ckuruppu/cosmic_recodata/'
    tape_loc = '/pnfs/e1039/persistent/users/ckuruppu/tape_backed/decoded_data/'
    #abspath = os.getcwd()
    abspath = '/seaquest/users/ckuruppu/SemiOnlineReconstruction2/'
    
    print("Reconstructing RAW files has started on {}".format(GetTime()))
    print("Checking Reconstruction Status")
    #deco_runs = GetDecoRunList('/data2/e1039/dst/')
    #reco_runs = GetRecoRunList('/seaquest/users/spinquestpro/app/ProjectSpinQuest/Utilities/RunCosmicReco/CosmicStats.root')
    #new_runs = list(set(deco_runs) - set(reco_runs))
    #new_runs = GetRecoRunList()
    # K-Mag Off runs
    #new_runs = [6061, 6062,6064, 6065, 6066, 6067, 6068]
    # GoodRuns List
    new_runs = [5846, 5847, 5848, 5994, 5995, 6052, 6053, 6054, 6056, 6057, 6058, 6069, 6070, 6071, 6094, 6097, 6099, 6111, 6112, 6113, 6114, 6115, 6116, 6117, 6118, 6135, 6136, 6137, 6138, 6139, 6140, 6142, 6143, 6144, 6145, 6146, 6149, 6150, 6151, 6152, 6153, 6154, 6155]
    #new_runs = [5847,5848,5994,5995,6052,6053,6054,6056,6057,6058,6069,6070,6071,6094,6097,6099,6111,6112,6113,6114,6115,6116,6117,6118,6118]
    #new_runs = [5786]
    if (len(new_runs) == 0):
        print ("There is nothing to reconstruct on {} exitting.!".format(GetTime()))
        sys.exit(0)
    new_runs.sort()
    print("Running reconstruction over following new runs: {}".format(new_runs))

    for newrun in new_runs:
        nfiles = GetNFiles(newrun)
        
        #cmd = ['export', 'GROUP\=spinquest']
        #proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        #o, e = proc.communicate()
        #print('Output: ' + o.decode('ascii'))
        #if(len(e.decode('ascii'))) == 0:
        #    print("Error: "+"No Errors")
        #else:
        #    print('Error: '  + e.decode('ascii'))
        #print('Exit Status: ' + str(proc.returncode))

        
        print("Submitting a grid job for run: {} with the number of files: {} on {}".format(newrun, nfiles, GetTime()))
        deco_dir = os.path.join(deco_loc, 'run_{:06d}'.format(newrun))
        print("Copying decoded files to tape area {}".format(GetTime()))
        cmd = ['rsync', '-a', '-v', '-P', deco_dir, tape_loc]
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        o, e = proc.communicate()
        print('Output: ' + o.decode('ascii'))
        if(len(e.decode('ascii'))) == 0:
            print("Error: "+"No Errors")
        else:
            print('Error: '  + e.decode('ascii'))
        print('Exit Status: ' + str(proc.returncode))
        print(abspath)
        print("Initiating job submission on {}".format(GetTime()))
        cmd = [abspath+'/gridsub_data.sh', 'run_{:06d}'.format(newrun), '1', str(newrun), '0', 'splitting']
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        o, e = proc.communicate()
        print('Output: ' + o.decode('ascii'))
        if(len(e.decode('ascii'))) == 0:
            print("Error: "+"No Errors")
        else:
            print('Error: '  + e.decode('ascii'))
        print('Exit Status: ' + str(proc.returncode))

if __name__ == '__main__':
    main()
