/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.
-*/
#include <cmath>
#include <iomanip>

#include "gutils/gconst.h"
#include "gutils/gmatrixconv.h"
#include "gutils/gsysconv.h"
#include "gmodels/gbancroft.h"
#include "gset/gsetgen.h"

namespace gnut
{

    int gbancroft(const Matrix &BBpass, ColumnVector &pos)
    {

        if (pos.Nrows() != 4)
        {
            pos.ReSize(4);
        }
        pos = 0.0;

        for (int iter = 1; iter <= 2; iter++)
        {
            Matrix BB = BBpass;
            int mm = BB.Nrows();
            for (int ii = 1; ii <= mm; ii++)
            {
                double xx = BB(ii, 1);
                double yy = BB(ii, 2);
                double traveltime = 0.072;
                if (iter > 1)
                {
                    double zz = BB(ii, 3);
                    double rho2 = (xx - pos(1)) * (xx - pos(1)) +
                                  (yy - pos(2)) * (yy - pos(2)) +
                                  (zz - pos(3)) * (zz - pos(3));
                    if (rho2 < 0)
                        return -1;

                    double rho = sqrt(rho2);

                    traveltime = rho / CLIGHT;
                }
                double angle = traveltime * OMEGA;
                double cosa = cos(angle);
                double sina = sin(angle);
                BB(ii, 1) = cosa * xx + sina * yy;
                BB(ii, 2) = -sina * xx + cosa * yy;
            }

            Matrix BBB;
            if (mm > 4)
            {
                SymmetricMatrix hlp;
                hlp << BB.t() * BB;
                BBB = hlp.i() * BB.t();
            }
            else
            {
                BBB = BB.i();
            }
            ColumnVector ee(mm);
            ee = 1.0;
            ColumnVector alpha(mm);
            alpha = 0.0;
            for (int ii = 1; ii <= mm; ii++)
            {
                alpha(ii) = lorentz(BB.Row(ii).t(), BB.Row(ii).t()) / 2.0;
            }
            ColumnVector BBBe = BBB * ee;
            ColumnVector BBBalpha = BBB * alpha;
            double aa = lorentz(BBBe, BBBe);
            double bb = lorentz(BBBe, BBBalpha) - 1;
            double cc = lorentz(BBBalpha, BBBalpha);

            double root2 = bb * bb - aa * cc;
            if (root2 < 0)
                return -1;
            double root = sqrt(root2);

            Matrix hlpPos(4, 2);
            hlpPos.Column(1) = (-bb - root) / aa * BBBe + BBBalpha;
            hlpPos.Column(2) = (-bb + root) / aa * BBBe + BBBalpha;

            ColumnVector omc(2);
            for (int pp = 1; pp <= 2; pp++)
            {
                hlpPos(4, pp) = -hlpPos(4, pp);
                double tm = (BB(1, 1) - hlpPos(1, pp)) * (BB(1, 1) - hlpPos(1, pp)) +
                            (BB(1, 2) - hlpPos(2, pp)) * (BB(1, 2) - hlpPos(2, pp)) +
                            (BB(1, 3) - hlpPos(3, pp)) * (BB(1, 3) - hlpPos(3, pp));
                if (tm < 0)
                    return -1;

                omc(pp) = BB(1, 4) -
                          sqrt(tm) -
                          hlpPos(4, pp);
            }
            if (fabs(omc(1)) > fabs(omc(2)))
            {
                pos = hlpPos.Column(2);
            }
            else
            {
                pos = hlpPos.Column(1);
            }
        }
        return 0;
    }

    double lorentz(const ColumnVector &aa, const ColumnVector &bb)
    {
        return aa(1) * bb(1) + aa(2) * bb(2) + aa(3) * bb(3) - aa(4) * bb(4);
    }

} // namespace
