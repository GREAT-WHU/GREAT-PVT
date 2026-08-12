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

    int gbancroft(const Eigen::MatrixXd& BBpass, Eigen::VectorXd& pos)
    {
        if (pos.rows() != 4)
        {
            pos.resize(4);
        }
        pos.setZero();

        for (int iter = 1; iter <= 2; iter++)
        {
            Eigen::MatrixXd BB = BBpass;
            int mm = BB.rows();
            for (int ii = 0; ii < mm; ++ii)
            {
                double xx = BB(ii, 0);
                double yy = BB(ii, 1);
                double traveltime = 0.072;
                if (iter > 1)
                {
                    double zz = BB(ii, 2);
                    double rho2 = (xx - pos(0)) * (xx - pos(0)) + (yy - pos(1)) * (yy - pos(1)) + (zz - pos(2)) * (zz - pos(2));
                    if (rho2 < 0)
                    {
                        return -1;
                    }

                    double rho = sqrt(rho2);

                    traveltime = rho / CLIGHT;
                }
                double angle = traveltime * OMEGA;
                double cosa = cos(angle);
                double sina = sin(angle);
                BB(ii, 0) = cosa * xx + sina * yy;
                BB(ii, 1) = -sina * xx + cosa * yy;
            }

            Eigen::MatrixXd BBB;
            if (mm > 4)
            {
                Eigen::MatrixXd hlp;
                hlp = BB.transpose() * BB;
                BBB = hlp.inverse() * BB.transpose();
            }
            else
            {
                BBB = BB.inverse();
            }
            Eigen::VectorXd ee(mm);
            ee.setOnes();
            Eigen::VectorXd alpha(mm);
            alpha.setZero();
            for (int ii = 0; ii < mm; ++ii)
            {
                alpha(ii) = lorentz(BB.row(ii).transpose(), BB.row(ii).transpose()) / 2.0;
            }
            Eigen::VectorXd BBBe = BBB * ee;
            Eigen::VectorXd BBBalpha = BBB * alpha;
            double aa = lorentz(BBBe, BBBe);
            double bb = lorentz(BBBe, BBBalpha) - 1;
            double cc = lorentz(BBBalpha, BBBalpha);

            double root2 = bb * bb - aa * cc;
            if (root2 < 0)
            {
                return -1;
            }
            double root = sqrt(root2);

            Eigen::MatrixXd hlpPos(4, 2);
            hlpPos.col(0) = (-bb - root) / aa * BBBe + BBBalpha;
            hlpPos.col(1) = (-bb + root) / aa * BBBe + BBBalpha;

            Eigen::VectorXd omc(2);
            for (int pp = 0; pp < 2; ++pp)
            {
                hlpPos(3, pp) = -hlpPos(3, pp);
                double tm = (BB(0, 0) - hlpPos(0, pp)) * (BB(0, 0) - hlpPos(0, pp)) +
                            (BB(0, 1) - hlpPos(1, pp)) * (BB(0, 1) - hlpPos(1, pp)) +
                            (BB(0, 2) - hlpPos(2, pp)) * (BB(0, 2) - hlpPos(2, pp));
                if (tm < 0)
                {
                    return -1;
                }

                omc(pp) = BB(0, 3) - sqrt(tm) - hlpPos(3, pp);
            }
            if (fabs(omc(0)) > fabs(omc(1)))
            {
                pos = hlpPos.col(1);
            }
            else
            {
                pos = hlpPos.col(0);
            }
        }
        return 0;
    }

    double lorentz(const Eigen::VectorXd& aa, const Eigen::VectorXd& bb)
    {
        return aa(0) * bb(0) + aa(1) * bb(1) + aa(2) * bb(2) - aa(3) * bb(3);
    }

} // namespace gnut
