
#include "gcfg_ppp.h"
#include <chrono>
#include <thread>

using namespace std;
using namespace gnut;
using namespace gsins;
using namespace std::chrono;

void catch_signal(int) { cout << "Program interrupted by Ctrl-C [SIGINT,2]\n"; }

// MAIN
// ----------
int main(int argc, char** argv)
{
    // Only to cout the Reminder here
    signal(SIGINT, catch_signal);

    // Construct the gset class and init some values in the class
    t_gcfg_ppp gset;
    gset.app("GRAET-PVT", "$Ver: 1.1 $", "$Rev:  $", "(https://github.com/GREAT-WHU/GREAT-PVT)", __DATE__, __TIME__);

    // Get the arguments from the command line
    gset.arg(argc, argv, true, false);

    // Creat and set the log file : ppp.log
    auto log_type = dynamic_cast<t_gsetout *>(&gset)->log_type();
    auto log_level = dynamic_cast<t_gsetout *>(&gset)->log_level();
    auto log_name = dynamic_cast<t_gsetout *>(&gset)->log_name();
    auto log_pattern = dynamic_cast<t_gsetout *>(&gset)->log_pattern();
    spdlog::set_level(log_level);
    spdlog::set_pattern(log_pattern);
    spdlog::flush_on(spdlog::level::err);
    t_grtlog great_log = t_grtlog(log_type, log_level, log_name);
    auto my_logger = great_log.spdlog();

    // Check the base station
    bool isBase = dynamic_cast<t_gsetgen*>(&gset)->list_base().size();

    // Prepare site list from gset
    set<string> sites = dynamic_cast<t_gsetgen*>(&gset)->recs();

    // Prepare input files list from gset
    multimap<IFMT, string> inp = gset.inputs_all();

    // Get sample intval from gset. if not, init with the default value
    int sample = int(dynamic_cast<t_gsetgen*>(&gset)->sampling());
    if (!sample) sample = int(dynamic_cast<t_gsetgen*>(&gset)->sampling_default());

    //--- INITIALIZATIONS ---
    t_gdata* gdata = nullptr;
    t_gnavde* gde = new t_gnavde;
    t_gpoleut1* gerp = new t_gpoleut1;
    t_gallobs* gobs = new t_gallobs();  gobs->spdlog(my_logger); gobs->gset(&gset);
    t_gallprec* gorb = new t_gallprec(); gorb->spdlog(my_logger);
    t_gallpcv* gpcv = nullptr; if (gset.input_size("atx") > 0) { gpcv = new t_gallpcv;  gpcv->spdlog(my_logger); }
    t_gallotl* gotl = nullptr; if (gset.input_size("blq") > 0) { gotl = new t_gallotl;  gotl->spdlog(my_logger); }
    t_gallbias* gbia = nullptr; if (gset.input_size("biasinex") > 0 ||gset.input_size("bias") > 0) { gbia = new t_gallbias; gbia->spdlog(my_logger); }
    t_gallobj* gobj = new t_gallobj(my_logger, gpcv, gotl); gobj->spdlog(my_logger);
    t_gupd* gupd = nullptr; if (gset.input_size("upd") > 0) { gupd = new t_gupd;  gupd->spdlog(my_logger); }
    t_gifcb* gifcb = nullptr;  if (gset.input_size("ifcb") > 0) { gifcb = new t_gifcb;  gifcb->spdlog(my_logger); }

    // vgppp for the process of ppp with filter
    vector<t_gpvtflt*> vgpvt;

    // runepoch for the time costed each epoch 
    t_gtime runepoch(t_gtime::GPS);

    // lstepoch for the time of all epoches 
    t_gtime lstepoch(t_gtime::GPS);

    // check precision products
    if (gset.input_size("sp3") == 0 && gset.input_size("rinexc") == 0)
    {
        gorb->use_clknav(true);
        gorb->use_posnav(true);
    }
    else if (gset.input_size("sp3") > 0 && gset.input_size("rinexc") == 0)
    {
        gorb->use_clksp3(true);
    }

    // SET OBJECTS
    set<string>::const_iterator itOBJ;
    set<string> obj = dynamic_cast<t_gsetrec*>(&gset)->objects();
    for (itOBJ = obj.begin(); itOBJ != obj.end(); ++itOBJ) 
    {
        string name = *itOBJ;
        shared_ptr<t_grec> rec = dynamic_cast<t_gsetrec*>(&gset)->grec(name, my_logger);
        gobj->add(rec);
    }

    // Multi gcoder for multi-thread decoding data
    vector<t_gcoder*> gcoder;

    // Multi gior for multi-thread receiving data
    vector<t_gio*> gio;

    // multi-thread 
    vector<thread> gthread;

    t_gio* tgio = 0;
    t_gcoder* tgcoder = 0;

    if (!isBase)
    {
        // CHECK INPUTS, sp3+rinexc+rinexo, Necessary data
        if (gset.input_size("sp3") == 0 &&
            gset.input_size("rinexc") == 0 &&
            gset.input_size("rinexo") == 0
            ) 
        {
            SPDLOG_LOGGER_INFO(my_logger, "Error: incomplete input: rinexo + rinexc + sp3");
            gset.usage();
        }
    }

    // DATA READING
    multimap<IFMT, string>::const_iterator itINP = inp.begin();
    for (size_t i = 0; i < inp.size() && itINP != inp.end(); ++i, ++itINP)
    {
        // Get the file format/path, which will be used in decoder
        IFMT   ifmt(itINP->first);
        string path(itINP->second);
        string id("ID" + int2str(i));

        // For different file format, we prepare different data container and decoder for them.
        if (ifmt == IFMT::RINEXO_INP) { gdata = gobs; tgcoder = new t_rinexo(&gset, "", 4096); }
        else if (ifmt == IFMT::SP3_INP) { gdata = gorb; tgcoder = new t_sp3(&gset, "", 8172); }
        else if (ifmt == IFMT::RINEXC_INP) { gdata = gorb; tgcoder = new t_rinexc(&gset, "", 4096); }
        else if (ifmt == IFMT::RINEXN_INP) { gdata = gorb; tgcoder = new t_rinexn(&gset, "", 4096); }
        else if (ifmt == IFMT::ATX_INP) { gdata = gpcv; tgcoder = new t_atx(&gset, "", 4096); }
        else if (ifmt == IFMT::BLQ_INP) { gdata = gotl; tgcoder = new t_blq(&gset, "", 4096); }
        else if (ifmt == IFMT::UPD_INP) { gdata = gupd; tgcoder = new t_upd(&gset, "", 4096); }
        else if (ifmt == IFMT::BIASINEX_INP) { gdata = gbia; tgcoder = new t_biasinex(&gset, "", 20480); }
        else if (ifmt == IFMT::BIAS_INP) { gdata = gbia; tgcoder = new t_biabernese(&gset, "", 20480); }
        else if (ifmt == IFMT::DE_INP) { gdata = gde; tgcoder = new t_dvpteph405(&gset, "", 4096); }
        else if (ifmt == IFMT::EOP_INP) { gdata = gerp; tgcoder = new t_poleut1(&gset, "", 4096); }
        else if (ifmt == IFMT::IFCB_INP) { gdata = gifcb; tgcoder = new t_ifcb(&gset, "", 4096); }
        else 
        {
            SPDLOG_LOGGER_INFO(my_logger, "Error: unrecognized format " + int2str(int(ifmt)));
            gdata = 0;
        }

        // Check the file path
        if (path.substr(0, 7) == "file://") 
        {
            SPDLOG_LOGGER_INFO(my_logger, "path is file!");
            tgio = new t_gfile(my_logger);
            tgio->spdlog(my_logger);
            tgio->path(path);
        }

        // READ DATA FROM FILE
        if (tgcoder) 
        {
            // Put the file into gcoder
            tgcoder->clear();
            tgcoder->path(path);
            tgcoder->spdlog(my_logger);

            // Put the data container into gcoder
            tgcoder->add_data(id, gdata);
            tgcoder->add_data("OBJ", gobj); 

            // Put the gcoder into the gio
            // Note, gcoder contain the gdata and gio contain the gcoder
            tgio->coder(tgcoder);

            runepoch = t_gtime::current_time(t_gtime::GPS);

            // Read the data from file here
            tgio->run_read();
            lstepoch = t_gtime::current_time(t_gtime::GPS);

            // Write the information of reading process to log file
            SPDLOG_LOGGER_INFO(my_logger, "READ: " + path + " time: " + dbl2str(lstepoch.diff(runepoch)) + " sec");

            // Delete 
            delete tgio;
            delete tgcoder;

        }
    }
    // set antennas for satllites (must be before PCV assigning)
    t_gtime beg = dynamic_cast<t_gsetgen*>(&gset)->beg();
    gobj->read_satinfo(beg);

    // assigning PCV pointers to objects
    gobj->sync_pcvs();

    // add all data
    t_gallproc* data = new t_gallproc();
    if (gobs)data->Add_Data(t_gdata::type2str(gobs->id_type()), gobs);
    if (gorb)data->Add_Data(t_gdata::type2str(gorb->id_type()), gorb);
    if (gobj)data->Add_Data(t_gdata::type2str(gobj->id_type()), gobj);
    if (gbia)data->Add_Data(t_gdata::type2str(gbia->id_type()), gbia);
    if (gotl)data->Add_Data(t_gdata::type2str(gotl->id_type()), gotl);
    if (gde)data->Add_Data(t_gdata::type2str(gde->id_type()), gde);
    if (gerp)data->Add_Data(t_gdata::type2str(gerp->id_type()), gerp);
    if (gupd && dynamic_cast<t_gsetamb*>(&gset)->fix_mode() != FIX_MODE::NO && !isBase)
    {
        data->Add_Data(t_gdata::type2str(gupd->id_type()), gupd);
    }
    int frequency = dynamic_cast<t_gsetproc*>(&gset)->frequency();
    set<string> system = dynamic_cast<t_gsetgen*>(&gset)->sys();
    if (frequency >= 3 && system.find("GPS") != system.end() && !isBase)
    {
        if (gifcb)data->Add_Data(t_gdata::type2str(gifcb->id_type()), gifcb);
    }
    
    // Record current time
    auto tic_start = system_clock::now();

    // PVT PROCESSING - loop over sites from settings
    int i = 0;
    if (isBase)
    {
        vector<string> list_rover = gset.list_rover();  
        sites = set<string>(list_rover.begin(), list_rover.end());
    }
    int nsite = sites.size();
    set<string>::iterator it = sites.begin();
    while (i < nsite)
    {
        string site_base = "";
        string site = *it;
        // Check site data
        if (isBase)
        {
            site_base = (gset.list_base())[i];
            site = (gset.list_rover())[i];
            if (gobs->beg_obs(site_base) == LAST_TIME || gobs->end_obs(site_base) == FIRST_TIME ||
                 site_base.empty() || gobs->isSite(site_base) == false) 
            {
                SPDLOG_LOGGER_INFO(my_logger, "No two site/data for processing!");
                i++;
                continue;
            }
        }
        if (gobs->beg_obs(site) == LAST_TIME ||
            gobs->end_obs(site) == FIRST_TIME ||
            site.empty() || gobs->isSite(site) == false) 
        {
            SPDLOG_LOGGER_INFO(my_logger, "No site/data for processing!");
            if (!isBase) it++;
            i++;
            continue;
        }

        // Add site data
        vgpvt.push_back(0); int idx = vgpvt.size() - 1;
        vgpvt[idx] = new t_gpvtflt(site, site_base, &gset, my_logger, data);
        if (dynamic_cast<t_gsetamb*>(&gset)->fix_mode() != FIX_MODE::NO && !isBase)
        {
            vgpvt[idx]->Add_UPD(gupd);
        }

        t_gtime beg = dynamic_cast<t_gsetgen*>(&gset)->beg();
        t_gtime end = dynamic_cast<t_gsetgen*>(&gset)->end();
        SPDLOG_LOGGER_INFO(my_logger, "PVT processing started ");
        SPDLOG_LOGGER_INFO(my_logger, beg.str_ymdhms("  beg: ") + end.str_ymdhms("  end: "));

        runepoch = t_gtime::current_time(t_gtime::GPS);

        // The main processing code : processBatch
        vgpvt[idx]->processBatch(beg, end, true);

        // The time when process ends
        lstepoch = t_gtime::current_time(t_gtime::GPS);

        // Write the log file
        SPDLOG_LOGGER_INFO(my_logger, site_base + site + "PVT processing finished : duration  " + dbl2str(lstepoch.diff(runepoch)) + " sec");

        if (!isBase) it++;
        i++;
    }

    //Delete pointer
    for (size_t i = 0; i < gio.size(); ++i) { delete gio[i]; }; gio.clear();
    for (size_t i = 0; i < gcoder.size(); ++i) { delete gcoder[i]; }; gcoder.clear();
    for (unsigned int i = 0; i < vgpvt.size(); ++i) { if (vgpvt[i])  delete vgpvt[i]; }
    if (gobs) delete gobs;
    if (gpcv) delete gpcv;
    if (gotl) delete gotl;
    if (gobj) delete gobj;
    if (gorb) delete gorb;
    if (gbia) delete gbia;
    if (gde)  delete gde;
    if (gerp)  delete gerp;
    if (gupd)  delete gupd;
    if (gifcb)  delete gifcb;
    if (data)  delete data;

    // Record the current time and the time spent
    auto tic_end = system_clock::now();
    auto duration = duration_cast<microseconds>(tic_end - tic_start);
    cout << "Spent" << double(duration.count()) * microseconds::period::num / microseconds::period::den << " seconds." << endl;

    return 0;
}

