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
#include "gall/gallrslt.h"

using namespace std;

namespace gnut
{
    t_gallrslt::t_gallrslt()
    {
    }

    void t_gallrslt::append(const t_gallrslt::result &rslt)
    {
        if (rslt.type == "CRD_X" || rslt.type == "CRD_Y" || rslt.type == "CRD_Z")
        {
            set_crd.insert(rslt);
        }
        else if (rslt.type == "TRP")
        {
            set_trp.insert(rslt);
        }
        else if (rslt.type == "CLK")
        {
            set_clk.insert(rslt);
        }
        else if (rslt.type == "AMB")
        {
            set_amb.insert(rslt);
        }
        else
        {
            cerr << "WRONG TYPE. POSSSIBLE ARE: CRD_X, CRD_Y, CRD_Z, CLK, TRP, AMB" << endl;
        }

        v_rslt.push_back(rslt);
    }

    void t_gallrslt::append(const string &type, const t_gtime &beg, const t_gtime &end, int idx,
                            const string &prn, double adj, double rms, double val)
    {
        t_gallrslt::result rslt;

        rslt.type = type;
        rslt.beg = beg;
        rslt.end = end;
        rslt.index = idx;
        rslt.prn = prn;
        rslt.adj = adj;
        rslt.rms = rms;
        rslt.val = val;

        if (type == "CRD_X" || type == "CRD_Y" || type == "CRD_Z")
        {
            set_crd.insert(rslt);
        }
        else if (type == "TRP")
        {
            set_trp.insert(rslt);
        }
        else if (type == "CLK")
        {
            set_clk.insert(rslt);
        }
        else if (type == "AMB")
        {
            set_amb.insert(rslt);
        }
        else
        {
            cerr << "WRONG TYPE. POSSSIBLE ARE: CRD_X, CRD_Y, CRD_Z, CLK, TRP, AMB" << endl;
        }

        v_rslt.push_back(rslt);
    }

    bool t_gallrslt::result::operator<(const t_gallrslt::result &rslt) const
    {
        if ((beg < rslt.beg) ||
            (beg == rslt.beg && end < rslt.end) ||
            (beg == rslt.beg && end == rslt.end && prn < rslt.prn) ||
            (beg == rslt.beg && end == rslt.end && prn == rslt.prn && type < rslt.type))
        {
            return true;
        }
        return false;
    }

} // namespace
