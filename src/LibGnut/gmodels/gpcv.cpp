
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 * 
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.

-*/

#include <stdlib.h>
#include <iostream>
#include <iomanip>

#include "gmodels/gpcv.h"
#include "gutils/gsysconv.h"
#include "gutils/gtypeconv.h"
#include "gutils/gmatrixconv.h"
#include "gmodels/ginterp.h"
#include "gmodels/gpppmodel.h"

using namespace std;

namespace gnut
{
    t_gpcv::t_gpcv()
        : t_gdata(),
          _trans(true), // transmitter (default yes)
          _anten(""),   // antenna type
          _ident(""),   // antenna identification
          _svcod(""),   // SVN code
          _method(""),  // calibartion method
          _source(""),  // source of calibration
          _snxcod("")   // sinex code
    {
        _beg.tsys(t_gtime::GPS);
        _end.tsys(t_gtime::GPS);
        id_type(t_gdata::PCV);
        _pcv_noazi = false;
    }

    t_gpcv::t_gpcv(t_spdlog spdlog)
        : t_gdata(spdlog),
          _trans(true), // transmitter (default yes)
          _anten(""),   // antenna type
          _ident(""),   // antenna identification
          _svcod(""),   // SVN code
          _method(""),  // calibartion method
          _source(""),  // source of calibration
          _snxcod("")   // sinex code
    {
        _beg.tsys(t_gtime::GPS);
        _end.tsys(t_gtime::GPS);
        id_type(t_gdata::PCV);
        _pcv_noazi = false;
    }

    t_gpcv::~t_gpcv() {}


    double t_gpcv::pco(const double &zen, const double &azi, const GFRQ &f)
    {
        _gmutex.lock();

        double corr = 0.0;
        if (zen > G_PI)
        {
            ostringstream lg;
            lg << "not valid zenith angle:" << zen * R2D << endl;
            if (_spdlog)
                SPDLOG_LOGGER_ERROR(_spdlog, lg.str());
            _gmutex.unlock();
            return corr;
        }

        // satellite only PCO (Z-offset) mapped to rec-sat direction (approximated)
        if (_mappco.find(f) != _mappco.end())
        {
            corr = _mappco[f][2] * cos(zen); // for satellite zen should to be zen-alfa
        }

        _gmutex.unlock();
        return corr;
    }

    int t_gpcv::pcoS(t_gsatdata &satdata, t_gtriple &pco, OBSCOMBIN lc, GOBSBAND &b1, GOBSBAND &b2)
    {
        switch (lc)
        {
        case OBSCOMBIN::IONO_FREE:
            return pcoS_cmb(satdata, pco, b1, b2);
            break;
        case OBSCOMBIN::RAW_ALL:
        case OBSCOMBIN::RAW_MIX:
            return pcoS_raw(satdata, pco, b1);
            break;
        default:
            return -1;
            break;
        }
    }

    int t_gpcv::pcoR(t_gsatdata &satdata, t_gtriple &pco, OBSCOMBIN lc, GOBSBAND &b1, GOBSBAND &b2)
    {
        switch (lc)
        {
        case OBSCOMBIN::IONO_FREE:
            return pcoR_cmb(satdata, pco, b1, b2);
            break;
        case OBSCOMBIN::RAW_ALL:
        case OBSCOMBIN::RAW_MIX:
            return pcoR_raw(satdata, pco, b1);
            break;
        default:
            return -1;
            break;
        }
    }

    int t_gpcv::pcvS(double &corr, t_gsatdata &satdata, OBSCOMBIN lc, GOBSBAND &b1, GOBSBAND &b2, t_gtriple &site)
    {
        switch (lc)
        {
        case OBSCOMBIN::IONO_FREE:
            return pcvS_cmb(corr, satdata, b1, b2, site);
            break;
        case OBSCOMBIN::RAW_ALL:
        case OBSCOMBIN::RAW_MIX:
            return pcvS_raw(corr, satdata, b1, site);
            break;
        default:
            return -1;
            break;
        }
    }

    int t_gpcv::pcvR(double &corr, t_gsatdata &satdata, OBSCOMBIN lc, GOBSBAND &b1, GOBSBAND &b2)
    {
        switch (lc)
        {
        case OBSCOMBIN::IONO_FREE:
            return pcvR_cmb(corr, satdata, b1, b2);
            break;
        case OBSCOMBIN::RAW_ALL:
        case OBSCOMBIN::RAW_MIX:
            return pcvR_raw(corr, satdata, b1);
            break;
        default:
            return -1;
            break;
        }
    }

    int t_gpcv::pcoS_cmb(t_gsatdata &satdata, t_gtriple &pco, GOBSBAND &b1, GOBSBAND &b2)
    {

        GSYS gsys = satdata.gsys();
        string sat = satdata.sat();

        t_gtriple apcf1, apcf2, apcLC;

        GFRQ f1 = t_gsys::band2gfrq(gsys, b1);
        GFRQ f2 = t_gsys::band2gfrq(gsys, b2);

        if (_mappco.find(f1) == _mappco.end())
        {
            f1 = t_gsys::freq_priority(gsys, (t_gsys::gfrq2freq(gsys, f1) <= 2) ? t_gsys::gfrq2freq(gsys, f1) : FREQ_2);
        }

        if (_mappco.find(f2) == _mappco.end())
        {
            f2 = t_gsys::freq_priority(gsys, (t_gsys::gfrq2freq(gsys, f2) <= 2) ? t_gsys::gfrq2freq(gsys, f2) : FREQ_2);
        }

        //TODO COMMENT
        if (_mappco.find(f1) != _mappco.end() && _mappco.find(f2) != _mappco.end())
        {
            apcf1 = _mappco.at(f1);
            apcf2 = _mappco.at(f2);
        }
        else
        {
            if (_gnote)
            {
                if (_mappco.find(f1) != _mappco.end())
                    _gnote->mesg(GWARNING, "gpcv", "no SAT PCO [" + _anten + "/" + t_gfreq::gfreq2str(f1) + "], using hardwired values");
                if (_mappco.find(f2) != _mappco.end())
                    _gnote->mesg(GWARNING, "gpcv", "no SAT PCO [" + _anten + "/" + t_gfreq::gfreq2str(f2) + "], using hardwired values");
            }

            t_map_offs offs = GNSS_PCO_OFFSETS();
            t_map_offs::iterator itSYS = offs.find(gsys);
            if (itSYS != offs.end())
            {
                t_map_pcos::iterator itb1 = itSYS->second.find(b1);
                t_map_pcos::iterator itb2 = itSYS->second.find(b2);

                if (itb1 != itSYS->second.end())
                {
                    apcf1[0] = itb1->second[0];
                    apcf1[1] = itb1->second[1];
                    apcf1[2] = itb1->second[2];
                }

                if (itb2 != itSYS->second.end())
                {
                    apcf2[0] = itb2->second[0];
                    apcf2[1] = itb2->second[1];
                    apcf2[2] = itb2->second[2];
                }
            }
        }

        for (int i = 0; i <= 2; i++)
        {
            apcf1[i] /= 1000.0;
            apcf2[i] /= 1000.0;
        }

        double koef1 = 0.0;
        double koef2 = 0.0;
        satdata.coef_ionofree(b1, koef1, b2, koef2);
        apcLC = apcf1 * koef1 + apcf2 * koef2;

        pco = apcLC;

        return 1;
    }

