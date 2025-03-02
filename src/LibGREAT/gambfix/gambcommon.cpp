/**
 * @file         gambcommon.cpp
 * @author       GREAT-WHU (https://github.com/GREAT-WHU)
 * @brief        declare some classes and some mathematical method.
 * @version      1.0
 * @date         2024-08-29
 * 
 * @copyright Copyright (c) 2024, Wuhan University. All rights reserved.
 * 
 */
#include "gambfix/gambcommon.h"
using namespace std;

namespace great
{

    epoch_amb_res::epoch_amb_res()
    {
        wl_res = 0.0;
        wl_fixed = false;
        nl_res = 0.0;
        nl_fixed = false;
    }

    epoch_amb_res::~epoch_amb_res()
    {
    }

    epoch_amb::epoch_amb()
    {
        bc = 0.0;
        sbc = 0.0;
        bw = 0.0;
        sbw = 0.0;
        bw0 = 0.0;
        bn = 0.0;
        isnewarc = true;
        nepo = 0;
        elev = 0.0;
        bwi = 0.0;
    }

    epoch_amb::~epoch_amb()
    {
    }

    void getMeanFract(list<pair<double, double>> &values, double &mean, double &sigma, double &sigx)
    {
        try
        {
            if (values.empty())
                return;

            //local variables
            double temp_fract;
            double mean2, sigma2;
            list<pair<double, double>> fracts[3];

            //fraction range is (0,1)
            for (auto it_val = values.begin(); it_val != values.end(); it_val++)
            {
                temp_fract = it_val->first - round(it_val->first);
                if (temp_fract < 0.0)
                    temp_fract += 1.0;
                fracts[0].push_back(make_pair(temp_fract, it_val->second));
            }

            getMeanWgt(true, fracts[0], mean, sigma, sigx);

            //fraction range is (-0.50,0.50)
            for (auto it_val = values.begin(); it_val != values.end(); it_val++)
            {
                temp_fract = getFraction(it_val->first, -0.50);
                fracts[1].push_back(make_pair(temp_fract, it_val->second));
            }

            getMeanWgt(true, fracts[1], mean2, sigma2, sigx);

            if (sigma2 < sigma && !double_eq(sigma, sigma2)) // to be same with fortran add "="
                mean = mean2;
            // to avoid bias

            //-------
            for (auto it_val = values.begin(); it_val != values.end(); it_val++)
            {
                temp_fract = it_val->first - mean;
                temp_fract -= round(temp_fract);
                fracts[2].push_back(make_pair(temp_fract, it_val->second));
            }

            getMeanWgt(true, fracts[2], mean2, sigma, sigx);

            mean += mean2;
            values = fracts[2];
        }
        catch (...)
        {
            cout << "ERROR: t_gmath::get_mean_fract , unknown mistake";
            throw(-1);
        }
    }

    void getMeanWgt(bool is_edit, list<pair<double, double>> &wx, double &mean, double &sigma, double &mean_sig)
    {
        try
        {
            if (wx.empty())
            {
                mean_sig = 0.0;
                return;
            }
            double wgt_sum;

            bool loop_flag = true;
            while (loop_flag)
            {
                // mean
                mean = 0.0;
                wgt_sum = 0.0;
                for (auto itwx = wx.begin(); itwx != wx.end(); itwx++)
                {
                    mean += itwx->first * itwx->second;
                    wgt_sum += itwx->second;
                }
                mean /= wgt_sum;

                // sigma
                sigma = 0.0;
                wgt_sum = 0.0;
                for (auto itwx = wx.begin(); itwx != wx.end(); itwx++)
                {
                    sigma += (itwx->first - mean) * (itwx->first - mean) * itwx->second;
                    wgt_sum += itwx->second;
                }
                if (wx.size() == 1)
                {
                    sigma = 999.0;
                    mean_sig = 999.0;
                    return;
                }
                else
                {
                    sigma = sqrt(sigma / (wx.size() - 1.0));
                    mean_sig = sigma / sqrt(wgt_sum);
                }

                if (is_edit)
                {
                    loop_flag = false;
                    for (auto itwx = wx.begin(); itwx != wx.end();)
                    {
                        if (fabs(itwx->first - mean) > 3 * (sigma))
                        {
                            itwx = wx.erase(itwx);
                            loop_flag = true;
                        }
                        else
                        {
                            itwx++;
                        }
                    }
                }
                else
                {
                    return;
                }
            }
            return;
        }
        catch (...)
        {
            cout << "ERROR: t_gmath::get_wgt_mean , unknown mistake";
            throw(-1);
        }
    }

    double getFraction(double x, double min)
    {
        double fract;
        fract = x - round(x);

        if (double_eq(min, 0.0))
        {
            if (fract < 0.0)
            {
                fract += 1.0;
            }
            else if (fract >= 1.0)
            {
                fract -= 1.0;
            }
        }
        else if (double_eq(min, -0.50))
        {
            if (fract < -0.5)
            {
                fract += 1.0;
            }
            else if (fract >= 0.5)
            {
                fract -= 1.0;
            }
        }
        else
        {
            if (fract >= 1.0)
            {
                fract -= 1.0;
            }
        }

        return fract;
    }

    t_oneway_ambiguity::t_oneway_ambiguity()
    {
    }

}