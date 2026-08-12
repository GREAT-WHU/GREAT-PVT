
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.

-*/

#include <stdlib.h>
#include <iostream>
#include <iomanip>

#include "gmodels/ginterp.h"
#include "gutils/gtypeconv.h"


namespace gnut
{
    t_ginterp::t_ginterp() :
        _interp_1d(INTERP_1D::LINEAR),
        _interp_2d(INTERP_2D::BILINEAR),
        _interp_3d(INTERP_3D::VER2HOR),
        _interp_ht(INTERP_HT::INTERPOLATE)
    {
    }

    t_ginterp::~t_ginterp()
    {
    }

    t_ginterp::INTERP_1D t_ginterp::str_to_interp_1d(const std::string& str)
    {
        if (str.compare("LINEAR") == 0)
        {
            return INTERP_1D::LINEAR;
        }
        else if (str.compare("SPLINE") == 0)
        {
            return INTERP_1D::SPLINE;
        }
        else
        {
            std::cerr << "# warning - unknown 1D interpolation method: " << str << std::endl;
        }

        return INTERP_1D::LINEAR;
    }

    t_ginterp::INTERP_2D t_ginterp::str_to_interp_2d(const std::string& str)
    {
        if (str.compare("BILINEAR") == 0)
        {
            return INTERP_2D::BILINEAR;
        }
        else if (str.compare("IDW") == 0)
        {
            return INTERP_2D::IDW;
        }
        else if (str.compare("TPS") == 0)
        {
            return INTERP_2D::TPS;
        }
        else
        {
            std::cerr << "# warning - unknown 2D interpolation method: " << str << std::endl;
        }

        return INTERP_2D::BILINEAR;
    }

    t_ginterp::INTERP_3D t_ginterp::str_to_interp_3d(const std::string& str)
    {
        if (str.compare("HOR2VER") == 0)
        {
            return INTERP_3D::HOR2VER;
        }
        else if (str.compare("VER2HOR") == 0)
        {
            return INTERP_3D::VER2HOR;
        }
        else
        {
            std::cerr << "# warning - unknown 3D interpolation method: " << str << std::endl;
        }

        return INTERP_3D::VER2HOR;
    }

    t_ginterp::INTERP_HT t_ginterp::str_to_interp_ht(const std::string& str)
    {
        if (str.compare("INTERPOLATE") == 0)
        {
            return INTERP_HT::INTERPOLATE;
        }
        else if (str.compare("SCALE") == 0)
        {
            return INTERP_HT::SCALE;
        }
        else
        {
            std::cerr << "# warning - unknown HT interpolation method: " << str << std::endl;
        }

        return INTERP_HT::INTERPOLATE;
    }

    std::string t_ginterp::interp_1d_to_str(const INTERP_1D& typ)
    {
        switch (typ)
        {
            case INTERP_1D::LINEAR:
                return "LINEAR";
                break;
            case INTERP_1D::SPLINE:
                return "SPLINE";
                break;
            default:
                std::cerr << "# warning - unknown 1D interpolation method\n";
        }
        return "LINEAR";
    }

    std::string t_ginterp::interp_2d_to_str(const INTERP_2D& typ)
    {
        switch (typ)
        {
            case INTERP_2D::BILINEAR:
                return "BILINEAR";
                break;
            case INTERP_2D::IDW:
                return "IDW";
                break;
            case INTERP_2D::TPS:
                return "TPS";
                break;
            default:
                std::cerr << "# warning - unknown 2D interpolation method\n";
        }
        return "BILINEAR";
    }

    std::string t_ginterp::interp_3d_to_str(const INTERP_3D& typ)
    {
        switch (typ)
        {
            case INTERP_3D::HOR2VER:
                return "HOR2VER";
                break;
            case INTERP_3D::VER2HOR:
                return "VER2HOR";
                break;
            default:
                std::cerr << "# warning - unknown 3D interpolation method\n";
        }
        return "VER2HOR";
    }

    std::string t_ginterp::interp_ht_to_str(const INTERP_HT& typ)
    {
        switch (typ)
        {
            case INTERP_HT::INTERPOLATE:
                return "INTERPOLATE";
                break;
            case INTERP_HT::SCALE:
                return "SCALE";
                break;
            default:
                std::cerr << "# warning - unknown HT interpolation method\n";
        }
        return "INTERPOLATE";
    }

    int t_ginterp::linear(std::map<double, double>& data, double val, double& fval)
    {
        std::map<double, double>::iterator it2, it1;
        it2 = data.lower_bound(val);

        if (it2 != data.end() && double_eq(it2->first, val))
        {
            fval = it2->second;
            return 0;
        }
        if (it2 == data.begin() || it2 == data.end())
        {
#ifdef DEBUG
            std::cerr << "# warning: extrapolation is not allowed " << it2->first << " " << val << "!\n";
#endif
            return -1;
        }

        it1 = it2;
        it1--;

        Eigen::MatrixXd x(2, 2), y(2, 1);
        x << it1->first, 1, it2->first, 1;
        y << it1->second, it2->second;

        Eigen::MatrixXd coef(2, 1);
        coef = x.inverse() * y;

        fval = coef(0, 0) * val + coef(1, 0);

#ifdef DEBUG
        std::cerr << "t_ginterp::linear: "
             << "x = " << val << " fx = " << fval << std::endl;
#endif
        return 0;
    }