    int t_gpcv::pcoR_cmb(t_gsatdata &satdata, t_gtriple &pco, GOBSBAND &b1, GOBSBAND &b2)
    {
        _gmutex.lock();
        t_gtriple apcf1;
        t_gtriple apcf2;

        GSYS gsys = satdata.gsys();

        GFRQ f1 = t_gsys::band2gfrq(gsys, b1);
        GFRQ f2 = t_gsys::band2gfrq(gsys, b2);

        if (_mappco.find(f1) == _mappco.end())
        {
            if (_gnote)
                _gnote->mesg(GWARNING, "gpcv", "no REC PCO [" + _anten + "/" + t_gfreq::gfreq2str(f1) + "], substituted with GPS L1");
            f1 = t_gsys::freq_priority(gsys, (t_gsys::gfrq2freq(gsys, f1) <= 2) ? t_gsys::gfrq2freq(gsys, f1) : FREQ_2);
        }
        if (_mappco.find(f2) == _mappco.end())
        {
            if (_gnote)
                _gnote->mesg(GWARNING, "gpcv", "no REC PCO [" + _anten + "/" + t_gfreq::gfreq2str(f2) + "], substituted with GPS L2");
            f2 = t_gsys::freq_priority(gsys, (t_gsys::gfrq2freq(gsys, f2) <= 2) ? t_gsys::gfrq2freq(gsys, f2) : FREQ_2);
        }

        if (_mappco.find(f1) == _mappco.end())
        {
            if (_gnote)
                _gnote->mesg(GWARNING, "gpcv", "no REC PCO [" + _anten + "/" + t_gfreq::gfreq2str(f1) + "], substituted with GPS L1");
            f1 = t_gsys::freq_priority(GPS, (t_gsys::gfrq2freq(gsys, f1) <= 2) ? t_gsys::gfrq2freq(gsys, f1) : FREQ_2);
        }
        if (_mappco.find(f2) == _mappco.end())
        {
            if (_gnote)
                _gnote->mesg(GWARNING, "gpcv", "no REC PCO [" + _anten + "/" + t_gfreq::gfreq2str(f2) + "], substituted with GPS L2");
            f2 = t_gsys::freq_priority(GPS, (t_gsys::gfrq2freq(gsys, f2) <= 2) ? t_gsys::gfrq2freq(gsys, f2) : FREQ_2);
        }
        // ==================================================

        if (_mappco.find(f1) == _mappco.end() || _mappco.find(f2) == _mappco.end())
        {
            if (_gnote)
            {
                if (_mappco.find(f1) == _mappco.end())
                {
                    _gnote->mesg(GWARNING, "gpcv", "no REC PCO [" + _anten + "/" + t_gfreq::gfreq2str(f1) + "]");
                }
                if (_mappco.find(f2) == _mappco.end())
                {
                    _gnote->mesg(GWARNING, "gpcv", "no REC PCO [" + _anten + "/" + t_gfreq::gfreq2str(f2) + "]");
                }
            }
            _gmutex.unlock();
            return -1;
        }

        apcf1 = _mappco[f1];
        apcf2 = _mappco[f2];

        for (int i = 0; i <= 2; i++)
        {
            apcf1[i] /= 1000.0;
            apcf2[i] /= 1000.0;
        }

        double koef1 = 0.0;
        double koef2 = 0.0;
        satdata.coef_ionofree(b1, koef1, b2, koef2);
        pco = apcf1 * koef1 + apcf2 * koef2;

        _gmutex.unlock();
        return 1;
    }

    int t_gpcv::pcvS_cmb(double &corr, t_gsatdata &satdata, GOBSBAND &b1, GOBSBAND &b2, t_gtriple &site)
    {
        _gmutex.lock();
        corr = 0.0;
        GSYS gsys = satdata.gsys();

        GFRQ f1 = t_gsys::band2gfrq(gsys, b1);
        GFRQ f2 = t_gsys::band2gfrq(gsys, b2);

        if (_mapzen.find(f1) == _mapzen.end())
        {
            if (_gnote)
                _gnote->mesg(GWARNING, "gpcv", "no REC PCV [" + _anten + "/" + t_gfreq::gfreq2str(f1) + "], substituted with GSYS L1");
            f1 = t_gsys::freq_priority(gsys, (t_gsys::gfrq2freq(gsys, f1) <= 2) ? t_gsys::gfrq2freq(gsys, f1) : FREQ_2);
        }

        if (_mapzen.find(f2) == _mapzen.end())
        {
            if (_gnote)
                _gnote->mesg(GWARNING, "gpcv", "no REC PCV [" + _anten + "/" + t_gfreq::gfreq2str(f2) + "], substituted with GSYS L1");
            f1 = t_gsys::freq_priority(gsys, (t_gsys::gfrq2freq(gsys, f2) <= 2) ? t_gsys::gfrq2freq(gsys, f2) : FREQ_2);
        }

        if (_mapzen.find(f1) == _mapzen.end() || _mapzen.find(f2) == _mapzen.end())
        {
            if (_gnote)
            {
                _gnote->mesg(GWARNING, "gpcv", "no SAT PCV [" + _anten + "/" + t_gfreq::gfreq2str(f1) + "]");
            }
            _gmutex.unlock();
            return -1;
        }

        map<double, double> mapDataf1;
        mapDataf1 = _mapzen[f1];
        map<double, double> mapDataf2;
        mapDataf2 = _mapzen[f2];

        double eleS = satdata.ele();
        double rS = satdata.satcrd().crd_cvect().NormFrobenius();
        double rR = site.crd_cvect().NormFrobenius();

        double sinz = (rR / rS) * cos(eleS);
        double zen = asin(sinz) * R2D;

        double corrf1 = 0.0;
        double corrf2 = 0.0;

        t_ginterp interp(_spdlog);

        if (interp.linear(mapDataf1, zen, corrf1) < 0 || interp.linear(mapDataf2, zen, corrf2) < 0)
        {
            _gmutex.unlock();
            return -1;
        }
        // [mm] -> [m]
        corrf1 /= 1000.0;
        corrf2 /= 1000.0;

        double koef1 = 0.0;
        double koef2 = 0.0;
        satdata.coef_ionofree(b1, koef1, b2, koef2);
        corr = corrf1 * koef1 + corrf2 * koef2;

        _gmutex.unlock();
        return 1;
    }

