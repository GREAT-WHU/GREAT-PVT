
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 * 
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  
  (c) 2011-2017 Geodetic Observatory Pecny, http://www.pecny.cz (gnss@pecny.cz)
      Research Institute of Geodesy, Topography and Cartography
      Ondrejov 244, 251 65, Czech Republic

  This file is part of the G-Nut C++ library.
 
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 3 of
  the License, or (at your option) any later version.
 
  This library is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, see <http://www.gnu.org/licenses>.

-*/

#include "gppp.h"

#include "gutils/gconst.h"
#include "gutils/gtriple.h"
#include "gutils/gsysconv.h"
#include "gutils/gtypeconv.h"
#include "gutils/gmatrixconv.h"
#include "gmodels/gpppmodel.h"

namespace gnut
{

    t_gppp::t_gppp(string mark, t_gsetbase *set)
        : t_gspp(mark, set),
          _gotl(0),
          _running(false)
    {
        _tides = new t_gtide2010(_spdlog);
        _gModel = new t_gpppmodel(_site, _spdlog, _set);

        _get_settings();
    }

    t_gppp::t_gppp(string mark, t_gsetbase *set, t_spdlog spdlog)
        : t_gspp(mark, set, spdlog),
          _gotl(0),
          _running(false)
    {
        _tides = new t_gtide2010(_spdlog);
        _gModel = new t_gpppmodel(_site, _spdlog, _set);

        _get_settings();
    }

    t_gppp::~t_gppp()
    {
        if (_tides)
            delete _tides;
    }

    void t_gppp::setOTL(t_gallotl *gotl)
    {
        _gotl = gotl;
        if (_tides)
            _tides->setOTL(_gotl);
    }

    void t_gppp::setOBJ(t_gallobj *gallobj)
    {
        t_gspp::setOBJ(gallobj);

        dynamic_cast<t_gpppmodel *>(_gModel)->setOBJ(gallobj);
    }

    bool t_gppp::isRunning()
    {
        return _running;
    }

    int t_gppp::_get_settings()
    {
        t_gspp::_get_settings();

        _phase = dynamic_cast<t_gsetproc *>(_set)->phase();
        _doppler = dynamic_cast<t_gsetproc *>(_set)->doppler();
        _sigAmbig = dynamic_cast<t_gsetproc *>(_set)->sig_init_amb();

        return 1;
    }

    void t_gppp::_prt_results(t_giof *giof, const t_gallrslt &rslt)
    {
        if (!giof)
            return;
        ostringstream os;

        vector<t_gallrslt::result>::const_iterator it;
        double ztd = 0.0;
        double ztdrms = 0.0;
        t_gtriple xyz(0.0, 0.0, 0.0);
        t_gtriple xyz_ref(0.0, 0.0, 0.0);
        t_gtriple neu(0.0, 0.0, 0.0);
        t_gtriple rms(0.0, 0.0, 0.0);

        // reference crd for xyz2neu
        it = rslt.v_rslt.begin();
        if (it->type == "CRD_X")
        {
            xyz_ref[0] = it->val;
        }
        if (it->type == "CRD_Y")
        {
            xyz_ref[1] = it->val;
        }
        if (it->type == "CRD_Z")
        {
            xyz_ref[2] = it->val;
        }

        for (it = rslt.v_rslt.begin(); it != rslt.v_rslt.end(); ++it)
        {
            // COLLECT COORDINATES
            if (it->type == "CRD_X")
            {
                xyz[0] = it->val;
                rms[0] = it->rms;
            }
            if (it->type == "CRD_Y")
            {
                xyz[1] = it->val;
                rms[1] = it->rms;
            }
            if (it->type == "CRD_Z")
            {
                xyz[2] = it->val;
                rms[2] = it->rms;
            }

            // TROPO-DRIVEN OUTPUT (SUCH OUTPUT IS TEMPORARY SOLUTION!)
            if (it->type == "TRP" && xyz[0] != 0.0)
            {
                ztd = it->val;
                ztdrms = it->rms;
                t_gtime tt = it->beg + (it->end - it->beg);

                t_gtriple xyz_rho = xyz - xyz_ref;
                t_gtriple ell_ref;
                xyz2ell(xyz_ref, ell_ref, false);

                xyz2neu(ell_ref, xyz_rho, neu);

                os << fixed << setprecision(3)
                   << tt.str_ymdhms()
                   << " " << setw(13) << xyz[0]
                   << " " << setw(13) << xyz[1]
                   << " " << setw(13) << xyz[2]
                   << " " << setw(7) << ztd
                   << " " << setw(13) << neu[0]
                   << " " << setw(13) << neu[1]
                   << " " << setw(13) << neu[2]
                   << fixed << setprecision(5)
                   << " " << setw(9) << rms[0]
                   << " " << setw(9) << rms[1]
                   << " " << setw(9) << rms[2]
                   << " " << setw(7) << ztdrms
                   << " " << setw(2) << 0 // DATA SIZE NOT KNOWN HERE !!!!!
                   << endl;

                giof->write(os.str().c_str(), os.str().size()); // _flt->flush();

                os.clear();
                os.str("");
            }
        }
    }

} // namespace