    // ----------------------------
    // Spline Interpolation (double)
    // ----------------------------
    int t_ginterp::spline(std::map<double, double>& data, double val, double& fval)
    {
        if (data.size() == 1)
        {
            std::cerr << "Warning: For using of Cubic Spline Interpolation Method, the dimension of input data vector equal "
                    "to (at least): dimm = 3, is requested "
                 << std::endl;
            return -1;
        }
        else if (data.size() == 2)
        {
            t_ginterp CallLin;
            CallLin.linear(data, val, fval);
        }
        else if (data.size() > 2)
        {
            //    map<double, double>::const_iterator I = data.upper_bound( val );
            std::map<double, double>::const_iterator II;

            std::vector<double> X, Y;
            for (II = data.begin(); II != data.end(); ++II)
            {
                X.push_back(II->first);
                Y.push_back(II->second);
            }

#ifdef DEBUG
            for (II = data.begin(); II != data.end(); ++II)
            {
                std::cout << II->first << "  :  " << II->second << std::endl;
            }

            std::cout << "Interpolated value " << val << std::endl;
#endif

            double e = val;

            std::vector<double> h, lambda, eta, B, C, D, g, CoefA, CoefB, CoefC, CoefD, AA, BB, FI;

            // vector h: OK
            for (unsigned int i = 0; i < X.size() - 1; i++)
            {
                h.push_back(X[i + 1] - X[i]);
            }
            // lambda: OK, eta: OK, g
            for (unsigned int i = 0; i < X.size() - 2; i++)
            {
                lambda.push_back(h[i] / (h[i] + h[i + 1]));
                eta.push_back(1.0 - lambda[i]);
                B.push_back(2); // (4.6.12.) diag = 2;
            }
            for (unsigned int j = 1; j < X.size() - 2; j++)
            {
                C.push_back(lambda[j]);
            }
            for (unsigned int j = 0; j < X.size() - 3; j++)
            {
                D.push_back(eta[j]);
            }

            Eigen::MatrixXd A(X.size() - 2, X.size() - 2), M(X.size() - 2, X.size() - 2);
            A.setZero();
            for (unsigned int i = 0; i < X.size() - 2; i++)
            {
                A(i, i) = B[i];
            }
            for (unsigned int i = 0; i < X.size() - 3; i++)
            {
                A(i, i + 1) = D[i];
                A(i + 1, i) = C[i];
            }

            Eigen::VectorXd G(X.size() - 2), MM(X.size());
            for (unsigned int i = 0; i < X.size() - 2; i++) // ok
            {
                g.push_back((6.0 / (h[i] + h[i + 1])) * (((Y[i + 2] - Y[i + 1]) / h[i + 1]) - ((Y[i + 1] - Y[i]) / h[i])));
            }

            for (unsigned int i = 0; i < g.size(); i++)
            {
                G(i) = g[i];
            }

            // Solving a linear equations
            M = A.inverse() * G;
            double M0 = 0.0;
            double Mn = 0.0;

            MM(0) = M0;
            for (unsigned int i = 1; i < X.size() - 1; i++) // ok
            {
                MM(i) = M(i - 1);
            }
            MM(X.size() - 1) = Mn;

            // Coeficients a, b, c, d
            for (unsigned int i = 0; i < X.size() - 1; i++) // ok
            {
                CoefA.push_back(Y[i]);
                CoefB.push_back(((Y[i + 1] - Y[i]) / h[i]) - (2 * MM[i] + MM[i + 1]) * h[i] / 6);
                CoefC.push_back(0.5 * MM[i]);
                CoefD.push_back((MM[i + 1] - MM[i]) / (6 * h[i]));

                // Interpolation

                AA.push_back(((Y[i + 1] - Y[i]) / h[i]) - (h[i] / 6) * (MM[i + 1] - MM[i]));
                BB.push_back(Y[i] - MM[i] * (pow(h[i], 2) / 6));
                FI.push_back(MM[i] * (pow((X[i + 1] - e), 3) / (6 * h[i])) + MM[i + 1] * ((pow((e - X[i]), 3)) / (6 * h[i])) +
                             AA[i] * (e - X[i]) + BB[i]);
                if (e == X[i])
                {
                    fval = Y[i];
                }
                else if (X[i] < e && e < X[i + 1])
                {
                    fval = FI[i];
                }
            }
        }
        else
        {
            std::cerr << "Warning (ginterp-spline): something wrong with input data!" << std::endl;
        }

        return 0;
    }