    int t_gpcv::pcvR_cmb(double &corr, t_gsatdata &satdata, GOBSBAND &b1, GOBSBAND &b2)
    {
        _gmutex.lock();
        GSYS gsys = satdata.gsys();

        GFRQ f1 = t_gsys::band2gfrq(gsys, b1);
        GFRQ f2 = t_gsys::band2gfrq(gsys, b2);

        if (_mapzen.find(f1) == _mapzen.end())
        {
            if (_gnote)
                _gnote->mesg(GWARNING, "gpcv", "no REC PCV [" + _anten + "/" + t_gfreq::gfreq2str(f1) + "], substituted with GPS L1");
            f1 = t_gsys::freq_priority(gsys, (t_gsys::gfrq2freq(gsys, f1) <= 2) ? t_gsys::gfrq2freq(gsys, f1) : FREQ_2);
        }

        if (_mapzen.find(f2) == _mapzen.end())
        {
            if (_gnote)
                _gnote->mesg(GWARNING, "gpcv", "no REC PCV [" + _anten + "/" + t_gfreq::gfreq2str(f2) + "], substituted with GPS L2");
            f2 = t_gsys::freq_priority(gsys, (t_gsys::gfrq2freq(gsys, f2) <= 2) ? t_gsys::gfrq2freq(gsys, f2) : FREQ_2);
        }

        if (_mapzen.find(f1) == _mapzen.end())
        {
            if (_gnote)
                _gnote->mesg(GWARNING, "gpcv", "no REC PCV [" + _anten + "/" + t_gfreq::gfreq2str(f1) + "], substituted with GPS L1");
            f1 = t_gsys::freq_priority(GPS, (t_gsys::gfrq2freq(gsys, f1) <= 2) ? t_gsys::gfrq2freq(gsys, f1) : FREQ_2);
        }

        if (_mapzen.find(f2) == _mapzen.end())
        {
            if (_gnote)
                _gnote->mesg(GWARNING, "gpcv", "no REC PCV [" + _anten + "/" + t_gfreq::gfreq2str(f2) + "], substituted with GPS L2");
            f2 = t_gsys::freq_priority(GPS, (t_gsys::gfrq2freq(gsys, f2) <= 2) ? t_gsys::gfrq2freq(gsys, f2) : FREQ_2);
        }

        if (_mapzen.find(f1) == _mapzen.end())
        {
            _gmutex.unlock();
            return -1;
        }
        if (_mapzen.find(f2) == _mapzen.end())
        {
            _gmutex.unlock();
            return -1;
        }
        double zen = G_PI / 2.0 - satdata.ele();
        zen *= R2D;

        double azi = satdata.azi();
        azi *= R2D;

        double corrf1 = 0.0;
        double corrf2 = 0.0;

        if (_azi_dependent(f1) && _azi_dependent(f2))
        {
            // AZI-dependant calibration available
            t_gpair p_az(azi, zen);
            map<t_gpair, double> mapDataf1;
            map<t_gpair, double> mapDataf2;

            for (map<GFRQ, t_map_A>::iterator itGFRQ = _mapazi.begin(); itGFRQ != _mapazi.end(); itGFRQ++)
            {
                GFRQ f = itGFRQ->first;
                if (f != f1 && f != f2)
                    continue;

                map<double, t_map_Z>::iterator itA1 = itGFRQ->second.lower_bound(azi);
                if (itA1 == itGFRQ->second.end() || itA1 == itGFRQ->second.begin())
                {
                    _gmutex.unlock();
                    return -1;
                }
                map<double, t_map_Z>::iterator itA2 = itA1;
                itA2--;

                map<double, double>::iterator itZ1 = itA1->second.lower_bound(zen);
                if (itZ1 == itA1->second.end() || itZ1 == itA1->second.begin())
                {
                    _gmutex.unlock();
                    return -1;
                }
                map<double, double>::iterator itZ2 = itZ1;
                itZ2--;

                map<double, double>::iterator itZ3 = itA2->second.lower_bound(zen);
                if (itZ3 == itA2->second.end() || itZ3 == itA2->second.begin())
                {
                    _gmutex.unlock();
                    return -1;
                }
                map<double, double>::iterator itZ4 = itZ3;
                itZ4--;

                t_gpair p1(itA1->first, itZ1->first);
                t_gpair p2(itA1->first, itZ2->first);
                t_gpair p3(itA2->first, itZ3->first);
                t_gpair p4(itA2->first, itZ4->first);

                if (f == f1)
                {
                    mapDataf1[p1] = itZ1->second; 
                    mapDataf1[p2] = itZ2->second;
                    mapDataf1[p3] = itZ3->second; 
                    mapDataf1[p4] = itZ4->second;
                }
                else if (f == f2)
                {
                    mapDataf2[p1] = itZ1->second;
                    mapDataf2[p2] = itZ2->second;
                    mapDataf2[p3] = itZ3->second;
                    mapDataf2[p4] = itZ4->second;
                }
                else
                {
                    continue;
                }
            }

            t_ginterp interp(_spdlog);
            if (interp.bilinear(mapDataf1, p_az, corrf1) < 0 || interp.bilinear(mapDataf2, p_az, corrf2) < 0)
            {
                _gmutex.unlock();
                return -1;
            }
        }
        else
        { // AZI-dependant calibration NOT available (only NOAZI)

            if (_gnote)
            {
                _gnote->mesg(GWARNING, "gpcv", "no REC AZI PCV [" + _anten + "/freq:" + t_gfreq::gfreq2str(f1) + "], just used NOAZI");
            }

            map<double, double> mapDataf1;
            map<double, double> mapDataf2;

            if (_mapzen.find(f1) == _mapzen.end() || _mapzen.find(f2) == _mapzen.end())
            {
                _gmutex.unlock();
                return -1;
            }

            mapDataf1 = _mapzen[f1];
            mapDataf2 = _mapzen[f2];
            t_ginterp interp(_spdlog);
            if (interp.linear(mapDataf1, zen, corrf1) < 0 || interp.linear(mapDataf2, zen, corrf2) < 0)
            {
                _gmutex.unlock();
                return -1;
            }
        }

        corrf1 /= 1000.0;
        corrf2 /= 1000.0;

        // PCV for linear combination
        double koef1 = 0.0;
        double koef2 = 0.0;
        satdata.coef_ionofree(b1, koef1, b2, koef2);
        corr = corrf1 * koef1 + corrf2 * koef2;
        _gmutex.unlock();
        return 1;
    }

    int t_gpcv::pcoS_raw(t_gsatdata &satdata, t_gtriple &pco, GOBSBAND &b1)
    {

        GSYS gsys = satdata.gsys();
        string sat = satdata.sat();

        t_gtriple apcf1;

        GFRQ f1 = t_gsys::band2gfrq(gsys, b1);
        if (_mappco.find(f1) == _mappco.end() && f1 == C09)
        {
            f1 = C07;
        }
        if (_mappco.find(f1) == _mappco.end())
        {
            f1 = t_gsys::freq_priority(gsys, (t_gsys::gfrq2freq(gsys, f1) <= 2) ? t_gsys::gfrq2freq(gsys, f1) : FREQ_2);
        }

        if (_mappco.find(f1) != _mappco.end())
        {
            apcf1 = _mappco.at(f1);
        }
        else
        {
            if (_gnote)
            {
                if (_mappco.find(f1) != _mappco.end())
                    _gnote->mesg(GWARNING, "gpcv", "no SAT PCO [" + _anten + "/" + t_gfreq::gfreq2str(f1) + "], using hardwired values");
            }

            t_map_offs offs = GNSS_PCO_OFFSETS();
            t_map_offs::iterator itSYS = offs.find(gsys);
            if (itSYS != offs.end())
            {
                t_map_pcos::iterator itb1 = itSYS->second.find(b1);

                if (itb1 != itSYS->second.end())
                {
                    apcf1[0] = itb1->second[0];
                    apcf1[1] = itb1->second[1];
                    apcf1[2] = itb1->second[2];
                }
            }
        }

        for (int i = 0; i <= 2; i++)
        {
            apcf1[i] /= 1000.0;
        }

        // PCO for linear combination
        pco = apcf1;

        return 1;
    }

    int t_gpcv::pcoR_raw(t_gsatdata &satdata, t_gtriple &pco, GOBSBAND &b1)
    {

        t_gtriple apcf1, apcLC;
        GSYS gsys = satdata.gsys();

        GFRQ f1 = t_gsys::band2gfrq(gsys, b1);

        if (_mappco.find(f1) == _mappco.end() && f1 == C09)
        {
            f1 = C07;
        }

        // fix for muti-GNSS
        if (_mappco.find(f1) == _mappco.end())
        {
            if (_gnote)
                _gnote->mesg(GWARNING, "gpcv", "no REC PCO [" + _anten + "/" + t_gfreq::gfreq2str(f1) + "], substituted with GPS L2");
            f1 = t_gsys::freq_priority(gsys, (t_gsys::gfrq2freq(gsys, f1) <= 2) ? t_gsys::gfrq2freq(gsys, f1) : FREQ_2);
        }

        if (_mappco.find(f1) == _mappco.end())
        {
            if (_gnote)
                _gnote->mesg(GWARNING, "gpcv", "no REC PCO [" + _anten + "/" + t_gfreq::gfreq2str(f1) + "], substituted with GPS L2");
            f1 = t_gsys::freq_priority(GPS, (t_gsys::gfrq2freq(gsys, f1) <= 2) ? t_gsys::gfrq2freq(gsys, f1) : FREQ_2);
        }

        // ==================================================
        if (_mappco.find(f1) == _mappco.end())
        {
            if (_gnote)
            {
                if (_mappco.find(f1) == _mappco.end())
                {
                    _gnote->mesg(GWARNING, "gpcv", "no REC PCO [" + _anten + "/" + t_gfreq::gfreq2str(f1) + "]");
                }
            }
            return -1;
        }

        apcf1 = _mappco[f1];

        for (int i = 0; i <= 2; i++)
        {
            apcf1[i] /= 1000.0;
        }

        pco = apcf1;

            return 1;
    }

