
/* ----------------------------------------------------------------------
 * G-Nut - GNSS software development library
 *
  (c) 2018 G-Nut Software s.r.o. (software@gnutsoftware.com)
  This file is part of the G-Nut C++ library.

-*/

#include "gprod/gprodcrd.h"


namespace gnut
{

    t_gprodcrd::t_gprodcrd(const t_gtime& t, std::shared_ptr<t_gobj> pt) :
        t_gprod(t, pt)
    {
        id_type(ID_TYPE::POS);
        id_group(ID_GROUP::GRP_PRODUCT);
    }

    t_gprodcrd::~t_gprodcrd()
    {
    }

    void t_gprodcrd::xyz(const t_gtriple& xyz)
    {
        _xyz = xyz;
    }

    t_gtriple t_gprodcrd::xyz() const
    {
        return _xyz;
    }

    void t_gprodcrd::xyz_rms(const t_gtriple& xyz_rms)
    {
        _xyz_rms = xyz_rms;
    }

    t_gtriple t_gprodcrd::xyz_rms() const
    {
        return _xyz_rms;
    }

    t_gtriple t_gprodcrd::xyz_var() const
    {
        t_gtriple var(_xyz_rms[0] * _xyz_rms[0], _xyz_rms[1] * _xyz_rms[1], _xyz_rms[2] * _xyz_rms[2]);

        return var;
    }

    void t_gprodcrd::apr(const t_gtriple& apr)
    {
        _apr = apr;
    }

    t_gtriple t_gprodcrd::apr() const
    {
        return _apr;
    }

    void t_gprodcrd::apr_rms(const t_gtriple& apr_rms)
    {
        _apr_rms = apr_rms;
    }

    t_gtriple t_gprodcrd::apr_rms() const
    {
        return _apr_rms;
    }

    t_gtriple t_gprodcrd::apr_var() const
    {
        t_gtriple var(_apr_rms[0] * _apr_rms[0], _apr_rms[1] * _apr_rms[1], _apr_rms[2] * _apr_rms[2]);

        return var;
    }

    void t_gprodcrd::cov(COV_TYPE type, double& cov)
    {
        switch (type)
        {
            case COV_TYPE::COV_XY:
                _xy_cov = cov;
                break;
            case COV_TYPE::COV_XZ:
                _xz_cov = cov;
                break;
            case COV_TYPE::COV_YZ:
                _yz_cov = cov;
                break;
            default:
                _xy_cov = _xz_cov = _yz_cov = 0.0;
        }
    }

    double t_gprodcrd::cov(COV_TYPE type) const
    {
        switch (type)
        {
            case COV_TYPE::COV_XY:
                return _xy_cov;
                break;
            case COV_TYPE::COV_XZ:
                return _xz_cov;
                break;
            case COV_TYPE::COV_YZ:
                return _yz_cov;
                break;
            default:
                return 0.0;
        }
    }

} // namespace gnut