    // ----------------------------
    // Linear Interpolation (time)
    // ----------------------------
    int t_ginterp::linear(const std::map<t_gtime, double>& data, const t_gtime& epo, double& fval)
    {
        if (data.size() == 0)
        {
            return -1;
        }
        else if (epo == data.begin()->first)
        { // data.size() == 1 && ){

            fval = data.begin()->second;
            return 0;
        }
        else if (epo == data.rbegin()->first)
        { // data.size() == 1 && ){

            fval = data.rbegin()->second;
            return 0;
        }
        else if (data.size() == 2)
        {
            std::map<t_gtime, double>::const_iterator itDAT;
            std::map<double, double> tmp;

            for (itDAT = data.begin(); itDAT != data.end(); ++itDAT)
            {
                t_gtime dt(itDAT->first);
                double gt = (dt.mjd() - epo.mjd()) * 86400 + dt.sod() - epo.sod() + dt.dsec() - epo.dsec(); // all in sec

                tmp[gt] = itDAT->second;
                //      cout << " seconds = " << itDAT->first.str_ymdhms() << " " << gt << endl;
            }

            int irc = linear(tmp, 0.0, fval);
            return irc;
        }
        else
        {
            std::cerr << "# warning: a problem in time linear interpolation! [#node:" << data.size() << "] " << epo.str_ymdhms() << std::endl;
            return -1;
        }

        return 0;
    }

    // ----------------------------
    // bilinear interpolation
    //
    // 0 .. 11 (bottom-left)         21 *---------* 22
    // 1 .. 12 (bottom-right)           |         |
    // 2 .. 21 (top-left)               |         |
    // 3 .. 22 (top-right)           11 *---------* 12
    // ----------------------------
    int t_ginterp::bilinear(const std::map<t_gpair, double>& data, const t_gpair& req_pos, double& fval)
    {
        std::vector<t_gpair> v_pt;
        std::map<t_gpair, double>::const_iterator itDAT = data.begin();
        for (; itDAT != data.end(); ++itDAT)
        {
            v_pt.push_back(itDAT->first);
#ifdef DEBUG
            std::cout << "# interp:" << std::fixed << std::setprecision(0) << " " << itDAT->first[0] << ":" << itDAT->first[1] << " " << std::setprecision(3)
                 << " " << data.at(v_pt.at(v_pt.size() - 1)) << std::endl;
#endif
        }

        // no interpolation (1 point)
        if (v_pt.size() == 1)
        {
            fval = data.at(v_pt[0]);

            // linear interpolation (2 points)
        }
        else if (v_pt.size() == 2)
        {
            double x = 0.0;
            double a = 0.0;
            double b = 0.0;

            if (v_pt[1][0] - v_pt[0][0] == 0)
            {
                x = req_pos[1]; // lat=const
                a = v_pt[0][1];
                b = v_pt[1][1];
            }
            else
            {
                x = req_pos[0]; // lon=const
                a = v_pt[0][0];
                b = v_pt[1][0];
            }

            if (a - b == 0)
            {
                std::cout << "# warning: Bilinear interpolation failed (2 points) \n";
                fval = 0.0;
                return -1;
            }
            else
            {
                fval = data.at(v_pt[0]) + (data.at(v_pt[1]) - data.at(v_pt[0])) * (x - a) / (a - b);
            }

            // bilinear interpolation (4 points)
        }
        else if (v_pt.size() == 4)
        {
#ifdef DEBUG
            std::cout << "# Linear interpolation (4): " << std::fixed << std::setprecision(0) << "  " << v_pt[0][0] << ":" << v_pt[0][1] << "  "
                 << v_pt[1][0] << ":" << v_pt[1][1] << "  " << v_pt[2][0] << ":" << v_pt[2][1] << "  " << v_pt[3][0] << ":" << v_pt[3][1]
                 << " ." << std::setprecision(3) << "  " << req_pos[0] << "-" << req_pos[1] << "  " << data.at(v_pt[0]) << "  "
                 << data.at(v_pt[1]) << "  " << data.at(v_pt[2]) << "  " << data.at(v_pt[3]) << std::endl;
#endif

            double a = data.at(v_pt[0]) * (v_pt[3][0] - req_pos[0]) * (v_pt[3][1] - req_pos[1]);
            double b = data.at(v_pt[1]) * (req_pos[0] - v_pt[2][0]) * (v_pt[2][1] - req_pos[1]);
            double c = data.at(v_pt[2]) * (v_pt[1][0] - req_pos[0]) * (req_pos[1] - v_pt[1][1]);
            double d = data.at(v_pt[3]) * (req_pos[0] - v_pt[0][0]) * (req_pos[1] - v_pt[0][1]);

            if ((v_pt[2][0] - v_pt[1][0]) * (v_pt[1][1] - v_pt[0][1]) == 0)
            {
                //       cout << "# warning: Bilinear interpolation failed (4 points)\n";
                fval = 0.0;
                return -1;
            }
            else
            {
                fval = (a + b + c + d) / ((v_pt[2][0] - v_pt[1][0]) * (v_pt[1][1] - v_pt[0][1]));
            }
        }
        else
        {
            fval = 0.0;
            //     cout << "# warning: Bilinear interpolation failed (X points ?)\n";
            return -1;
        }

        return 0;
    }

} // namespace gnut