    int t_gpcv::pcvS_raw(double &corr, t_gsatdata &satdata, GOBSBAND &b1, t_gtriple &site)
    {

        GSYS gsys = satdata.gsys();

        GFRQ f1 = t_gsys::band2gfrq(gsys, b1);

        if (_mapzen.find(f1) == _mapzen.end() && f1 == C09)
        {
            f1 = C07;
        }
        if (_mapzen.find(f1) == _mapzen.end())
        {
            if (_gnote)
                _gnote->mesg(GWARNING, "gpcv", "no REC PCV [" + _anten + "/" + t_gfreq::gfreq2str(f1) + "], substituted with GSYS L1");
            f1 = t_gsys::freq_priority(gsys, (t_gsys::gfrq2freq(gsys, f1) <= 2) ? t_gsys::gfrq2freq(gsys, f1) : FREQ_2);
        }

        if (_mapzen.find(f1) == _mapzen.end())
        {

            return -1;
        }

        double eleS = satdata.ele();
        double rS = satdata.satcrd().crd_cvect().NormFrobenius();
        double rR = site.crd_cvect().NormFrobenius();
        double sinz = (rR / rS) * cos(eleS);
        double zen = asin(sinz) * R2D;
        double azi = satdata.azi_sat() * R2D;

        double corrf1 = 0.0;
        if (_azi_dependent(f1))
        {
            t_gpair p_az(azi, zen);
            map<t_gpair, double> mapDataf1;

            for (map<GFRQ, t_map_A>::iterator itGFRQ = _mapazi.begin(); itGFRQ != _mapazi.end(); itGFRQ++)
            {
                GFRQ f = itGFRQ->first;

                if (f != f1)
                    continue;

                map<double, t_map_Z>::iterator itA1 = itGFRQ->second.lower_bound(azi);
                if (itA1 == itGFRQ->second.end() || itA1 == itGFRQ->second.begin())
                {
                    return -1;
                }
                map<double, t_map_Z>::iterator itA2 = itA1;
                itA2--;

                map<double, double>::iterator itZ1 = itA1->second.lower_bound(zen);
                if (itZ1 == itA1->second.end() || itZ1 == itA1->second.begin())
                {
                    return -1;
                }
                map<double, double>::iterator itZ2 = itZ1;
                itZ2--;

                map<double, double>::iterator itZ3 = itA2->second.lower_bound(zen);
                if (itZ3 == itA2->second.end() || itZ3 == itA2->second.begin())
                {
                    return -1;
                }
                map<double, double>::iterator itZ4 = itZ3;
                itZ4--;

                t_gpair p1(itA1->first, itZ1->first);
                t_gpair p2(itA1->first, itZ2->first);
                t_gpair p3(itA2->first, itZ3->first);
                t_gpair p4(itA2->first, itZ4->first);

                if (f == f1)
                {
                    mapDataf1[p1] = itZ1->second;
                    mapDataf1[p2] = itZ2->second; 
                    mapDataf1[p3] = itZ3->second; 
                    mapDataf1[p4] = itZ4->second; 
                }
                else
                {
                    continue;
                }
            }
            t_ginterp interp(_spdlog);
            if (interp.bilinear(mapDataf1, p_az, corrf1) < 0)
            {
                return -1;
            }
        }
        else
        {
            map<double, double> mapDataf1;
            mapDataf1 = _mapzen.at(f1);

            t_ginterp interp(_spdlog);
            if (interp.linear(mapDataf1, zen, corrf1) < 0)
            {
                return -1;
            }
        }

        corrf1 /= 1000.0;

        // PCV for linear combination
        corr = corrf1;

        return 1;
    }

    int t_gpcv::pcvR_raw(double &corr, t_gsatdata &satdata, GOBSBAND &b1)
    {
        GSYS gsys = satdata.gsys();

        GFRQ f1 = t_gsys::band2gfrq(gsys, b1);

        if (_mapzen.find(f1) == _mapzen.end() && f1 == C09)
        {
            f1 = C07;
        }

        if (_mapzen.find(f1) == _mapzen.end())
        {
            if (_gnote)
                _gnote->mesg(GWARNING, "gpcv", "no REC PCV [" + _anten + "/" + t_gfreq::gfreq2str(f1) + "], substituted with GPS L1");
            f1 = t_gsys::freq_priority(gsys, (t_gsys::gfrq2freq(gsys, f1) <= 2) ? t_gsys::gfrq2freq(gsys, f1) : FREQ_2);
        }

        if (_mapzen.find(f1) == _mapzen.end())
        {
            if (_gnote)
                _gnote->mesg(GWARNING, "gpcv", "no REC PCV [" + _anten + "/" + t_gfreq::gfreq2str(f1) + "], substituted with GPS L1");
            f1 = t_gsys::freq_priority(GPS, (t_gsys::gfrq2freq(gsys, f1) <= 2) ? t_gsys::gfrq2freq(gsys, f1) : FREQ_2);
        }

        if (_mapzen.find(f1) == _mapzen.end())
        {
            return -1;
        }

        double zen = G_PI / 2.0 - satdata.ele();
        zen *= R2D;

        double azi = satdata.azi();
        azi *= R2D;

        double corrf1 = 0.0;

        if (_azi_dependent(f1))
        {
            // AZI-dependant calibration available
            t_gpair p_az(azi, zen);
            map<t_gpair, double> mapDataf1;
            for (map<GFRQ, t_map_A>::iterator itGFRQ = _mapazi.begin(); itGFRQ != _mapazi.end(); itGFRQ++)
            {
                GFRQ f = itGFRQ->first;

                if (f != f1)
                    continue;

                map<double, t_map_Z>::iterator itA1 = itGFRQ->second.lower_bound(azi);
                if (itA1 == itGFRQ->second.end() || itA1 == itGFRQ->second.begin())
                {
                    return -1;
                }
                map<double, t_map_Z>::iterator itA2 = itA1;
                itA2--;

                map<double, double>::iterator itZ1 = itA1->second.lower_bound(zen);
                if (itZ1 == itA1->second.end() || itZ1 == itA1->second.begin())
                {
                    return -1;
                }
                map<double, double>::iterator itZ2 = itZ1;
                itZ2--;

                map<double, double>::iterator itZ3 = itA2->second.lower_bound(zen);
                if (itZ3 == itA2->second.end() || itZ3 == itA2->second.begin())
                {
                    return -1;
                }
                map<double, double>::iterator itZ4 = itZ3;
                itZ4--;

                t_gpair p1(itA1->first, itZ1->first);
                t_gpair p2(itA1->first, itZ2->first);
                t_gpair p3(itA2->first, itZ3->first);
                t_gpair p4(itA2->first, itZ4->first);

                if (f == f1)
                {
                    mapDataf1[p1] = itZ1->second;
                    mapDataf1[p2] = itZ2->second;
                    mapDataf1[p3] = itZ3->second;
                    mapDataf1[p4] = itZ4->second; 
                }
                else
                {
                    continue;
                }
            }

            t_ginterp interp(_spdlog);
            if (interp.bilinear(mapDataf1, p_az, corrf1) < 0)
            {
                return -1;
            }
        }
        else
        { // AZI-dependant calibration NOT available (only NOAZI)

            if (_gnote)
            {
                _gnote->mesg(GWARNING, "gpcv", "no REC AZI PCV [" + _anten + "/freq:" + t_gfreq::gfreq2str(f1) + "], just used NOAZI");
            }

            map<double, double> mapDataf1;
            if (_mapzen.find(f1) != _mapzen.end())
            {
                mapDataf1 = _mapzen[f1];
            }
            else
            {
                return -1;
            }

            t_ginterp interp(_spdlog);
            if (interp.linear(mapDataf1, zen, corrf1) < 0)
            {
                return -1;
            }
        }

        corrf1 /= 1000.0;

        corr = corrf1;

        return 1;
    }

    int t_gpcv::pcoS(t_gsatdata &satdata, t_gtriple &pco, GOBS_LC lc, GOBSBAND k1, GOBSBAND k2)
    {
        t_gtime epo = satdata.epoch();
        t_gtriple Satcrd_t = satdata.satcrd();
        t_gtriple Satvel_t = satdata.satvel(); // TEMPORARY
        GSYS gsys = satdata.gsys();
        string sat = satdata.sat();
        ColumnVector Satcrd = Satcrd_t.crd_cvect();
        ColumnVector Satvel = Satvel_t.crd_cvect(); // TEMPORARY

        t_gtriple apcf1, apcf2, apcf3, apcf4, apcf5, apcLC;

        // JD: New flexible way of defining L3 frequency for multi-GNSS
        GOBSBAND b1 = t_gsys::band_priority(gsys, FREQ_1);
        GFRQ f1 = t_gsys::freq_priority(gsys, FREQ_1);
        GOBSBAND b2 = t_gsys::band_priority(gsys, FREQ_2);
        GFRQ f2 = t_gsys::freq_priority(gsys, FREQ_2);
        GOBSBAND b3 = t_gsys::band_priority(gsys, FREQ_3);
        GFRQ f3 = t_gsys::freq_priority(gsys, FREQ_3);
        GOBSBAND b4 = t_gsys::band_priority(gsys, FREQ_4);
        GFRQ f4 = t_gsys::freq_priority(gsys, FREQ_4);
        GOBSBAND b5 = t_gsys::band_priority(gsys, FREQ_5);
        GFRQ f5 = t_gsys::freq_priority(gsys, FREQ_5);

        if (gsys == BDS)
        {
            b1 = t_gsys::band_priority(gsys, FREQ_1);
            f1 = t_gsys::freq_priority(gsys, FREQ_1);
            b2 = t_gsys::band_priority(gsys, FREQ_3);
            f2 = t_gsys::freq_priority(gsys, FREQ_3);
            b3 = t_gsys::band_priority(gsys, FREQ_4);
            f3 = t_gsys::freq_priority(gsys, FREQ_4);
            b4 = t_gsys::band_priority(gsys, FREQ_6);
            f4 = t_gsys::freq_priority(gsys, FREQ_6);
            b5 = t_gsys::band_priority(gsys, FREQ_5);
            f5 = t_gsys::freq_priority(gsys, FREQ_5);
        }

        if (_mappco.find(f1) != _mappco.end() && _mappco.find(f2) != _mappco.end())
        {
            apcf1 = _mappco.at(f1);
            apcf2 = _mappco.at(f2);
            apcf3 = (_mappco.find(f3) != _mappco.end()) ? _mappco.at(f3) : _mappco.at(f2);
            apcf4 = (_mappco.find(f4) != _mappco.end()) ? _mappco.at(f4) : _mappco.at(f2);
            apcf5 = (_mappco.find(f5) != _mappco.end()) ? _mappco.at(f5) : _mappco.at(f2);
        }
        else
        {
            if (_gnote)
            {
                if (_mappco.find(f1) != _mappco.end())
                    _gnote->mesg(GWARNING, "gpcv", "no SAT PCO [" + _anten + "/" + t_gfreq::gfreq2str(f1) + "], using hardwired values");
                if (_mappco.find(f2) != _mappco.end())
                    _gnote->mesg(GWARNING, "gpcv", "no SAT PCO [" + _anten + "/" + t_gfreq::gfreq2str(f2) + "], using hardwired values");
                if (_mappco.find(f3) != _mappco.end())
                    _gnote->mesg(GWARNING, "gpcv", "no SAT PCO [" + _anten + "/" + t_gfreq::gfreq2str(f3) + "], using hardwired values");
                if (_mappco.find(f4) != _mappco.end())
                    _gnote->mesg(GWARNING, "gpcv", "no SAT PCO [" + _anten + "/" + t_gfreq::gfreq2str(f4) + "], using hardwired values");
                if (_mappco.find(f5) != _mappco.end())
                    _gnote->mesg(GWARNING, "gpcv", "no SAT PCO [" + _anten + "/" + t_gfreq::gfreq2str(f5) + "], using hardwired values");
            }

            t_map_offs offs = GNSS_PCO_OFFSETS();
            t_map_offs::iterator itSYS = offs.find(gsys);
            if (itSYS != offs.end())
            {
                t_map_pcos::iterator itb1 = itSYS->second.find(b1);
                t_map_pcos::iterator itb2 = itSYS->second.find(b2);
                t_map_pcos::iterator itb3 = itSYS->second.find(b3);
                t_map_pcos::iterator itb4 = itSYS->second.find(b4);
                t_map_pcos::iterator itb5 = itSYS->second.find(b5);

                if (itb1 != itSYS->second.end())
                {
                    apcf1[0] = itb1->second[0];
                    apcf1[1] = itb1->second[1];
                    apcf1[2] = itb1->second[2];
                }

                if (itb2 != itSYS->second.end())
                {
                    apcf2[0] = itb2->second[0];
                    apcf2[1] = itb2->second[1];
                    apcf2[2] = itb2->second[2];
                }

                if (itb3 != itSYS->second.end())
                {
                    apcf3[0] = itb3->second[0];
                    apcf3[1] = itb3->second[1];
                    apcf3[2] = itb3->second[2];
                }

                if (itb4 != itSYS->second.end())
                {
                    apcf4[0] = itb4->second[0];
                    apcf4[1] = itb4->second[1];
                    apcf4[2] = itb4->second[2];
                }

                if (itb5 != itSYS->second.end())
                {
                    apcf5[0] = itb5->second[0];
                    apcf5[1] = itb5->second[1];
                    apcf5[2] = itb5->second[2];
                }
            }
        }

        for (int i = 0; i <= 2; i++)
        {
            apcf1[i] /= 1000.0;
            apcf2[i] /= 1000.0;
            apcf3[i] /= 1000.0;
            apcf4[i] /= 1000.0;
            apcf5[i] /= 1000.0;
        }

        // PCO for linear combination
        if (lc == LC_IF)
        {
            double koef1 = 0.0;
            double koef2 = 0.0;
            satdata.coef_ionofree(k1, koef1, k2, koef2);
            apcLC = apcf1 * koef1 + apcf2 * koef2;
        }
        else if (lc == LC_L1)
        {
            apcLC = apcf1;
        }
        else if (lc == LC_L2)
        {
            apcLC = apcf2;
        }
        else if (lc == LC_L3)
        {
            apcLC = apcf3;
        }
        else if (lc == LC_L4)
        {
            apcLC = apcf4;
        }
        else if (lc == LC_L5)
        {
            apcLC = apcf5;
        }
        else
        {
            apcLC = apcf2;
        }

        pco = apcLC;

        return 1;
    }

    // Receiver pco
    // -----------------------------------------------
    int t_gpcv::pcoR(t_gsatdata &satdata, t_gtriple &pco, GOBS_LC lc, GOBSBAND k1, GOBSBAND k2)
    {
        _gmutex.lock();

        t_gtriple apcf1, apcf2, apcLC;

        GSYS gsys = satdata.gsys();

        // Temporary - GAL site antenna calibration not available

        // JD: New flexible way of defining L3 frequency for multi-GNSS
        GFRQ f1 = t_gsys::freq_priority(gsys, FREQ_1);
        GFRQ f2 = t_gsys::freq_priority(gsys, FREQ_2);
        _gnote = nullptr;
        if (_mappco.find(f1) == _mappco.end())
        {
            if (_gnote)
                _gnote->mesg(GWARNING, "gpcv", "no REC PCO [" + _anten + "/" + t_gfreq::gfreq2str(f1) + "], substituted with GPS L1");
            f1 = t_gsys::freq_priority(GPS, (FREQ_1));
        }
        if (_mappco.find(f2) == _mappco.end())
        {
            if (_gnote)
                _gnote->mesg(GWARNING, "gpcv", "no REC PCO [" + _anten + "/" + t_gfreq::gfreq2str(f2) + "], substituted with GPS L2");
            f2 = t_gsys::freq_priority(GPS, (t_gsys::gfrq2freq(gsys, f2) <= 2) ? t_gsys::gfrq2freq(gsys, f2) : FREQ_2);
        }
        // ==================================================

        if (_mappco.find(f1) == _mappco.end() || _mappco.find(f2) == _mappco.end())
        {
            if (_gnote)
            {
                if (_mappco.find(f1) == _mappco.end())
                {
                    _gnote->mesg(GWARNING, "gpcv", "no REC PCO [" + _anten + "/" + t_gfreq::gfreq2str(f1) + "]");
                }
                if (_mappco.find(f2) == _mappco.end())
                {
                    _gnote->mesg(GWARNING, "gpcv", "no REC PCO [" + _anten + "/" + t_gfreq::gfreq2str(f2) + "]");
                }
            }
            _gmutex.unlock();
            return -1;
        }

        apcf1 = _mappco[f1];
        apcf2 = _mappco[f2];

        for (int i = 0; i <= 2; i++)
        {
            apcf1[i] /= 1000.0;
            apcf2[i] /= 1000.0;
        }

        // PCO for linear combination
        if (lc == LC_IF)
        {
            double koef1 = 0.0;
            double koef2 = 0.0;

            satdata.coef_ionofree(k1, koef1, k2, koef2);
            apcLC = apcf1 * koef1 + apcf2 * koef2;
        }
        else if (lc == LC_L1)
        {
            apcLC = apcf1;
        }
        else if (lc == LC_L2)
        {
            apcLC = apcf2;
        }
        else
            apcLC = apcf2;

        pco = apcLC;
        _gmutex.unlock();
        return 1;
    }

    int t_gpcv::pcoR(t_gsatdata &satdata, t_gtriple &dx, t_gtriple &site, GOBS_LC lc)
    {
        _gmutex.lock();

        t_gtriple apcf1, apcf2, apcLC;

        GSYS gsys = satdata.gsys();
        GFRQ f1 = t_gsys::freq_priority(gsys, FREQ_1);
        GFRQ f2 = t_gsys::freq_priority(gsys, FREQ_2);
        GOBSBAND b1, b2; 
        if (_mappco.find(f1) == _mappco.end())
        {
            if (_gnote)
                _gnote->mesg(GWARNING, "gpcv", "no REC PCO [" + _anten + "/" + t_gfreq::gfreq2str(f1) + "], substituted with GPS L1");
            f1 = t_gsys::freq_priority(GPS, (FREQ_1));
        }
        if (_mappco.find(f2) == _mappco.end())
        {
            if (_gnote)
                _gnote->mesg(GWARNING, "gpcv", "no REC PCO [" + _anten + "/" + t_gfreq::gfreq2str(f2) + "], substituted with GPS L2");
            f2 = t_gsys::freq_priority(GPS, (t_gsys::gfrq2freq(gsys, f2) <= 2) ? t_gsys::gfrq2freq(gsys, f2) : FREQ_2);
        }
        if (_mappco.find(f1) == _mappco.end() || _mappco.find(f2) == _mappco.end())
        {
            if (_gnote)
            {
                if (_mappco.find(f1) == _mappco.end())
                {
                    _gnote->mesg(GWARNING, "gpcv", "no REC PCO [" + _anten + "/" + t_gfreq::gfreq2str(f1) + "]");
                }
                if (_mappco.find(f2) == _mappco.end())
                {
                    _gnote->mesg(GWARNING, "gpcv", "no REC PCO [" + _anten + "/" + t_gfreq::gfreq2str(f2) + "]");
                }
            }
            _gmutex.unlock();
            return -1;
        }

        apcf1 = _mappco[f1];
        apcf2 = _mappco[f2];

        for (int i = 0; i <= 2; i++)
        {
            apcf1[i] /= 1000.0;
            apcf2[i] /= 1000.0;
        }

        if (lc == LC_IF)
        {
            double koef1 = 0.0;
            double koef2 = 0.0;
            b1 = t_gsys::band_priority(gsys, FREQ_1);
            b2 = t_gsys::band_priority(gsys, FREQ_2);
            satdata.coef_ionofree(b1, koef1, b2, koef2);
            apcLC = apcf1 * koef1 + apcf2 * koef2;
        }
        else if (lc == LC_L1)
        {
            apcLC = apcf1;
        }
        else if (lc == LC_L2)
        {
            apcLC = apcf2;
        }
        else
            apcLC = apcf2;

        t_gtriple ell(0.0, 0.0, 0.0);
        xyz2ell(site, ell, false);
        neu2xyz(ell, apcLC, dx);

        _gmutex.unlock();
        return 1;
    }

    int t_gpcv::pco_proj(double &corr, t_gsatdata &satdata, t_gtriple &site, t_gtriple &dx)
    {

        t_gtriple satcrd = satdata.satcrd();
        t_gtriple SatRec(0.0, 0.0, 0.0);
        SatRec = satcrd - site;
        ColumnVector e = SatRec.unitary();

        corr = DotProduct(dx.crd_cvect(), e);

        return 1;
    }

    int t_gpcv::pcvS(double &corrLC, t_gsatdata &satdata, t_gtriple &site, GOBS_LC lc, GOBSBAND k1, GOBSBAND k2)
    {
        _gmutex.lock();

        GSYS gsys = satdata.gsys();

        GFRQ f1 = t_gsys::freq_priority(gsys, FREQ_1);
        GFRQ f2 = t_gsys::freq_priority(gsys, FREQ_2);
        GFRQ f3 = t_gsys::freq_priority(gsys, FREQ_3); 
        GFRQ f4 = t_gsys::freq_priority(gsys, FREQ_4);
        GFRQ f5 = t_gsys::freq_priority(gsys, FREQ_5);

        if (lc == GOBS_LC::LC_IF)
        {
            if (_mapzen.find(f1) == _mapzen.end() || _mapzen.find(f2) == _mapzen.end())
            {
                if (_gnote)
                {
                    _gnote->mesg(GWARNING, "gpcv", "no SAT PCV [" + _anten + "/" + t_gfreq::gfreq2str(f1) + "]");
                }
                _gmutex.unlock();
                return -1;
            }
        }

        if (lc == GOBS_LC::LC_L1 || lc == GOBS_LC::LC_L2 || lc == GOBS_LC::LC_L3 || lc == GOBS_LC::LC_L4 || lc == GOBS_LC::LC_L5)
        {
            switch (lc)
            {
            case GOBS_LC::LC_L1:
                if (_mapzen.find(f1) == _mapzen.end())
                {
                    _gmutex.unlock();
                    return -1;
                }
                break;
            case GOBS_LC::LC_L2:
                if (_mapzen.find(f2) == _mapzen.end())
                {
                    _gmutex.unlock();
                    return -1;
                }
                break;
            case GOBS_LC::LC_L3:
                if (_mapzen.find(f3) == _mapzen.end())
                    f3 = f2;
                if (_mapzen.find(f3) == _mapzen.end())
                {
                    _gmutex.unlock();
                    return -1;
                }
                break;
            case GOBS_LC::LC_L4:
                if (_mapzen.find(f4) == _mapzen.end())
                    f4 = f2;
                if (_mapzen.find(f4) == _mapzen.end())
                {
                    _gmutex.unlock();
                    return -1;
                }
                break;
            case GOBS_LC::LC_L5:
                if (_mapzen.find(f5) == _mapzen.end())
                    f5 = f2;
                if (_mapzen.find(f5) == _mapzen.end())
                {
                    _gmutex.unlock();
                    return -1;
                }
                break;
            default:
            {
                _gmutex.unlock();
                return -1;
            }
            }
        }

        map<double, double> mapDataf1;
        mapDataf1 = _mapzen[f1];
        map<double, double> mapDataf2;
        mapDataf2 = _mapzen[f2];
        map<double, double> mapDataf3;
        mapDataf3 = _mapzen[f3];
        map<double, double> mapDataf4;
        mapDataf4 = _mapzen[f4];
        map<double, double> mapDataf5;
        mapDataf5 = _mapzen[f5];

        double eleS = satdata.ele();
        double rS = satdata.satcrd().crd_cvect().NormFrobenius();
        double rR = site.crd_cvect().NormFrobenius();

        double sinz = (rR / rS) * cos(eleS);
        double zen = asin(sinz) * R2D;

        double corrf1 = 0.0;
        double corrf2 = 0.0;
        double corrf3 = 0.0;
        double corrf4 = 0.0;
        double corrf5 = 0.0;

        t_ginterp interp(_spdlog);
        if (lc == GOBS_LC::LC_IF)
        {
            if (interp.linear(mapDataf1, zen, corrf1) < 0 || interp.linear(mapDataf2, zen, corrf2) < 0)
            {
                _gmutex.unlock();
                return -1;
            }
        }
        else
        {
            
            switch (lc)
            {
            case GOBS_LC::LC_L1:
                if (interp.linear(mapDataf1, zen, corrf1) < 0)
                {
                    _gmutex.unlock();
                    return -1;
                }
                break;
            case GOBS_LC::LC_L2:
                if (interp.linear(mapDataf2, zen, corrf2) < 0)
                {
                    _gmutex.unlock();
                    return -1;
                }
                break;
            case GOBS_LC::LC_L3:
                if (interp.linear(mapDataf3, zen, corrf3) < 0)
                {
                    mapDataf3 = mapDataf2;
                }
                if (interp.linear(mapDataf3, zen, corrf3) < 0)
                {
                    _gmutex.unlock();
                    return -1;
                }
                break;
            case GOBS_LC::LC_L4:
                if (interp.linear(mapDataf4, zen, corrf4) < 0)
                {
                    mapDataf4 = mapDataf2;
                }
                if (interp.linear(mapDataf4, zen, corrf4) < 0)
                {
                    _gmutex.unlock();
                    return -1;
                }
                break;
            case GOBS_LC::LC_L5:
                if (interp.linear(mapDataf5, zen, corrf5) < 0)
                {
                    mapDataf5 = mapDataf2;
                }
                if (interp.linear(mapDataf5, zen, corrf5) < 0)
                {
                    _gmutex.unlock();
                    return -1;
                }
                break;
            default:
            {
                _gmutex.unlock();
                return -1;
            }
            }
        }
        // [mm] -> [m]
        corrf1 /= 1000.0;
        corrf2 /= 1000.0;
        corrf3 /= 1000.0;
        corrf4 /= 1000.0;
        corrf5 /= 1000.0;

        // PCV for linear combination
        if (lc == LC_IF)
        {
            double koef1 = 0.0;
            double koef2 = 0.0;

            satdata.coef_ionofree(k1, koef1, k2, koef2);
            corrLC = corrf1 * koef1 + corrf2 * koef2;
        }
        else if (lc == LC_L1)
        {
            corrLC = corrf1;
        }
        else if (lc == LC_L2)
        {
            corrLC = corrf2;
        }
        else if (lc == LC_L3)
        {
            corrLC = corrf3;
        }
        else if (lc == LC_L4)
        {
            corrLC = corrf4;
        }
        else if (lc == LC_L5)
        {
            corrLC = corrf5;
        }
        else
            corrLC = corrf2;

        _gmutex.unlock();
        return 1;
    }

    int t_gpcv::pcvR(double &corrLC, t_gsatdata &satdata, GOBS_LC lc, GOBSBAND k1, GOBSBAND k2)
    {
        _gmutex.lock();

        GSYS gsys = satdata.gsys();

        GFRQ f1 = t_gsys::freq_priority(gsys, FREQ_1);
        GFRQ f2 = t_gsys::freq_priority(gsys, FREQ_2);
        GFRQ f3 = t_gsys::freq_priority(gsys, FREQ_3);
        GFRQ f4 = t_gsys::freq_priority(gsys, FREQ_4);
        GFRQ f5 = t_gsys::freq_priority(gsys, FREQ_5);

        if (lc == GOBS_LC::LC_IF)
        {
            if (_mapzen.find(f1) == _mapzen.end())
            {
                if (_gnote)
                    _gnote->mesg(GWARNING, "gpcv", "no REC PCV [" + _anten + "/" + t_gfreq::gfreq2str(f1) + "], substituted with GPS L1");
                f1 = t_gsys::freq_priority(GPS, (FREQ_1));
            }
            if (_mapzen.find(f2) == _mapzen.end())
            {
                if (_gnote)
                    _gnote->mesg(GWARNING, "gpcv", "no REC PCV [" + _anten + "/" + t_gfreq::gfreq2str(f2) + "], substituted with GPS L2");
                f2 = t_gsys::freq_priority(GPS, (t_gsys::gfrq2freq(gsys, f2) <= 2) ? t_gsys::gfrq2freq(gsys, f2) : FREQ_2);
            }

            if (_mapzen.find(f1) == _mapzen.end())
            {
                _gmutex.unlock();
                return -1;
            }
            if (_mapzen.find(f2) == _mapzen.end())
            {
                _gmutex.unlock();
                return -1;
            }
        }
        else
        {
            switch (lc)
            {
            case GOBS_LC::LC_L1:
                if (_mapzen.find(f1) == _mapzen.end())
                {
                    if (_gnote)
                        _gnote->mesg(GWARNING, "gpcv", "no REC PCV [" + _anten + "/" + t_gfreq::gfreq2str(f1) + "], substituted with GPS L1");
                    f1 = t_gsys::freq_priority(GPS, (FREQ_1));
                }
                if (_mapzen.find(f1) == _mapzen.end())
                {
                    _gmutex.unlock();
                    return -1;
                }
                break;
            case GOBS_LC::LC_L2:
                if (_mapzen.find(f2) == _mapzen.end())
                {
                    if (_gnote)
                        _gnote->mesg(GWARNING, "gpcv", "no REC PCV [" + _anten + "/" + t_gfreq::gfreq2str(f2) + "], substituted with GPS L2");
                    f2 = t_gsys::freq_priority(GPS, (t_gsys::gfrq2freq(gsys, f2) <= 2) ? t_gsys::gfrq2freq(gsys, f2) : FREQ_2);
                }
                if (_mapzen.find(f2) == _mapzen.end())
                {
                    _gmutex.unlock();
                    return -1;
                }
                break;
            case GOBS_LC::LC_L3:
                if (_mapzen.find(f3) == _mapzen.end())
                {
                    if (_gnote)
                        _gnote->mesg(GWARNING, "gpcv", "no REC PCV [" + _anten + "/" + t_gfreq::gfreq2str(f2) + "], substituted with GPS L2");
                    f3 = t_gsys::freq_priority(GPS, (t_gsys::gfrq2freq(gsys, f2) <= 2) ? t_gsys::gfrq2freq(gsys, f2) : FREQ_2);
                }
                if (_mapzen.find(f3) == _mapzen.end())
                {
                    _gmutex.unlock();
                    return -1;
                }
                break;
            case GOBS_LC::LC_L4:
                if (_mapzen.find(f4) == _mapzen.end())
                {
                    if (_gnote)
                        _gnote->mesg(GWARNING, "gpcv", "no REC PCV [" + _anten + "/" + t_gfreq::gfreq2str(f2) + "], substituted with GPS L2");
                    f4 = t_gsys::freq_priority(GPS, (t_gsys::gfrq2freq(gsys, f2) <= 2) ? t_gsys::gfrq2freq(gsys, f2) : FREQ_2);
                }
                if (_mapzen.find(f4) == _mapzen.end())
                {
                    _gmutex.unlock();
                    return -1;
                }
                break;
            case GOBS_LC::LC_L5:
                if (_mapzen.find(f5) == _mapzen.end())
                {
                    if (_gnote)
                        _gnote->mesg(GWARNING, "gpcv", "no REC PCV [" + _anten + "/" + t_gfreq::gfreq2str(f2) + "], substituted with GPS L2");
                    f5 = t_gsys::freq_priority(GPS, (t_gsys::gfrq2freq(gsys, f2) <= 2) ? t_gsys::gfrq2freq(gsys, f2) : FREQ_2);
                }
                if (_mapzen.find(f5) == _mapzen.end())
                {
                    _gmutex.unlock();
                    return -1;
                }
                break;
            default:
                _gmutex.unlock();
                return -1;
            }
        }

        // ==================================================

        double zen = G_PI / 2.0 - satdata.ele();
        zen *= R2D;

        double azi = satdata.azi();
        azi *= R2D;

        double corrf1 = 0.0;
        double corrf2 = 0.0;
        double corrf3 = 0.0;
        double corrf4 = 0.0;
        double corrf5 = 0.0;

        bool isAzi = false;
        if (lc == GOBS_LC::LC_IF && _azi_dependent(f1) && _azi_dependent(f2))
            isAzi = true;
        else if (lc == GOBS_LC::LC_L1 && _azi_dependent(f1))
            isAzi = true;
        else if (lc == GOBS_LC::LC_L2 && _azi_dependent(f2))
            isAzi = true;
        else if (lc == GOBS_LC::LC_L3 && _azi_dependent(f3))
            isAzi = true;
        else if (lc == GOBS_LC::LC_L4 && _azi_dependent(f4))
            isAzi = true;
        else if (lc == GOBS_LC::LC_L5 && _azi_dependent(f5))
            isAzi = true;
        else
            isAzi = _azi_dependent(f2);

        if (isAzi)
        {
            // AZI-dependant calibration available
            t_gpair p_az(azi, zen);

            map<t_gpair, double> mapDataf1;
            map<t_gpair, double> mapDataf2;
            map<t_gpair, double> mapDataf3;
            map<t_gpair, double> mapDataf4;
            map<t_gpair, double> mapDataf5;

            for (map<GFRQ, t_map_A>::iterator itGFRQ = _mapazi.begin(); itGFRQ != _mapazi.end(); itGFRQ++)
            {
                GFRQ f = itGFRQ->first;

                if (lc == GOBS_LC::LC_IF && (f != f1 && f != f2))
                    continue;
                if (lc == GOBS_LC::LC_L1 && (f != f1))
                    continue;
                if (lc == GOBS_LC::LC_L2 && (f != f2))
                    continue;
                if (lc == GOBS_LC::LC_L3 && (f != f3))
                    continue;
                if (lc == GOBS_LC::LC_L4 && (f != f4))
                    continue;
                if (lc == GOBS_LC::LC_L5 && (f != f5))
                    continue;

                map<double, t_map_Z>::iterator itA1 = itGFRQ->second.lower_bound(azi);
                if (itA1 == itGFRQ->second.end() || itA1 == itGFRQ->second.begin())
                {
                    _gmutex.unlock();
                    return -1;
                }
                map<double, t_map_Z>::iterator itA2 = itA1;
                itA2--;

                map<double, double>::iterator itZ1 = itA1->second.lower_bound(zen);
                if (itZ1 == itA1->second.end() || itZ1 == itA1->second.begin())
                {
                    _gmutex.unlock();
                    return -1;
                }
                map<double, double>::iterator itZ2 = itZ1;
                itZ2--;

                map<double, double>::iterator itZ3 = itA2->second.lower_bound(zen);
                if (itZ3 == itA2->second.end() || itZ3 == itA2->second.begin())
                {
                    _gmutex.unlock();
                    return -1;
                }
                map<double, double>::iterator itZ4 = itZ3;
                itZ4--;

                t_gpair p1(itA1->first, itZ1->first);
                t_gpair p2(itA1->first, itZ2->first);
                t_gpair p3(itA2->first, itZ3->first);
                t_gpair p4(itA2->first, itZ4->first);

                if (f == f1)
                {
                    mapDataf1[p1] = itZ1->second;
                    mapDataf1[p2] = itZ2->second; 
                    mapDataf1[p3] = itZ3->second; 
                    mapDataf1[p4] = itZ4->second; 
                }
                else if (f == f2)
                {
                    mapDataf2[p1] = itZ1->second;
                    mapDataf2[p2] = itZ2->second;
                    mapDataf2[p3] = itZ3->second;
                    mapDataf2[p4] = itZ4->second;
                }
                else if (f == f3)
                {
                    mapDataf3[p1] = itZ1->second;
                    mapDataf3[p2] = itZ2->second;
                    mapDataf3[p3] = itZ3->second;
                    mapDataf3[p4] = itZ4->second;
                }
                else if (f == f4)
                {
                    mapDataf4[p1] = itZ1->second;
                    mapDataf4[p2] = itZ2->second;
                    mapDataf4[p3] = itZ3->second;
                    mapDataf4[p4] = itZ4->second;
                }
                else if (f == f5)
                {
                    mapDataf5[p1] = itZ1->second;
                    mapDataf5[p2] = itZ2->second;
                    mapDataf5[p3] = itZ3->second;
                    mapDataf5[p4] = itZ4->second;
                }
                else
                {
                    continue;
                }
            }

            t_ginterp interp(_spdlog);
            if (lc == GOBS_LC::LC_IF)
            {
                if (interp.bilinear(mapDataf1, p_az, corrf1) < 0 || interp.bilinear(mapDataf2, p_az, corrf2) < 0)
                {
                    _gmutex.unlock();
                    return -1;
                }
            }
            else
            {
                switch (lc)
                {
                case GOBS_LC::LC_L1:
                    if (interp.bilinear(mapDataf1, p_az, corrf1) < 0)
                    {
                        _gmutex.unlock();
                        return -1;
                    }
                    break;
                case GOBS_LC::LC_L2:
                    if (interp.bilinear(mapDataf2, p_az, corrf2) < 0)
                    {
                        _gmutex.unlock();
                        return -1;
                    }
                    break;
                case GOBS_LC::LC_L3:
                    if (interp.bilinear(mapDataf3, p_az, corrf3) < 0)
                    {
                        _gmutex.unlock();
                        return -1;
                    }
                    break;
                case GOBS_LC::LC_L4:
                    if (interp.bilinear(mapDataf4, p_az, corrf4) < 0)
                    {
                        _gmutex.unlock();
                        return -1;
                    }
                    break;
                case GOBS_LC::LC_L5:
                    if (interp.bilinear(mapDataf5, p_az, corrf5) < 0)
                    {
                        _gmutex.unlock();
                        return -1;
                    }
                    break;
                default:
                {
                    _gmutex.unlock();
                    return -1;
                }
                }
            }
        }
        else
        { // AZI-dependant calibration NOT available (only NOAZI)

            if (_gnote)
            {
                _gnote->mesg(GWARNING, "gpcv", "no REC AZI PCV [" + _anten + "/freq:" + t_gfreq::gfreq2str(f1) + "], just used NOAZI");
            }

            map<double, double> mapDataf1;
            map<double, double> mapDataf2;
            map<double, double> mapDataf3;
            map<double, double> mapDataf4;
            map<double, double> mapDataf5;

            if (lc == GOBS_LC::LC_IF)
            {
                if (_mapzen.find(f1) == _mapzen.end() || _mapzen.find(f2) == _mapzen.end())
                {
                    _gmutex.unlock();
                    return -1;
                }

                mapDataf1 = _mapzen[f1];
                mapDataf2 = _mapzen[f2];
                t_ginterp interp(_spdlog);
                if (interp.linear(mapDataf1, zen, corrf1) < 0 || interp.linear(mapDataf2, zen, corrf2) < 0)
                {
                    _gmutex.unlock();
                    return -1;
                }
            }
            else
            {
                if (_mapzen.find(f1) != _mapzen.end())
                {
                    mapDataf1 = _mapzen[f1];
                }
                if (_mapzen.find(f2) != _mapzen.end())
                {
                    mapDataf2 = _mapzen[f2];
                }
                if (_mapzen.find(f3) != _mapzen.end())
                {
                    mapDataf3 = _mapzen[f3];
                }
                if (_mapzen.find(f4) != _mapzen.end())
                {
                    mapDataf4 = _mapzen[f4];
                }
                if (_mapzen.find(f5) != _mapzen.end())
                {
                    mapDataf5 = _mapzen[f5];
                }

                t_ginterp interp(_spdlog);

                switch (lc)
                {
                case GOBS_LC::LC_L1:
                    if (interp.linear(mapDataf1, zen, corrf1) < 0)
                    {
                        _gmutex.unlock();
                        return -1;
                    }
                    break;
                case GOBS_LC::LC_L2:
                    if (interp.linear(mapDataf2, zen, corrf2) < 0)
                    {
                        _gmutex.unlock();
                        return -1;
                    }
                    break;
                case GOBS_LC::LC_L3:
                    if (interp.linear(mapDataf3, zen, corrf3) < 0)
                    {
                        _gmutex.unlock();
                        return -1;
                    }
                    break;
                case GOBS_LC::LC_L4:
                    if (interp.linear(mapDataf4, zen, corrf4) < 0)
                    {
                        _gmutex.unlock();
                        return -1;
                    }
                    break;
                case GOBS_LC::LC_L5:
                    if (interp.linear(mapDataf5, zen, corrf5) < 0)
                    {
                        _gmutex.unlock();
                        return -1;
                    }
                    break;
                default:
                {
                    _gmutex.unlock();
                    return -1;
                }
                }
            }
        }

        // [mm] -> [m]
        corrf1 /= 1000.0;
        corrf2 /= 1000.0;
        corrf3 /= 1000.0;
        corrf4 /= 1000.0;
        corrf5 /= 1000.0;

        // PCV for linear combination
        if (lc == LC_IF)
        {
            double koef1 = 0.0;
            double koef2 = 0.0;
            satdata.coef_ionofree(k1, koef1, k2, koef2);
            corrLC = corrf1 * koef1 + corrf2 * koef2;
        }
        else if (lc == LC_L1)
        {
            corrLC = corrf1;
        }
        else if (lc == LC_L2)
        {
            corrLC = corrf2;
        }
        else if (lc == LC_L3)
        {
            corrLC = corrf3;
        }
        else if (lc == LC_L4)
        {
            corrLC = corrf4;
        }
        else if (lc == LC_L5)
        {
            corrLC = corrf5;
        }
        else
            corrLC = corrf2;

        _gmutex.unlock();
        return 1;
    }

    bool t_gpcv::_azi_dependent(GFRQ f)
    {
        t_map_azi::iterator it = _mapazi.find(f);

        bool ret = false;

        if (it == _mapazi.end())
        {
            ret = false;
        }
        else
        {

            int size = _mapazi[f].size();
            if (size > 0)
            {
                ret = true;
            }
            else
            {
                ret = false;
            }
        }
        if (_pcv_noazi)
            ret = false;
        return ret;
    }

} // namespace
